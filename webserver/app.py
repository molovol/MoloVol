import os
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
logdir = "./logs/"

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
    with open(f"{logdir}/{id}") as f:
        return Response(response=f.read(), status=200, mimetype="text/plain")


def save_log(log) -> str:
    # generate random filename
    # check if directory exists and create if missing
    if not os.path.exists(logdir):
        os.makedirs(logdir)
    filename = f"{logdir}{uuid4()}.log"
    with open(filename, "w") as f:
        f.write(log)
    return filename


@app.route('/', methods=['GET', 'POST'])
def io():
    global out
    out = None
    loglink = None
    inputdict = {}
    if request.method == 'POST':
        # when arguments ignore form data
        # split cli string so that it can be passed to subprocess
        args: list[str] = []
        if len(request.form) > 0:
            for key, value in request.form.items():
                if not key.startswith("cli_"):
                    continue
                key = key.removeprefix("cli_")
                args.append(f"--{key}")
                if value != "" and value != "on":
                    args.append(str(value))
            inputdict = request.form

            # read structure file
            if structure_path := manage_uploaded_file(request, "structure"):
                args.append("-fs")
                args.append(structure_path)

            # read elements file
            if request.form.get("use_default_elements", "") != "on":
                print("using custom elements file")
                elements_path = manage_uploaded_file(request, "elements")
            else:
                elements_path = "./inputfile/elements.txt"
            args.append("-fe")
            args.append(elements_path)
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
                loglink = f"{request.url_root}{save_log(out)}"
            except Exception as e:
                out = "Exception: " + str(e)
    if request.accept_mimetypes['text/html']:
        if type(out) is str:
            out = out.split("\n")
        return render_template('form.html', inputdict=inputdict, returnvalues=out,
                               loglink=loglink)
    elif request.accept_mimetypes['application/json']:
        return jsonify({"output": out})
