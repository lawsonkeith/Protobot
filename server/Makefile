all: demo_raw teleop_server 

HDRS = I2Cdev.h 
CMN_OBJS = I2Cdev.o MPU6050.o
RAW_OBJS = demo_raw.o


# Set DMP FIFO rate to 20Hz to avoid overflows on 3d demo.  See comments in

CXXFLAGS = -DDMP_FIFO_RATE=9 -Wall -g -O2 

$(CMN_OBJS) $(DMP_OBJS) $(RAW_OBJS) : $(HDRS)

demo_raw: $(CMN_OBJS) $(RAW_OBJS)
	$(CXX) -o $@ $^ -lm

teleop_server: teleop_server.c $(CMN_OBJS) 
	$(CXX) -o $@ $^ -lm


clean:
	rm -f $(CMN_OBJS) $(DMP_OBJS) $(D3D_OBJS) $(RAW_OBJS) demo_raw teleop_server

