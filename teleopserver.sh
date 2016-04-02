#! /bin/sh
# /etc/init.d/teleopserver


export PATH=$PATH:/home/pi/Teleop2
#path_node=/home/pi/picar/app.js
# required since all files are here
cd /home/pi/Teleop2

case "$1" in
	start)
		sleep 1
		echo "starting teleopserver"
		echo "[$date]" >> /var/log/teleopserver.log		
		#node $path_node >> /var/log/teleopserver.log & 
		./rungserver.sh &
		;;
	stop)
		echo "stop teleopserver"
		#echo "['date']" >> /var/log/node-server.log
		#killall node
		
		;;
	*)
		echo "Usage start|stop"
		exit 1
		;;
esac
exit 0
