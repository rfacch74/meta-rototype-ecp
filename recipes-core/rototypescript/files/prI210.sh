#!/bin/bash

file_i210="/home/root/update/mac.addr"

if [ -f "$file_i210" ] 
then
	EepromAccessTool -NIC=1 -F=/home/root/firmware/I210_Invm_Copper_NoAPM_v0.6.HEX -MAC=cat $file_i210
fi


