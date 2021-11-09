FROM ubuntu
RUN apt-get update
RUN DEBIAN_FRONTEND="noninteractive" apt-get -y install tzdata
RUN apt-get install -y build-essential manpages-dev libgtk2.0-dev wget
RUN wget https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.5/wxWidgets-3.1.5.tar.bz2
RUN tar xvf wxWidgets-3.1.5.tar.bz2
WORKDIR  wxWidgets-3.1.5
RUN ./configure --disable-shared --enable-unicode
RUN make install

# hack to create a headless x server, does not work when set in dockerfile?
RUN apt-get install xvfb -y
ENV DISPLAY=:1.0


WORKDIR /
COPY Makefile Makefile
COPY src/ src/
COPY include/ include/
RUN make

WORKDIR /src/bin/
COPY launch_headless.sh launch.sh
RUN chmod +x launch.sh
ENTRYPOINT ["/src/bin/launch.sh"]
#you can connect and run with "podman run -it <image id> /bin/sh"