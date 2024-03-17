#!/bin/sh

DEBIAN_FRONTEND="noninteractive" apt-get update & apt-get -y install tzdata build-essential manpages-dev libgtk2.0-dev wget
#version in varialbe
wget https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.5/wxWidgets-3.1.5.tar.bz2
tar xvf wxWidgets-3.1.5.tar.bz2
cd wxWidgets-3.1.5
./configure --disable-shared --enable-unicode
make install