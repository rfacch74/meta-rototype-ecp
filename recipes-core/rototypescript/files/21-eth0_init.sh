#!/bin/bash

echo "    Ethernet eth0 Init"

ETH0_PARAM_FILE="/home/root/config/eth0.param"

INTERFACE0=eth0
IPADDR0=169.254.10.10
SUBNET0=255.255.255.0
GWADDR0=169.254.10.1
DHCP_MODE0="disable"



if [ -e $ETH0_PARAM_FILE ]; then
	IPADDR0=$(sed -n -e '/ipaddr/{s/.*=//p}'  $ETH0_PARAM_FILE)
	GWADDR0=$(sed -n -e '/gwaddr/{s/.*=//p}'  $ETH0_PARAM_FILE)
	DHCP_MODE0=$(sed -n -e '/dhcp_mode/{s/.*=//p}'  $ETH0_PARAM_FILE)
fi;

echo $IPADDR0 $GWADDR0 $DHCP_MODE

if [ "$DHCP_MODE0" = "enable" ];then
	ip link set $INTERFACE0 down
	udhcpc -i eth0
	ip link set $INTERFACE0 up
else
	ip addr flush dev $INTERFACE0
	ip link set $INTERFACE0 down
	ip link set $INTERFACE0 up
	ip addr add $IPADDR0/24 dev $INTERFACE0
	ip route add default via $GWADDR0	
fi;


