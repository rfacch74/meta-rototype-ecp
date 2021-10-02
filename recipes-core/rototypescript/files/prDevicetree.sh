#!/bin/bash

file_dtb="/home/root/update/imx8mq-ecp.dtb"

if [ -f "$file_dtb" ] 
then
	echo "    update Devicetree.."
	cp $file_dtb /boot/
fi


