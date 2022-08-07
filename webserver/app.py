import os
import zipfile
from typing import Optional
from uuid import uuid4

from flask import Flask, request, jsonify, render_template, Response
import subprocess

from werkzeug.utils import secure_filename

app = Flask(__name__)
UPLOAD_FOLDER = './userupload/'
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

# check if upload folder exists and create if missing
if not os.path.exists(UPLOAD_FOLDER):
    os.makedirs(UPLOAD_FOLDER)

ALLOWED_EXTENSIONS = {'xyz', 'pdb', 'txt', 'cif'}


def allowed_file(filename):
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS


out = None
log_dir = "./logs/"
export_dir = "./export/"


def manage_uploaded_file(request, filetype="structure"):
    global out
    path: Optional[str] = None
    if filetype in request.files:
        # try using last value
        structure_file = request.files['structure']
        if structure_file.filename == '':
            if out is None:
                out = "No structure file selected"
            else:
                out += "No structure file selected\n"
        if structure_file and allowed_file(structure_file.filename):
            path = os.path.join(app.config['UPLOAD_FOLDER'], secure_filename(structure_file.filename))
            structure_file.save(path)
            return path
    if path is None:
        path = request.form.get(f"last{filetype}", None)
        if path is None or path == "":
            path = None
            out += "error as no file was uploaded and last one could not be reused\n"
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
            print("Zipping file: " +path+ file)
            zip.write(path+file, arcname=file)
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
                if value != "" and value != "on":
                    # TODO: Appending the value without making sure it isn't harmful is a security
                    # risk. A user can willingly or unwillingly inject code here that is run on
                    # the command line.
                    args.append(str(value))

                # when export is requested add -do option
                if export is None and key.startswith("export") and value == "on":
                    export = uuid4()
                    tmp_outdir = f"{export_dir}{export}/"
                    # check if directory exists and create if missing
                    print(f"Exporting in zip in {tmp_outdir}")
                    if not os.path.exists(tmp_outdir):
                        os.makedirs(tmp_outdir)
                    args.append(f"-do")
                    args.append(tmp_outdir)
            inputdict = request.form

            # read structure file
            if structure_path := manage_uploaded_file(request, "structure"):
                args.append("-fs")
                args.append(structure_path)

            # read elements file
            elements_path = "./inputfile/elements.txt"
            args.append("-fe")
            args.append(elements_path)

            # we only print it out in the end so we can put it to quiet
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
        return render_template('form.html', inputdict=inputdict, returnvalues=out,
                               resultslink=resultslink, version=v_out)
    elif request.accept_mimetypes['application/json']:
        return jsonify({"output": out})


# Request the executable's version. If the executable is not found, then the web page crashes
def app_version():
    return subprocess.check_output(["./launch_headless.sh"] + ["-v"], stderr=subprocess.STDOUT).decode("utf-8")


def is_nonzero_numeric(value):
    try:
        if float(value) == 0:
            return False
    except ValueError:
        return False
    return True

