FROM alpine
RUN apk apk update
RUN apk add build-base
RUN apk add gtk+2.0-dev
RUN wget https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.5/wxWidgets-3.1.5.tar.bz2
RUN tar xvf wxWidgets-3.1.5.tar.bz2
WORKDIR  wxWidgets-3.1.5
RUN ./configure --disable-shared --enable-unicode
RUN make install