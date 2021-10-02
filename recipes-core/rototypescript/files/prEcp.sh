#!/bin/bash

file_prboard="/home/root/update/_program"
file_update="/home/root/update/_update"


if [ -f "$file_prboard" ] 
then
	/home/root/script/prEMMC.sh
	/home/root/script/prI210.sh
	/home/root/script/prFpga.sh
	/home/root/script/prSoc.sh
fi

if [ -f "$file_update" ] 
then
	/home/root/script/prKernel.sh
	/home/root/script/prDevicetree.sh
	/home/root/script/prFpga.sh
	/home/root/script/prSoc.sh
fi


