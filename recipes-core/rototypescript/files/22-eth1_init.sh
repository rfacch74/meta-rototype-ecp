#!/bin/bash

echo "    Ethernet eth1 Init"

INTERFACE1=eth1
IPADDR1=192.168.121.2


ip link set $INTERFACE1 up
ip addr add $IPADDR1/24 dev $INTERFACE1




