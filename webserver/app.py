import os
from typing import Optional

from flask import Flask, request, jsonify, render_template
import subprocess

from werkzeug.utils import secure_filename

app = Flask(__name__)

ALLOWED_EXTENSIONS = {'xyz', 'pdb', 'txt', 'cif'}


def allowed_file(filename):
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS


def manage_uploaded_file(request, filetype="structure"):
    global out
    path: Optional[str] = None
    if filetype in request.files:
        # try using last value
        structure_file = request.files['structure']
        if structure_file.filename == '':
            out += "No selected file\n"
        if structure_file and allowed_file(structure_file.filename):
            path = secure_filename(structure_file.filename)
            structure_file.save(os.path.join(app.config['UPLOAD_FOLDER'], path))
    if path is None:
        path = request.form.get(f"last{filetype}", None)
        if path is None:
            out += "error as no file was uploaded and last one could not be reused\n"
    return path


@app.route('/', methods=['GET', 'POST'])
def io():
    out = None
    inputdict = {}
    if request.method == 'POST':
        # when arguments ignore form data
        # split cli string so that it can be passed to subprocess
        args: list[str] = []
        if len(request.form) > 0:
            for key, value in request.form.items():
                if not key.startswith("cli_"):
                    continue
                key.removeprefix("cli_")
                args.append(f"--{key}")
                if value != "":
                    args.append(str(value))
            inputdict = request.form

            # read structure file
            if structure_path := manage_uploaded_file(request, "structure"):
                args.append("-fs")
                args.append(structure_path)

            # read elements file
            if request.form.get("use_default_elements", "") != "on":
                print("using custom elements file")
                elements_path = manage_uploaded_file(request, "elements"):
                args.append("-fe")
                args.append(elements_path)

        else:
            args = request.args.get('cli-arguments', '').split(" ")
            inputdict = request.args
        try:
            out = subprocess.check_output(["./launch_headless.sh"] + args, stderr=subprocess.STDOUT).decode(
                "utf-8")
        except Exception as e:
            out = "Exception: " + str(e)
    if request.accept_mimetypes['text/html']:
        return render_template('form.html', inputdict=inputdict, returnvalues=out)
    elif request.accept_mimetypes['application/json']:
        return jsonify({"output": out})
