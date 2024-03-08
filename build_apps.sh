#!/bin/sh -ex
/home/hirokichi/osbook/devenv/buildenv.sh
make 
sudo mount haribote.img img
for APP in $(ls apps/*/Makefile)
do
	DIR=$(dirname $APP)
	ANAME=$(basename $DIR)
	echo $DIR
	echo $ANAME
	make -C $DIR
	sudo cp $DIR/$ANAME img/$ANAME
done
sleep 0.5
sudo umount img
sudo cp -f haribote.img /media/sf_Windows2021/shr/past.iso
