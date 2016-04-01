
/*
 * $teleop_client.c$
 *
 * K Lawson
 * 28 Sep 2015
 * 
 * 
 * Teleop controller 
 * -----------------
 * This program allows an XBOX 360 controller to controll a 1/10 RC car.
 *  
 * The RC car is controlled over wifi using a wireless controller plugged
 * into a laptop.  This then uses WIFI to communicate with raspberry PI on
 * the car.  
 * 
 * A periodic UDP message is used to send commands to the car and receive
 * status information back from the car.  Accelerometer info fromt he car is then 
 * used to control the dual shock on the hand controller.
 * 
 * The laptop is client and the car is the server.
 * 
 * An XBOX webcam on the car can be accessed on the laptop via a web browser.
 *  #firefox 192.168.1.6:8081
 *
 * 
 * The aim is to evaluate the use of XBOX tech for Teleop robotics
 * 
 * usage
 * -----
 * You must pass the joystick driver handles as well as the servers IP 
 * address. 
 * 
 * sudo ./teleop_client /dev/input/event3 /dev/input/js0 192.168.1.6
 *
 */


#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <linux/input.h>
#include <linux/joystick.h>
#include <arpa/inet.h>

 
#define BUFLEN 	512  //Max length of buffer
#define PORT 	8888   //The port on which to send data

#define BITS_PER_LONG (sizeof(long) * 8)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)    ((array[LONG(bit)] >> OFF(bit)) & 1)
#define NAME_LENGTH 128

void FF_Rumble(unsigned int magnitude);
void JS_Read(int *Fore, int *Port);
void FF_Init(char *device_file_name);
void JS_Init(char *joy_file_name);
void SetupTimer(void);
void Task_Init(void);
void UDP_Init(char *ip_address);
void UDP_Send(int Fore, int Port, int Wdog, int *Accel);
void die(char *s);
void LimitIntMag(int *n,int lim);

// Main shared structures
struct ff_effect effects;	
struct input_event play, stop;
int Fore,Port;
int fd_e,fd_j;
struct itimerval old;
struct itimerval new;
struct sockaddr_in si_other;
int sock;

// Define UDP msg type
struct TMsg {
	int	Fore;
	int	Port;
	int	Accel;
	int Wdog;
	int spare[10];
	char endmsg[10];	
}Msg;	
	 


	
// Main control program for Teleop program. 
//	
int main(int argc, char** argv)
{
	char event_file_name[64];
	char joy_file_name[64];
	char ip_address[64];
	static int Accel,Wdog,Count;
	
	/* Read args */
	if(argc != 4){
		printf("usage: teleop_client [event] [joystick] [server_IP]\n");
		printf("e.g.   sudo ./teleop_client /dev/input/event3  /dev/input/js0 192.168.1.6\n");
		exit(1);	
	}	
	strncpy(event_file_name, argv[1], 64);
	strncpy(joy_file_name, argv[2], 64);
	strncpy(ip_address, argv[3], 64);
	
	// Init timer task and joystick and UDP stack
	FF_Init(event_file_name);
	JS_Init(joy_file_name);
	UDP_Init(ip_address);


	// hand over to timed task
	while(1) {
		usleep(10000); //10Hz
		
		JS_Read(&Fore,&Port);
		UDP_Send(Fore, Port, Wdog++, &Accel);
		
		printf("\n%d,%d,%X,%X",Fore,Port,Accel,Wdog);
		if(Count>0)
			Count--;
		
		// Haptic feedback
		if(Accel > 280) {
			if(Count ==0) {
				Count = 3;
				FF_Rumble(Accel);
			}
		}
	}//loop
		
	sleep(2);
}//END main



// Send a UDP datagram to the server on the car.  This is done non blocking now so the 
// failed packets don't make the whole process hang
//
void UDP_Send(int Fore, int Port, int Wdog,  int *Accel)
{
	unsigned int  slen=sizeof(si_other);
	char buf[BUFLEN];
  
	Msg.Fore = Fore;
	Msg.Port = Port;
	Msg.Wdog = Wdog;
		
	//send the message
	if (sendto(sock, &Msg, sizeof(Msg) , 0 , (struct sockaddr *) &si_other, slen)==-1) 	{
		die("sendto()");
	}
	 
	//receive a reply and print it
	//clear the buffer by filling null, it might have previously received data
	memset(buf,'\0', BUFLEN);
	//try to receive some data, this is a NON! blocking call
	if(recvfrom(sock, &Msg, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == sizeof(Msg)) 	{
		*Accel = Msg.Accel;
	}
     
}//END UDP_Send	



// Initialise UDP to talk to the client
//
void UDP_Init(char *ip_address)
{   
	int flags = fcntl(sock, F_GETFL);
	
	//open socket	
    if ( (sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        die("UDP_Init: socket");
    }
    
    // set rcvfrom to nonblocking else this program hangs
	flags |= O_NONBLOCK;
	fcntl(sock, F_SETFL, flags);
 
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
     
    if (inet_aton(ip_address , &si_other.sin_addr) == 0) {
		die("UDP_Init: aton");
    }
}//END UDP_Init



// Setup a 20 Hz Timer to poll the server on
// use POSIX timers
//
void Task_Init(void)
{
	signal (SIGALRM, TimedTask);
	new.it_interval.tv_sec = 0; 
	new.it_interval.tv_usec = 50000; //50ms = 20Hz = 50000us
	new.it_value.tv_sec = 0;
	new.it_value.tv_usec = 50000;
   
	old.it_interval.tv_sec = 0;
	old.it_interval.tv_usec = 0;
	old.it_value.tv_sec = 0;
	old.it_value.tv_usec = 0;
   
	if (setitimer (ITIMER_REAL, &new, &old) < 0) {
      die("Task_Init: timer init failed\n");
	}
	
	strcpy(Msg.endmsg,"EndMsg!");
}//END Task_Init


// JS_Read  Left axis is throttle, right axis is steering
// [1] -1K  = max fore 	[3] -1K = max port
//
void JS_Read(int *Fore, int *Port)
{
	unsigned char axes = 6;
	unsigned char buttons = 4;
	struct js_event js;
	static int axis[100];
	static char button[100];
	char  x;
	
	//read
	if (read(fd_j, &js, sizeof(struct js_event)) != sizeof(struct js_event)) {
		;//NO DATA
	}
	else{

		//btn or axes msg?
		switch(js.type & ~JS_EVENT_INIT) {
		case JS_EVENT_BUTTON:
			button[js.number] = js.value;
			break;
		case JS_EVENT_AXIS:
			axis[js.number] = js.value;
			break;
		}

		//read off the 2 vals we are interested in
		if (axes) {
			*Fore = axis[1] / 32.768;
			*Port = axis[3] / 32.768;
			
			LimitIntMag(Fore,1000);
			LimitIntMag(Port,1000);
		}

		if (buttons) {
			x = button[0];
		}
	}//END IF DATA
}//END JS_Read



// Initialise file handle to read joystick data from the gamepad
// 
void JS_Init(char *joy_file_name)
{
	unsigned char axes = 2;
	unsigned char buttons = 2;
	int version = 0x000800;
	char name[NAME_LENGTH] = "Unknown";
	
	if ((fd_j = open(joy_file_name, O_RDONLY)) < 0) {
		die("JS_Init: Error opening gamepad, is it connected?");
	}
	
	//Set to nonblocking file IO
	fcntl(fd_j, F_SETFL,fcntl(fd_j, F_GETFL) |O_NONBLOCK);
	
	//Read joystick information
	ioctl(fd_j, JSIOCGVERSION, &version);
	ioctl(fd_j, JSIOCGAXES, &axes);
	ioctl(fd_j, JSIOCGBUTTONS, &buttons);
	ioctl(fd_j, JSIOCGNAME(NAME_LENGTH), name);
	
	printf("Joystick (%s) has %d axes and %d btns. Kernel driver version %d.%d.%d.\n",
		name, axes, buttons, version >> 16, (version >> 8) & 0xff, version & 0xff);
	printf("Run ... (Ctrl+C to exit)\n");
}//END JS_Init
	


// Initialise the Force feedback device handlers to write data to gamepad
// open file handle.
//
void FF_Init(char *device_file_name)
{
	unsigned long features[4];
	int n_effects;	/* Number of effects the device can play at the same time */

	/* Open device */
	fd_e = open(device_file_name, O_RDWR);
	if (fd_e == -1) {
		die("Open device file");
	}
	printf("Device %s opened\n", device_file_name);
	
	/* Query device */
	if (ioctl(fd_e, EVIOCGBIT(EV_FF, sizeof(unsigned long) * 4), features) < 0) {
		die("Ioctl query");
	}
	if (ioctl(fd_e, EVIOCGEFFECTS, &n_effects) < 0) {
		die("Ioctl number of effects");
	}
	if(n_effects > 10)
		printf("Found XBOX 360 controller with [%d] effects\n", n_effects);
}//END FF_Init




// Set force feedback to rumble joypad to 0-1000 magnitude.
// we need to bu root to do this. Note - see fftest.c for the code behind this.
// 
void FF_Rumble(unsigned int magnitude)
{
	static int init = 1;
	
	if(init = 1) {
		init = 0;
		
		/* download a periodic sinusoidal effect & store for futuer playback */
		effects.type = FF_PERIODIC;
		effects.id = -1;
		effects.u.periodic.waveform = FF_SINE;
		effects.u.periodic.period = 0.1*0x100;	/* 0.1 second */
		effects.u.periodic.magnitude = magnitude * 32;	/* 0.5 * Maximum magnitude */
		effects.u.periodic.offset = 0;
		effects.u.periodic.phase = 0;
		effects.direction = 0x4000;	/* Along X axis */
		effects.u.periodic.envelope.attack_length = 0x100;
		effects.u.periodic.envelope.attack_level = 0;
		effects.u.periodic.envelope.fade_length = 0x100;
		effects.u.periodic.envelope.fade_level = 0;
		effects.trigger.button = 0;
		effects.trigger.interval = 0;
		effects.replay.length = 500;  /* .5 seconds */
		effects.replay.delay = 0;

		if (ioctl(fd_e, EVIOCSFF, &effects) < 0) {
			die("Upload effects[0]");
		}
	}//END init 
	
	LimitIntMag(&magnitude,1000);
	
	play.type = EV_FF;
	play.code = effects.id;
	play.value = 1;

	if (write(fd_e, (const void*) &play, sizeof(play)) == -1) {
		perror("\nFF_Rumble(): Play effect");
	}
}//END FF_Rumble
   
   
   
// Does what it says...
//   
void die(char *s)
{
    perror(s);
    exit(1);
    
}//END die


// Limit int Mag
// limit n to +/- lim
void LimitIntMag(int *n,int lim)
{
	if(*n > lim)
		*n = lim;
	
	if(*n < (lim * -1))
		*n = (lim * -1);
		
}//END LimitIntMag

