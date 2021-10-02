#!/bin/bash

file_kernel="/home/root/update/Image.bin"

if [ -f "$file_kernel" ] 
then
	echo "    update kernel.."
	cp $file_kernel /boot/Image
fi



