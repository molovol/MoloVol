#!bin/sh
targeticon=$1
targetdir=$2
for d in /usr/share/icons/hicolor/*/; do
	d=${d::-1}
	d=${d##*/}
	mkdir $targetdir/$d
	mkdir $targetdir/$d/apps
	n=${d##*x}
	if [[ $n =~ ^[0-9]+$ ]] ; then
		convert -resize $d $targeticon $targetdir/$d/apps/molovol.png
	fi
done
