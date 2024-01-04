#!/bin/sh -ex
/home/hirokichi/osbook/devenv/buildenv.sh
make 
sudo mount haribote.img img
sleep 0.5
sudo umount img
sudo cp -f haribote.img /media/sf_Windows2021/shr/past.iso
