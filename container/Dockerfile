# builds molovol and installs the webapp
FROM bsvogler/wxwidgets
# hack to create a headless x server, cmake for building
RUN apt update && apt upgrade -y
RUN apt-get install xvfb cmake -y
# does not work when set in dockerfile?
ENV DISPLAY=:1.0

# compile molovol, for some reason some less important files like the readme are needed for cmake
WORKDIR /build
COPY cmake/ cmake/
COPY CMakeLists.txt CMakeLists.txt
COPY res/ res/
COPY src/ src/
COPY include/ include/
COPY LICENSE ./
COPY README.md ./
COPY inputfile/ inputfile/
RUN cmake . -DCMAKE_BUILD_TYPE=RELEASE && make
WORKDIR /

# add flask webserver
#RUN apt install software-properties-common -y
RUN apt install python3.11 curl -y && apt-get clean && rm -rf /var/lib/apt/lists/*
# install poetry
RUN curl -sSL https://install.python-poetry.org | POETRY_HOME=/usr/local/ python3 - --version 1.6.1
RUN poetry config virtualenvs.create false
RUN poetry config virtualenvs.options.system-site-packages true
ENV PYTHONPATH=$PYTHONPATH:/usr/local/lib/python3.12/site-packages/
# install dependencies
COPY webserver/ ./
RUN poetry install --no-root --only main
WORKDIR /

COPY launch_headless.sh /launch_headless.sh
RUN chmod +x launch_headless.sh
ENV FLASK_APP=/webserver/app.py
CMD [ "python3", "-m" , "flask", "run", "--host=0.0.0.0"]
