import os
import zipfile
import re
import shutil
from typing import Optional
from uuid import uuid4
from enum import Enum

from flask import Flask, request, jsonify, render_template, Response, send_from_directory, url_for
from flask_cors import CORS
import subprocess

from werkzeug.security import safe_join
from werkzeug.utils import secure_filename

app = Flask(__name__)
UPLOAD_FOLDER = './userupload/'
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

# check if upload folder exists and create if missing
if not os.path.exists(UPLOAD_FOLDER):
    os.makedirs(UPLOAD_FOLDER)

log_dir = "./logs/"
export_dir = "./export/"

if not os.path.exists(log_dir):
    os.makedirs(log_dir)
if not os.path.exists(export_dir):
    os.makedirs(export_dir)

out = None

# Cross-Origin Resource Sharing
cors = CORS(app, resources={r"/api/*": {"origins": "http://localhost:4000"}})

# Error messages

# This prefix allows the renderer to tell that the line is an error
ERR_PREFIX = "E."
class ErrMsg(str, Enum):
    NOFILE =    ERR_PREFIX + "No file was uploaded and previous file could not be used",
    NORADIUS =  ERR_PREFIX + "Small probe radius must be specied",
    NOGRID =    ERR_PREFIX + "Spatial resolution must be specified",
    SMALLGRID = ERR_PREFIX + "If you would like to use a spatial resolution of less than 0.1, please use the desktop application",
    FORMAT =    ERR_PREFIX + "File format of uploaded file is not supported"

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
    if path:
        return os.path.basename(path).split('_',1)[-1]
    else:
        return ""


@app.template_filter('versionnumber')
def versionnumber(versiontxt):
    return re.findall("\d+\.\d+\.\d+", versiontxt)[0]


@app.template_filter('titleline')
def titleline(output):
    return re.search("<[A-Z]+>", output)


@app.template_filter('tablesplit')
def tablesplit(tablerow):
    n=20;
    return [tablerow[i:i+n] for i in range(0, len(tablerow), n)]


@app.template_filter('errorstrip')
def errorstrip(errorline):
    return errorline[len(ERR_PREFIX):]


ALLOWED_EXTENSIONS = {'xyz', 'pdb', 'cif'}


def validate_extension(filename):
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

        input_file = request.files[filetype]

        if not input_file.filename:
            path = request.form.get(f"last{filetype}", None)
            if not path:
                path = None
                out += ErrMsg.NOFILE + "\n"
        elif validate_extension(input_file.filename):
            # File extension validation needs to be handled server side. Users can circumvent
            # the html form by directly using the REST-API
            print(input_file.filename)
            path = os.path.join(app.config['UPLOAD_FOLDER'],
                                secure_filename(uuid4().hex + '_' + input_file.filename))
            input_file.save(path)
        else:
            out += ErrMsg.FORMAT + "\n"
            path = None
    
    if out:
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

        reduce_dir_to_target_size(export_dir, 
                                  3 * 1024 * 1024 * 1024, 
                                  4 * 1024 * 1024 * 1024, 
                                  600); # 3 GiB kept 10 mins, max 4 GiB
        reduce_dir_to_target_size(UPLOAD_FOLDER, 
                                  0.8 * 1024 * 1024 * 1024, 
                                  1.0 * 1024 * 1024 * 1024, 
                                  1800); # 0.8 GiB kept 30 mins, max 1 GiB
        reduce_dir_to_target_size(log_dir, 
                                  10 * 1024 * 1024, 
                                  100 * 1024 * 1024, 
                                  3600); # 10 MiB kept 60 mins, max 100 MiB

        out = ""
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
                if key == "radius2":
                    if not is_nonzero_numeric(value):
                        continue

                # Radius and grid are obligatory options. If they aren't specified
                # omit them here so the error can be caught later
                if key == "radius" and not value:
                    out += ErrMsg.NORADIUS + "\n"
                    continue
                if key == "grid":
                    if not value:
                        out += ErrMsg.NOGRID + "\n"
                        continue
                    elif is_str_smaller_than(value, 0.1):
                        out += ErrMsg.SMALLGRID + "\n"
                        continue


                args.append(f"--{key}")
                    
                # Handle checkboxes
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

                # Handle other options
                elif value and safe_join(str(value)):
                    # Always make sure that the value can be safely appended
                    # args will be passed to the command line!
                    args.append(str(value))

            inputdict = request.form

            # read structure file
            if structure_path := manage_uploaded_file(request, filetype="structure"):
                args.append("-fs")
                args.append(structure_path)

            # read elements file
            elements_path = "/usr/share/molovol/elements.txt"
            args.append("-fe")
            args.append(elements_path) # May not even be necessary

            # we only print it out in the end, so we can put it to quiet
            args.append("-q")
            args.append("-o") 
            args.append("resolution,depth,radius_small,radius_large,options,vol,surf,cavities")

        else:
            args = request.args.get('cli-arguments', '').split(" ")
            inputdict = request.args

        # These three options are required
        if "--radius" in args and "--grid" in args and structure_path:
            print(f"Starting process with args: {args}\n")
            
            try:
                mlvl_out = subprocess.check_output(["./launch_headless.sh"] + args, stderr=subprocess.STDOUT).decode(
                    "utf-8")
            except Exception as e:
                out = "Exception: " + str(e)

            # If an error occured, extract the error message from the output
            if mlvl_out.startswith("Usage"):
                mlvl_out = ERR_PREFIX + mlvl_out.split("\n")[-2] + "\n"
            else:
                mlvl_out = f"Results for structure file: {basename(structure_path)}\n" + mlvl_out 

            out += mlvl_out
            print(out)

            if export:
                resultslink = f"/{tmp_outdir}"
            else:
                resultslink = f"/{save_log(out)}"

    if request.accept_mimetypes['text/html']:

        if type(out) is str:
            # Remove the trailing newline
            out = out.split("\n")[:-1]

        if structure_path:
            print(structure_path)

        return render_template('form.html', inputdict=inputdict, returnvalues=out,
                               resultslink=resultslink, version=v_out, laststructure=structure_path)

    elif request.accept_mimetypes['application/json']:
        return jsonify({"output": out})

import time
# Makes space for new user upload and export files by deleting the oldest files until a predefined
# amount of disk space is available. Keeps all files that are younger than the grace period. The
# grace period is given in seconds.
def reduce_dir_to_target_size(directory, target_size, max_size, grace_period=0):
    print(f"Deleting entries in {directory} to reach {target_size} bytes.")
    total_size = get_entry_size(directory)

    entries = os.listdir(directory)
    entries = [os.path.join(directory,x) for x in entries]
    entries.sort(key=os.path.getctime, reverse=True)  # Sort files and folders by creation time (oldest first)

    current_time = time.time()
    latest_ctime = current_time - grace_period # Determine the latest allowed creation time

    while entries and total_size > target_size:
        entry_path = entries.pop()
        entry_size = get_entry_size(entry_path)
        
        # Skip file if it is too young and there is still space in the directory
        entry_ctime = os.path.getctime(entry_path)
        if entry_ctime > latest_ctime and total_size < max_size:
            continue

        entry_size = get_entry_size(entry_path)

        try:
            if os.path.isfile(entry_path):
                os.remove(entry_path) # Delete the file
            else:
                shutil.rmtree(entry_path) # Delete the folder
            print(f"Deleted: {entry_path}")
            total_size -= entry_size
        except OSError as e:
            print(f"Error deleting: {entry_path}, {e}")

    print(f"Total directory size is now {total_size} bytes.\n")

# Determines the size of an entry, i.e., a directory or a file
def get_entry_size(entry_path):
    total_size = 0
        
    if os.path.isfile(entry_path):
        total_size = os.path.getsize(entry_path)
    else:
        for path, dirs, files in os.walk(entry_path):
            for file in files:
                file_path = os.path.join(path, file)
                total_size += os.path.getsize(file_path)

    return total_size

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


def is_str_smaller_than(text, lowerlim):
    try:
        return float(text) < lowerlim
    except ValueError:
        return True

