#!/bin/sh
# 1. first agument is IP of remote server ./runclient.sh 192.168.1.6
#
if [ -z "$1" ]
  then
	tput setaf 1
    echo "No argument supplied, must specify IP of server (PI)"
    tput setaf 7
   exit
fi
#
# 2.find xbox controller
#
EVENT=$(cat /proc/bus/input/devices  | grep =event -m1  | cut -d'='  -f2 | cut -d' ' -f1)
JS=$(cat /proc/bus/input/devices  | grep =event -m1  | cut -d'='  -f2 | cut -d' ' -f2)
if [ -z "$EVENT" ]
  then
	tput setaf 1
    echo "Error - Connect XBOX360 controller or test with jstest"
    tput setaf 7
    ls /dev/input/event*
   exit
fi
if [ -z "$JS" ]
  then
	tput setaf 1
    echo "Error - Connect XBOX360 controller or test with jstest"
    tput setaf 7
    ls /dev/input/js*
   exit
fi
# 3.launch gstreamer client
#
gst-launch-1.0 -v tcpclientsrc host=$1 port=5000 ! gdpdepay ! rtph264depay ! avdec_h264 ! videoconvert ! videoflip method=horizontal-flip ! autovideosink sync=false &
#
# 4.launch client
#
cd ./client
./teleop_client /dev/input/$EVENT /dev/input/$JS  $1
