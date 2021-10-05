#!/bin/bash

echo "    ECP Platform Update>"

file_prboard="/home/root/_program"
file_prusb="/run/media/sda1/_program"
file_update="/home/root/update/_update"
file_update_usb="/run/media/sda1/_update_usb"




if [ -f "$file_prboard" ]
then
        systemctl stop weston
        psplash -n &
        psplash-write "ECP SYSTEM PROGRAMMING......"
        sleep 1.5
	if [ -f "$file_prusb" ]
	then
		mount /dev/sda1 /home/root/update
	fi

	psplash-write "MSG ECP_PROGRAMMING_FOUND.."
        psplash-write "PROGRESS 10"
        /home/root/script/prEMMC.sh
        psplash-write "PROGRESS 40"
        /home/root/script/prI210.sh
        psplash-write "PROGRESS 50"
        /home/root/script/prFpga.sh
        psplash-write "PROGRESS 60"
        /home/root/script/prSoc.sh
        psplash-write "PROGRESS 100"
        killall psplash
        systemctl start weston
fi

if [ -f "$file_update" ]
then
        echo "    ECP APP UPDATE FOUND............................>"
        /home/root/script/prKernel.sh
        /home/root/script/prDevicetree.sh
        /home/root/script/prFpga.sh
        /home/root/script/prSoc.sh
        rm $file_update
fi

if [ -f "$file_update_usb" ]
then
        echo "    ECP USB UPDATE FOUND............................>"
        mount /dev/sda1 /home/root/update
        /home/root/script/prKernel.sh
        /home/root/script/prDevicetree.sh
        /home/root/script/prFpga.sh
        /home/root/script/prSoc.sh
fi


