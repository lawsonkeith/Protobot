#!/bin/sh
# 1. first agument is IP of remote server ./runclient.sh 192.168.1.6
#
if [ -z "$1" ]
  then
    echo "No argument supplied, must specify IP of server (PI)"
   exit
fi
#
# 2.launch gstreamer client
#
gst-launch-1.0 -v tcpclientsrc host=$1 port=5000 ! gdpdepay ! rtph264depay ! avdec_h264 ! videoconvert ! videoflip method=horizontal-flip ! autovideosink sync=false
#
# 3.launch teleop client
#
