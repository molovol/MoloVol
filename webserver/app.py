from flask import Flask, request, jsonify, render_template
import subprocess

app = Flask(__name__)


@app.route('/', methods=['GET', 'POST'])
def io():
    returnvalues = None
    if request.method == 'POST':
        args: list[str] = request.args.get('cli-arguments', '').split(" ")
        try:
            returnvalues = subprocess.check_output(["./launch_headless.sh"] + args, stderr=subprocess.STDOUT).decode(
                "utf-8")
        except Exception as e:
            returnvalues = "Exception: " + str(e)
    if request.accept_mimetypes['text/html']:
        return render_template('form.html', returnvalues=returnvalues)
    elif request.accept_mimetypes['application/json']:
        return jsonify({"output": returnvalues})
