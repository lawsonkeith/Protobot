
/*
 * $teleop_server.c$
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
 * 
 * The aim is to evaluate the use of XBOX tech for Teleop robotics.
 * 
 * This module is the server; it waits for a connection with the client, goes live then
 * feeds back accelerometer data to the OCU.  This module works in parallel with the 
 * webcam daemon Monitor although they are loosely coupled.
 * 
 * usage
 * -----
 * sudo ./teleop_server
 *
 */

#include <stdio.h> //printf
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <stdint.h>
#include <unistd.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include <fcntl.h>

#define T 5 
#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data

#define GPIO_PORT 18
#define GPIO_FORE 17
#define GPIO_RED_LED 24
#define GPIO_GN_LED 25
//default  high GPIO >> GPIO0/2, GPIO1/3, GPIO4, GPIO7 and GPIO8.

// Define UDP msg type
struct TMsg {
	int	Fore;
	int	Port;
	int	Accel;
	int Wdog;
	int spare[10];
	char endmsg[10];	
}Msg;	

void die(const char *s);
int main(void);
void sigalrm_handler(int);
void GPIO_init(void);
void GPIO_drive(int Fore,int Port,int Wdog);
void GPIO_disable(void);
void Accel_init(void);
void Accel_read(int *Accel);
void PiBlast(int channel, float value);
void PiBlast_init(void);
void I2C_init(void);
void LimitInt(int *n,int lo,int hi);
void LimitIntMag(int *n,int lim);
int fp;
MPU6050 accelgyro;
char AlarmMsgEn = 1;

// Main server program.  This is polled by the client so the client controls the 
// update speed.
// 
int main(void)
{
    struct sockaddr_in si_me, si_other;
    int Accel;
    unsigned int slen =  sizeof(si_other);
    int s, i, recv_len;
    char buf[BUFLEN];

	if(sizeof(buf) < sizeof(Msg))
		die("\nmain: Fatal build error");

    //create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        die("\nsocket()");
    }
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    //bind socket to port
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1){
        die("bind()");
    }
    //Start alarm
    signal(SIGALRM, sigalrm_handler);   
    alarm(1);   
     
    PiBlast_init();
    GPIO_disable();
    Accel_init();
    
    //PiBlast(17,0.15);
    //usleep(1000000);
    
    //keep listening for data
    while(1)
    {
        //printf("Waiting for data...");
        //fflush(stdout);
         
        //try to receive some data, this is a blocking call @@@@@@ WAIT UDP @@@@@@@@
        if ((recv_len = recvfrom(s, buf /*(void *)Msg*/, sizeof(buf), 0, (struct sockaddr *) &si_other, &slen)) == -1)  {
            die("recvfrom()");
        }
        AlarmMsgEn = 1;
        memcpy(&Msg,buf,sizeof(Msg));
        
        Accel_read(&Msg.Accel);
        GPIO_drive(Msg.Fore,Msg.Port,Msg.Wdog);
         
        //print details of the client/peer and the data received
        //printf("\nReceived packet %x from %s:%d - A%d",Msg.Wdog, inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port),Msg.Accel);
        //printf("Data: %s\n" , buf);
        
        memcpy(buf,&Msg,sizeof(Msg));
         
        //now reply the client with the same data
        if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1) {
            die("sendto()");
        }
        
        //kick alarm
        alarm(1);
    }//END while
 
    close(s);
    return 0;
}// End main



// This function initialises the MPU6050
// 655 = me shaking violently
void Accel_init(void)
{
    printf("\nInvitializing I2C devices...");
    accelgyro.initialize();
	
}//END ACCEL_Init


	
    
// This function reads acceleration form the MPU 6050 and returns impacts
// 255-1000;
void Accel_read(int *Accel)
{
	int16_t ax, ay, az;
	int16_t gx, gy, gz;
	static int x;
	
	static int lax, lay, laz;	
	int s1,s2,s3;
	
    // read raw accel/gyro measurements from device
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // display accel/gyro x/y/z values
	s1= abs(ax-lax) / 100;
	s2 = abs(ay-lay) / 100;
	s3 = abs(az-laz) / 100 ;

	//get maxval
	if((s1 > s2) && (s1 > s3))	
		*Accel = s1;
	else if((s2 > s1) && (s1 > s3))	
		*Accel = s2;
	else
		*Accel =s3;

	//fflush(stdout);		
	//if(max > 254)
	//	printf("\n%d",max);
	
	//update last vals
	laz = az;
	lay = ay;
	lax = ax;
	
	// > 255 would be a servere jolt
	LimitInt(Accel,0,1000);
	
}//END ACCEL_Read



// This function sets the GIO on, off or to a PWM value
// 1=on 0=off 0.nn=PWM
//
void PiBlast(int channel, float value)
{
	char str[30];
	
	sprintf(str,"%d=%1.2f\n",channel,value);
	//string s = channel + "=" + value + "\n";	
	//printf("%s\n",str);

	write(fp, str, strlen(str) );

}//END PiBlast



// GPIOinit
//
void PiBlast_init(void)
{	
	fp = open("/dev/pi-blaster",O_WRONLY);
	if (fp < 1) {
		die("\nGPIO_Init: Error opening Pi-blaster FIFO");
	}
    
}//END PiBlast_init




// sigalrm_handler, called if we don't get UDP comms
//
void sigalrm_handler(int sig)
{
	//printf("\nsigalrm_handler: ALARM!");
    GPIO_disable();
    alarm(1);
}//END sigalrm_handler




// GPIO_drive, actually drive the car.
//
void GPIO_drive(int Fore,int Port,int Wdog)
{
	static int LastWdog,count;
	float fport, ffore;
	
	LimitIntMag(&Fore,1000);
	LimitIntMag(&Port,1000);
	
	//from picar...
	//piblaster.setPwm(17,0.105); //throttle rev  1000 
	//piblaster.setPwm(17,0.14); //throttle n
	//piblaster.setPwm(17,0.175); //throttle fwds -1000
	//-0.000035 * x + .14
	ffore = -0.000035 * Fore + .14;
	
	//18 BLACK TAPE
	//piblaster.setPwm(18,.105); // steer l -1000
	//piblaster.setPwm(18,.14); // steer mid
	//piblaster.setPwm(18,.175); // steer r 1000
	// 0.000035 * x + .14
	fport = 0.000035 * Port + .14;
	
	//client -=fore -=port  +/- 1000 Mag
	//
	if(Wdog == LastWdog) {
		printf("\nGPIO_drive: Error watchdog");
		fport = .14;
		ffore = .14;
	}
	
	//flash
	/*count++;
	if(count >= 10) {
		PiBlast(GPIO_RED_LED,1);
	} else if (count > 20) {
		PiBlast(GPIO_RED_LED,0);
		count = 0;
	}*/
	
	//printf("\n%f, %f",fport, ffore);
	PiBlast(GPIO_PORT,fport);
	PiBlast(GPIO_FORE,ffore);
	
	LastWdog = Wdog;
}//END GPIO_drive



// GPIO_disable
//
void GPIO_disable(void)
{
	if(AlarmMsgEn) {
		printf("\nGPIO_disable: DISABLE!");
		fflush(stdout);
	}
	
	AlarmMsgEn = 0;
	PiBlast(GPIO_PORT,0.14);
	PiBlast(GPIO_FORE,0.14);
	PiBlast(GPIO_RED_LED,0);
	PiBlast(GPIO_GN_LED,1);

}//END sigalrm_handler



// Does what it says...
//   
void die(const char *s)
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

// Limit int 
// limit n to lo & hi
void LimitInt(int *n,int lo,int hi)
{
	if(*n > hi)
		*n = hi;
	
	if(*n < lo)
		*n = lo;
		
}//END LimitInt
