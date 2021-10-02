#!/bin/bash

echo "    Network Init"

systemctl stop connman.service
systemctl stop systemd-networkd.socket
systemctl stop systemd-networkd.service


