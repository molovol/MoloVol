from flask import Flask, request, jsonify
import subprocess

app = Flask(__name__)


@app.route('/', methods=['GET'])
def hello_world():
    args: list[str] = request.args.get('cli-arguments', '').split(" ")
    print(args)
    try:
        returnvalues = subprocess.check_output(["./launch_podman.sh"] + args, stderr=subprocess.STDOUT)
    except Exception as e:
        return "Exception:" + str(e)
    return jsonify({"output": returnvalues.decode("utf-8")})
