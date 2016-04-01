#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include <stdlib.h>

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 accelgyro;

int16_t ax, ay, az;
int16_t gx, gy, gz;

void setup() {
    // initialize device
    printf("Initializing I2C devices...\n");
    accelgyro.initialize();

    // verify connection
    printf("Testing device connections...\n");
    printf(accelgyro.testConnection() ? "MPU6050 connection successful\n" : "MPU6050 connection failed\n");
}

void loop() {

    static int lax, lay, laz;	
	int s1,s2,s3,max;
    // read raw accel/gyro measurements from device
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // these methods (and a few others) are also available
    //accelgyro.getAcceleration(&ax, &ay, &az);
    //accelgyro.getRotation(&gx, &gy, &gz);

    // display accel/gyro x/y/z values
	s1= abs(ax-lax) / 100;
	s2 = abs(ay-lay) / 100;
	s3 = abs(az-laz) / 100 ;

	if((s1 > s2) && (s1 > s3))	
		max = s1;
	else if((s2 > s1) && (s1 > s3))	
		max = s2;
	else
		max =s3;

	fflush(stdout);		

	if(max > 254)
		printf("\n%d",max);

	laz = az;
	lay = ay;
	lax = ax;
}

int main()
{
    setup();
    for (;;)
        loop();
}

