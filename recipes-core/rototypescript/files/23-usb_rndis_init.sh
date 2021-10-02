#!/bin/bash

echo "    Ethernet Gadget Init"

USBADDR0=10.0.0.1

modprobe g_ether

sleep 2

ip addr add $USBADDR0/24 dev usb0
ip link set usb0 up

