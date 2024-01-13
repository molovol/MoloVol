# builds molovol and installs the webapp
FROM bsvogler/wxwidgets
# hack to create a headless x server, cmake for building
RUN apt-get install xvfb cmake -y
# does not work when set in dockerfile?
ENV DISPLAY=:1.0

# compile molovol
WORKDIR /build
COPY CMakeLists.txt CMakeLists.txt
COPY src/ src/
COPY include/ include/
RUN cmake . && make
COPY inputfile/ inputfile/
WORKDIR /

# add flask webserver
RUN apt update && apt upgrade -y
RUN apt install software-properties-common -y
RUN add-apt-repository ppa:deadsnakes/ppa -y
RUN apt install python3.11 -y
# install pip
RUN apt install python3.11-distutils -y
RUN apt install curl -y
RUN curl https://bootstrap.pypa.io/get-pip.py | python3.11
RUN python3.11 -m pip install --ignore-installed flask
COPY webserver/ /webserver/
WORKDIR /

COPY launch_headless.sh /launch_headless.sh
RUN chmod +x launch_headless.sh
ENV FLASK_APP=/webserver/app.py
CMD [ "python3.11", "-m" , "flask", "run", "--host=0.0.0.0"]