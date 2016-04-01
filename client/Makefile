all: teleop_client 

clean:
	rm -f teleop_client telop_client.o 

teleop_client: teleop_client.c 
	$(CC) -o $@ $< -Wl,-rpath,. -L.  -ludev


install: libgamepad.so

.PHONY: all clean install
