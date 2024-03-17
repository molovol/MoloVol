#!/bin/sh

#version in varialbe
WX_VERSION=3.1.5
DEBIAN_FRONTEND="noninteractive" apt-get update & apt-get -y install tzdata build-essential manpages-dev libgtk2.0-dev wget
wget https://github.com/wxWidgets/wxWidgets/releases/download/v${WX_VERSION}/wxWidgets-${WX_VERSION}.tar.bz2
tar xvf wxWidgets-${WX_VERSION}.tar.bz2
cd wxWidgets-${WX_VERSION}
./configure --disable-shared --enable-unicode
make install