ll: teleop_client teleop_server

clean:
	rm -f teleop_client telop_client.o teleop_server telop_server.o

teleop_client: teleop_client.c 
	$(CC) -o $@ $< -Wl,-rpath,. -L.  -ludev

teleop_server: teleop_server.c 
	$(CC) -o $@ $< -Wl,-rpath,. -L.  -ludev

install: libgamepad.so

.PHONY: all clean install
