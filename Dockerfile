FROM ubuntu
RUN apt-get update
RUN DEBIAN_FRONTEND="noninteractive" apt-get -y install tzdata
RUN apt-get install -y build-essential manpages-dev libgtk2.0-dev wget
RUN wget https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.5/wxWidgets-3.1.5.tar.bz2
RUN tar xvf wxWidgets-3.1.5.tar.bz2
WORKDIR  wxWidgets-3.1.5
RUN ./configure --disable-shared --enable-unicode
RUN make install
WORKDIR /
COPY Makefile Makefile
COPY src/ src/
COPY include/ include/
RUN make

# hack to create a headless x server
RUN apt-get install xvfb -y
RUN Xvfb :1 -screen 0 1024x768x16 &> xvfb.log  &
RUN DISPLAY=:1.0
RUN export DISPLAY

WORKDIR /src/bin/
CMD MoloVol --version
#you can connect and run with "podman run -it <image id> /bin/sh"