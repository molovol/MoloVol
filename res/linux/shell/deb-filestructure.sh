#!bin/sh
DIR=$1
mkdir $DIR/deb-staging
mkdir $DIR/deb-staging/DEBIAN
mkdir $DIR/deb-staging/usr
mkdir $DIR/deb-staging/usr/bin
mkdir $DIR/deb-staging/usr/share
mkdir $DIR/deb-staging/usr/share/applications
mkdir $DIR/deb-staging/usr/share/doc
mkdir $DIR/deb-staging/usr/share/doc/molovol
mkdir $DIR/deb-staging/usr/share/man
mkdir $DIR/deb-staging/usr/share/man/man1
mkdir $DIR/deb-staging/usr/share/molovol
mkdir $DIR/deb-staging/usr/share/pixmaps
mkdir $DIR/deb-staging/usr/share/icons
mkdir $DIR/deb-staging/usr/share/icons/hicolor
chmod 0755 $(find $DIR/deb-staging/usr -type d)
