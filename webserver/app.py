from flask import Flask, request, jsonify, render_template
import subprocess

app = Flask(__name__)


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
                args.append(f"--{key}")
                if value != "":
                    args.append(str(value))
            inputdict = request.form
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
