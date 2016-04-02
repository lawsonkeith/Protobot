## Teleop2
Improved RC car teleop system with haptic feedback using C on a raspberry Pi.  The main improvement is the use of an HD camera rather than the XBOX USB camera which couldn't go past really low res.
This now uses gstreamer rather than mjpeg-streamer to achieve this.

There are 4 software modules:

1. **teleop_client**		- Laptop
2. gstreamer video client	- Laptop 
3. **teleop_server**		- Pi
4. gstreamer video server	- Pi
 
There are 8 hardware modules:

1. XBOX 360 gamepad		- Laptop	
2. Wireless xbox receiver	- Laptop

3. MPU6050 IMU			- Pi
4. Wifi dongle 1000mw		- Pi	
5. DCDC Converter		- Pi
6. Pi Camera			- Pi
7. Pi V1 Model B		- Pi
8. Pi servo interface harness	- Pi

Functionally there is a remote RC car and a laptop operator control unit (Laptop).

NOTE - all IP addressese are for ref only.
192.168.1.1 - my router
192.168.1.2 - my laptop linux VM
192.168.1.6 - my PI

##usage
Acquire the windows XBOX wireless driver and install the drivers.  There's a tutorial here http://www.s-config.com/archived-xbox-360-receiver-install-for-win-xp-and-win-7/ if you buy a cheap Chinese copy and can't get it working.
The LED on the controller will indicate when it's paired.

Run the program inside a linux VM, get the controller running in windows then capture it in VMWAREs device capture menu.  The LED ont he controller should stay the same indicating it's working.

Install the client code on the laptop then the server code on the raspberry pi.  The Pi needs the Monitor code installing on it too.

Setup the Pi to boot it's server on startup as with the Monitor webcam device.  Wire up the Pi as per the attached schematic, power up.  Then boot the client code, you should be able to control the car and receive haptic feedback off the controller.


##Limitations
1. Currently if the server restarts the client doesn't reconnect.  Since this is just test code I think it's ok to relainch the client
2. Currently the settings are passed by argument, this is a bit of a mess and should move over to ini files.
3. Error checking is limited.
4. Joystick device enumeration is hard coded e.g. js0 event3 etc.


##XBOX 360 controller testing
This has been tested on Xubuntu 15.  Use the following commands to look for or test the XBOX controller.

1. use cat /proc/bus/input/devices, look at 'Handlers' if unsure!
2. fftest /dev/input/event3
3. jstest /dev/input/js0
4. ls /dev/input
 

##Installing Gstreamer on the Pi
Install motion on the pi as follows:

For gstreamer generally I used:
https://sparkyflight.wordpress.com/2014/02/22/raspberry-pi-camera-latency-testing-part-2/
http://pi.gbaman.info/?p=150
as a reference.

For the picamera:
https://www.youtube.com/watch?v=T8T6S5eFpqE

install gstreamer
1. Fit pi camera
2. I tend to use DHCP, to find out where everything is pull up your router on a browser
3. (e.g.) http://192.168.1.1
4. Navigate to the list of connected clients, you will see your laptop and the pi.
5. If you are using a VM set networking to bridged in your VM to ensure you are ont the same IP range.
6. If you have issues with the above check your firewall, add exceptions as required
7. (ssh to pi e.g. IP addr) sudo pi@192.168.1.6 
8. (Enable picam in bios) sudo raspi-config
9. run some basic jpg tests
10. (on pi & laptop run) sudo apt-get update
11. (on pi & laptop run) sudo apt-get install gstreamer1.0

![](https://github.com/lawsonkeith/Teleop2/raw/master/images/router.png)

Once you are happy you can test the Teleop2 script

1. cd ~/Teleop2
2. ./runclient.sh 192.168.1.6
	
##Git/misc cmds
Usefull cmds:

1. git clone https://github.com/lawsonkeith/Teleop2.git
2. git push origin master
3. git commit -am "comment"
4. git pull 
5. scp teleop_server.c pi@192.168.1.8:/home/pi/Teleop/server (copy to pi)
6. make | head
7. git reset --hard origin/master (force local to repo ver)

##UDP tests
send data to a client (Pi) interactively:

1. ncat -vv 192.168.1.6 8888 -u
	
look for open port on:

1. netstat -u -a
2. netstat -lnpu

##nano
Some usefull nano cmds...

1. CTRL+6 block sel
2. CTRL+6 copy
3. CTRL+K cut
4. CTRL+U uncut
5. F4 		SEL DN

##references
Usfull notes etc.
Linux timers - 2net.co.uk: periodic tasks in linux
IMU pulled from PiBits repo.

##Pi Cmds
General housekeeping..

sudo apt-get update
sudo apt-get dist-upgrade
sudo apt-get instal raspberrypi-ui-mods

##MPU6050
http://www.instructables.com/id/Reading-I2C-Inputs-in-Raspberry-Pi-using-C/?ALLSTEPS

1. Install i2c tools...
2. sudo apt-get install libi2c-dev
3. Edit the i2c On the pi's BIOS then reboot.
4. sudo raspi-config
5. sudo nano /etc/modules
6. sudo nano /etc/modprobe.d/raspi-blacklist.conf 
7. sudo i2cdetect -y 1
8. The MPU605 should appear as 68


##Pi Blaster
The PWM on the Pi is done with the Pi blaster Daemon.  In this the Pi receives commands over the FIFO.

1. sudo apt-get install autoconf
2. git clone https://github.com/sarfata/pi-blaster.git
3. cd pi-blaster
4. ./autogen.sh
5. ./configure
6. make
7. sudo make install
8. sudo make uninstall - to stop auto start

FIFO is at /dev/pi-blaster

GPIO number| Pin in P1 header
--- | ---
4    |     P1-7
17   |    P1-11
18   |     P1-12
21   |   P1-13
22   |    P1-15
23   |    P1-16
24   |    P1-18
25   |    P1-22
      
To completely turn on GPIO pin 17:

* echo "17=1" > /dev/pi-blaster

To set GPIO pin 17 to a PWM of 20%

* echo "17=0.2" > /dev/pi-blaster

##wifi
Follow adafruits guide to seting up the wifi using the terminal on the Pi.

1. sudo apt-get install avahi-daemon


##Refs
http://beej.us/guide/bgipc/output/html/singlepage/bgipc.html#fork
http://gnosis.cx/publish/programming/sockets2.html
http://www.2net.co.uk/tutorial/periodic_threads

