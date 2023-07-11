FROM ubuntu
RUN apt-get update
RUN DEBIAN_FRONTEND="noninteractive" apt-get -y install tzdata
RUN apt-get install -y build-essential manpages-dev libgtk2.0-dev wget
ARG wxversion=3.2.2
RUN wget https://github.com/wxWidgets/wxWidgets/releases/download/v${wxversion}/wxWidgets-${wxversion}.tar.bz2
RUN tar xvf wxWidgets-${wxversion}.tar.bz2
WORKDIR  wxWidgets-${wxversion}
RUN ./configure --disable-shared --enable-unicode
RUN make install
