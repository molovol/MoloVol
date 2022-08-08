import os
import zipfile
from typing import Optional
from uuid import uuid4

from flask import Flask, request, jsonify, render_template, Response, send_from_directory, url_for
import subprocess

from werkzeug.security import safe_join
from werkzeug.utils import secure_filename

app = Flask(__name__)
UPLOAD_FOLDER = './userupload/'
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

# check if upload folder exists and create if missing
if not os.path.exists(UPLOAD_FOLDER):
    os.makedirs(UPLOAD_FOLDER)

out = None
log_dir = "./logs/"
export_dir = "./export/"


# Redirects for favicons

@app.route('/favicon.ico')
def favicon():
    return send_from_directory(os.path.join(app.root_path, 'static'), 'favicon.ico')


@app.route('/favicon-16x16.png')
def favicon16():
    return send_from_directory(os.path.join(app.root_path, 'static'), 'favicon-16x16.png')


@app.route('/favicon-32x32.png')
def favicon32():
    return send_from_directory(os.path.join(app.root_path, 'static'), 'favicon-32x32.png')


@app.route('/site.webmanifest')
def manifest():
    return send_from_directory(os.path.join(app.root_path, 'static'), 'site.webmanifest')


@app.route('/apple-touch-icon.png')
def apple_touch_icon():
    return send_from_directory(os.path.join(app.root_path, 'static'), 'apple-touch-icon.png')


@app.route('/safari-pinned-tab.svg')
def safari_pinned_tab():
    return send_from_directory(os.path.join(app.root_path, 'static'), 'safari-pinned-tab.svg')


@app.route('/browserconfig.xml')
def browserconfig():
    return send_from_directory(os.path.join(app.root_path, 'static'), 'browserconfig.xml')


@app.route('/android-chrome-192x192.png')
def android_chrome192():
    return send_from_directory(os.path.join(app.root_path, 'static'), 'android-chrome-192x192.png')


@app.route('/android-chrome-512x512.png')
def android_chrome512():
    return send_from_directory(os.path.join(app.root_path, 'static'), 'android-chrome-512x512.png')


@app.route('/mstile-144x144.png')
def mstile144():
    return send_from_directory(os.path.join(app.root_path, 'static'), 'mstile-144x144.png')


@app.route('/mstile-150x150.png')
def mstile150():
    return send_from_directory(os.path.join(app.root_path, 'static'), 'mstile-150x150.png')


@app.route('/mstile-310x150.png')
def mstile310x150():
    return send_from_directory(os.path.join(app.root_path, 'static'), 'mstile-310x150.png')


@app.route('/mstile-310x310.png')
def mstile310():
    return send_from_directory(os.path.join(app.root_path, 'static'), 'mstile-310x310.png')


@app.route('/mstile-70x70.png')
def mstile70():
    return send_from_directory(os.path.join(app.root_path, 'static'), 'mstile-70x70.png')


# jinja2 filters

@app.template_filter('basename')
def basename(path):
    return os.path.basename(path)


ALLOWED_EXTENSIONS = {'xyz', 'pdb', 'cif'}


def allowed_file(filename):
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS


def manage_uploaded_file(request, filetype="structure"):
    """
    saves uploaded file to UPLOAD_FOLDER
    :param request:
    :param filetype:
    :return: path of the saved file
    """
    global out
    path: Optional[str] = None
    if filetype in request.files:
        # try using last value
        input_file = request.files[filetype]
        if not input_file.filename:
            if out is None:
                out = "No structure file selected"
            else:
                out += "No structure file selected\n"
        # File extension validation needs to be handled here because client can circumvent the html form
        # Does 'if input_file' ever fail? - JM
        elif input_file and allowed_file(input_file.filename):
            path = os.path.join(app.config['UPLOAD_FOLDER'],
                                secure_filename(uuid4().hex + input_file.filename.rsplit('_', maxsplit=1)[-1]))
            input_file.save(path)
        else:
            # This is necessary because otherwise 'path' does not get initialised, so that the next
            # if-statement gets executed, then a string is appended to 'out', so out cannot be None
            out = ""

    if path is None:
        path = request.form.get(f"last{filetype}", None)
        if not path:
            path = None
            out += "Error: No file was uploaded and previous one could not be used\n"
    print(out)
    return path


@app.route("/logs/<id>", methods=["GET"])
def get_logs(id):
    """id is the filename of the log file"""
    with open(f"{log_dir}/{id}") as f:
        return Response(response=f.read(), status=200, mimetype="text/plain")


@app.route("/export/<id>/", methods=["GET"])
def get_export(id):
    zip_export(f"{export_dir}{id}/")
    if not os.path.exists(f"{export_dir}{id}/results.zip"):
        return Response(response="No export found", status=404, mimetype="text/plain")
    with open(f"{export_dir}{id}/results.zip", "rb") as f:
        return Response(response=f.read(), status=200, mimetype="application/zip")


def zip_export(path) -> str:
    # zip files in path
    zip_file = f"{path}results.zip"
    dir_content = os.listdir(path)
    print(f"Found these files in path: {dir_content}. Zipping them")
    with zipfile.ZipFile(zip_file, 'w') as zip:
        for file in dir_content:
            print("Zipping file: " + path + file)
            zip.write(path + file, arcname=file)
    return zip_file


def save_log(log) -> str:
    # generate random filename
    # check if directory exists and create if missing
    if not os.path.exists(log_dir):
        os.makedirs(log_dir)
    filename = f"{log_dir}{uuid4()}.log"
    with open(filename, "w") as f:
        f.write(log)
    return filename


@app.route('/', methods=['GET', 'POST'])
def io():
    global out
    out = None
    resultslink = None
    inputdict = {}
    export = None
    tmp_outdir = None
    structure_path = None

    v_out = app_version()

    if request.method == 'POST':
        # when arguments ignore form data
        # split cli string so that it can be passed to subprocess
        args: list[str] = []
        if len(request.form) > 0:
            for key, value in request.form.items():
                if not key.startswith("cli_"):
                    continue
                key = key.removeprefix("cli_")

                # If the 'large probe radius' field contains a zero or a non-numeric
                # value, then the argument is not passed on
                if key == "radius2" and not is_nonzero_numeric(value):
                    continue

                args.append(f"--{key}")

                if value == "on":
                    # when export is requested add -do option
                    if export is None and key.startswith("export"):
                        export = uuid4()
                        tmp_outdir = f"{export_dir}{export}/"
                        # check if directory exists and create if missing
                        print(f"Exporting in zip in {tmp_outdir}")
                        if not os.path.exists(tmp_outdir):
                            os.makedirs(tmp_outdir)
                        args.append(f"-do")
                        args.append(tmp_outdir)
                else:
                    # Appending the value without making sure it isn't harmful is a security
                    # risk. A user can willingly or unwillingly inject code here that is run on
                    # the command line.
                    if value != "" and safe_join(str(value)):
                        args.append(str(value))

            inputdict = request.form

            # read structure file
            if structure_path := manage_uploaded_file(request, filetype="structure"):
                args.append("-fs")
                args.append(structure_path)

            # read elements file
            elements_path = "./inputfile/elements.txt"
            args.append("-fe")
            args.append(elements_path)

            # we only print it out in the end, so we can put it to quiet
            args.append("-q")

        else:
            args = request.args.get('cli-arguments', '').split(" ")
            inputdict = request.args
        # with no parameters it will launch the gui
        if len(args) == 0 or structure_path is None:
            out += "No arguments given\n You must specify at least the structure file\n"
            args.append("-h")
        else:
            try:
                print("Starting process with args:", args)
                out = subprocess.check_output(["./launch_headless.sh"] + args, stderr=subprocess.STDOUT).decode(
                    "utf-8")
                print(out)
                if export:
                    resultslink = f"/{tmp_outdir}"
                else:
                    resultslink = f"/{save_log(out)}"
            except Exception as e:
                out = "Exception: " + str(e)
    if request.accept_mimetypes['text/html']:

        if type(out) is str:
            out = out.split("\n")

        print(structure_path)

        return render_template('form.html', inputdict=inputdict, returnvalues=out,
                               resultslink=resultslink, version=v_out, laststructure=structure_path)

    elif request.accept_mimetypes['application/json']:
        return jsonify({"output": out})


# Request the executable's version. If the executable is not found, then the web page crashes
def app_version():
    return subprocess.check_output(["./launch_headless.sh", "-v"], stderr=subprocess.STDOUT).decode("utf-8")


def is_nonzero_numeric(value):
    try:
        if float(value) == 0:
            return False
    except ValueError:
        return False
    return True
