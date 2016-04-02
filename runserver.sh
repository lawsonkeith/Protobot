#!/bin/sh
# Gstreamer shell script, stream video to a linux laptop.  Since we're using
# DHCP this shell needs to wait for wlan0 and work out it's host address.
# 
LEDON="24=0" > /dev/pi-blaster
LEDOFF="24=1" > /dev/pi-blaster
NULLSTEER="18=0.14" > /dev/pi-blaster
NULLSPEED="17=0.14" > /dev/pi-blaster
STEERLEFT="18=0.105" > /dev/pi-blaster
#
# 0.zero / test PWM servos
echo $NULLSTEER
echo $NULLSPEED
echo $LEDON
sleep 1
echo $LEDOFF
#
echo $STEERLEFT
echo $LEDON
sleep 1
echo $LEDOFF
#
echo $NULLSTEER
#
# 1.wait for network, by pinging google!
#
echo wait for internet...
echo $LEDON
sleep 1
echo $LEDOFF
while ! ping  -c1 google.co.uk -W1 | grep " 0% p" ; do
  sleep 1
done
echo ok...
#
# 2.ok now get HOST IP
# 
echo $LEDON
sleep 1
echo $LEDOFF
HOST=$(ifconfig | grep "inet " | grep  "255.255" | cut -d':'  -f2 | cut -d' ' -f1)
#
# 3.run raspivid
#
echo $LEDON
sleep 1
echo $LEDOFF
raspivid -t 999999 -h 720 -w 1080 -fps 25 -hf -b 2000000 -o - | gst-launch-1.0 -v fdsrc ! h264parse !  rtph264pay config-interval=1 pt=96 ! gdppay ! tcpserversink host=$HOST port=5000
