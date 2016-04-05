#!/bin/sh
# Gstreamer shell script, stream video to a linux laptop.  Since we're using
# DHCP this shell needs to wait for wlan0 and work out it's host address.
#  
reset
echo
tput setaf 4
echo looking for XBOX 360...
tput setaf 7
echo 
EVENT=$(cat /proc/bus/input/devices  | grep =event -m1  | cut -d'='  -f2 | cut -d' ' -f1)
JS=$(cat /proc/bus/input/devices  | grep =event -m1  | cut -d'='  -f2 | cut -d' ' -f2)
echo sudo ./teleop_client /dev/input/$EVENT /dev/input/$JS  PI_IPADDR
echo
