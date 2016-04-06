## Protobot
Improved RC car teleop system with haptic feedback using C on a raspberry Pi.  The main improvement is the use of an HD camera rather than the XBOX USB camera which couldn't go past really low res.

<img src="https://github.com/lawsonkeith/Protobot/raw/master/images/Screenshot.png" width="700">

(HD pic missing off snapshot?)

This now uses gstreamer rather than mjpeg-streamer to achieve this.  The old teleop used a chep wifi dongle, this can use either 
a 1000mW unit with an antennae for better range or a dongle for internal use.

<img src="https://github.com/lawsonkeith/Protobot/raw/master/images/DSC_0422.JPG" width="700">

There are 4 software modules:

1. **teleop_client**		- Laptop
2. gstreamer video client	- Laptop 
3. **teleop_server**		- Pi
4. gstreamer video server	- Pi
 
There are 9 hardware modules:

1. XBOX 360 gamepad		- Laptop	
2. Wireless xbox receiver	- Laptop
3. MPU6050 IMU			- Pi
4. Wifi dongle / 1000mw		- Pi	
5. DCDC Converter		- Pi
6. Pi Camera			- Pi
7. Pi V1 Model B		- Pi
8. Pi servo interface harness	- Pi
9. Arduino Nano IO module	- Pi (future)

![](https://github.com/lawsonkeith/Teleop/raw/master/images/Schematic.pdf)

Functionally there is a remote RC car and a laptop operator control unit (Laptop).

NOTE - all IP addressese are for ref only.
* 192.168.1.1 - my router
* 192.168.1.2 - my laptop - linux VM
* 192.168.1.4 - my laptop - win 7
* 192.168.1.6 - my PI

##Usage
1. Start Xbox controller, check it works in windows
2. Start linux VM, capture Xbox controller
3. Check your linux IP address via router & ifconfig
4. Turn on car, locate it's IP address on router
5. Run client script (./runclient.sh PI_IPADDR) specifying PI's IP address
6. Drive around...


##General installation 

1. Setup the PI wifi
2. Get XBOX xontroller working in windows
3. Run linux in a VM, get the controller running in windows then capture it in VMWAREs device capture menu
4. The LED on the controller should stay the same indicating it's working.  Check with jstest
5. Install the client code on the laptop then the server code on the raspberry pi via github
6. Recompile both using make; resolve and dependencies
7. Get the picamera working over gstreamer and shell commands  
8. Wire up the Pi as per the attached schematic, power up
9. Check servos work
9. Check the IMU works
10. Set pi to boot automatically
11. Then run the server then client code, you should be able to control the car and receive haptic feedback off the XBOX controller when the car crashes into walls etc


##Limitations
1. Currently if the server restarts the client doesn't reconnect.  Since this is just test code I think it's ok to relainch the client
2. Currently the settings are passed by argument, this is a bit of a mess and should move over to ini files.
3. Error checking is limited.
4. Joystick device enumeration is hard coded e.g. js0 event3 etc.


##XBOX 360 controller testing
Acquire the windows XBOX wireless driver and install the drivers.  There's a tutorial here http://www.s-config.com/archived-xbox-360-receiver-install-for-win-xp-and-win-7/ if you buy a cheap Chinese copy and can't get it working.
The LED on the controller will indicate when it's paired.  The drivers should already be on linux if it's a recent release, I used Xubuntu 15.  Use the following commands to look for or test the XBOX controller.

1. use cat /proc/bus/input/devices, look at 'Handlers' if unsure!
2. fftest /dev/input/event3, this tests force feedback.
3. jstest /dev/input/js0, this test analogs.
4. ls /dev/input, you should see the joystick FIFOs here.
5. I didn't have to install anything in linux (apart from maybe jstest), all the pain was in windows.
 
![](https://github.com/lawsonkeith/Protobot/raw/master/images/Capture2.JPG)

Test it in windows first, in 'devices and printers' you can check all the controls work on the gamepad.  Then test in linux with jstest and fftest.  

 
##Installing Gstreamer & picamera on the Pi
For gstreamer generally I used:
https://sparkyflight.wordpress.com/2014/02/22/raspberry-pi-camera-latency-testing-part-2/
http://pi.gbaman.info/?p=150
as a reference.

For the picamera:
https://www.youtube.com/watch?v=T8T6S5eFpqE

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

![](https://github.com/lawsonkeith/Protobot/raw/master/images/router.png)

Once you are happy you can test the Protobot script

1. cd ~/Protobot
2. ./runclient.sh 192.168.1.6

The server (robot) runs automatically once setup.


##Configure PI to run node app by default

We now need to configure the app to run as default when we power up the Pi.

1. sudo cp teleopserver.sh /etc/init.d/
2. sudo update-rc.d teleopserver.sh defaults

Reboot your Pi and check you can log onto the web page. You should now be ready to go.
	
	
##Git/misc cmds
Usefull cmds:

1. git clone https://github.com/lawsonkeith/Protobot.git
2. git push origin master
3. git commit -am "comment"
4. git pull 
5. scp teleop_server.c pi@192.168.1.8:/home/pi/Protobot/server (copy to pi)
6. make | head
7. git reset --hard origin/master (force local to repo ver)
8. git mv old new


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
The PI uses a MPU6050 IMU.  First off you have to enable I2C on the pi, wire it up then test it using the commands in 'scripts'. Again there may be some dependencies.  The server transmits impacts back to the client to transmit to the operator as haptic feedback using the XBOX force feedback.

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
The PWM on the Pi is done with the Pi blaster Daemon.  The server sends data to the servos direct from the Pi and in software to the PI-BLASTER FIFO.

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
     
![](https://github.com/lawsonkeith/Protobot/raw/master/images/Raspberry-Pi-GPIO-pinouts.png)
      
To completely turn on GPIO pin 17:

* echo "17=1" > /dev/pi-blaster

To set GPIO pin 17 to a PWM of 20%

* echo "17=0.2" > /dev/pi-blaster

To test just use a multimeter and some echo commands, write down on paper what you need to send to your servos to get them to do what you want.  Check my server code for my values, yours will be close luckily you can control them with the 0-3.3V offered by the PI as the control signal is unipolar.

For the wiring loom I used an IDC connector and some 2.54mm header.

**NOTE** -
(1) I've had issues with this interfering with the PIs windows environment in the past with lockups so I don't tend to boot into the PI X windows interface.  
(2) sudo chown root test.sh, then sudo chmod +s test.sh to allow scripts to acces GPIO as non root


##Wifi
Follow adafruits guide to seting up the wifi using the terminal on the Pi.  I found it easier to do it via the
command line and had issues with the GUI in x windows. 

1. sudo cp interfaces /etc/network/


##Refs
http://beej.us/guide/bgipc/output/html/singlepage/bgipc.html#fork
http://gnosis.cx/publish/programming/sockets2.html
http://wwhttps://learn.adafruit.com/adafruits-raspberry-pi-lesson-3-network-setup/setting-up-wifi-with-occidentalis
