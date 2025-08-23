# builds molovol and installs the webapp
FROM alpine:latest
RUN apk add --no-cache \
    cmake \
    make \
    gcc \
    g++ \
    musl-dev
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
RUN cmake . -DCMAKE_BUILD_TYPE=RELEASE -DMOLOVOL_ABS_RESOURCE_PATH=ON -DMOLOVOL_BUILD_GUI=OFF && make
RUN apk add --no-cache \
    file \
    dpkg
RUN cpack -G DEB && dpkg -i MoloVol_*.deb
WORKDIR /

# add flask webserver
RUN apk add --no-cache python3 py3-pip curl

# install poetry
RUN curl -sSL https://install.python-poetry.org | POETRY_HOME=/usr/local/ python3 - --version 1.8.1
RUN poetry config virtualenvs.create false
RUN poetry config virtualenvs.options.system-site-packages true

# install dependencies
COPY webserver/ ./
RUN poetry install --no-root --only main

WORKDIR /

ENV FLASK_APP=/webserver/app.py
CMD [ "python3", "-m" , "flask", "run", "--host=0.0.0.0"]