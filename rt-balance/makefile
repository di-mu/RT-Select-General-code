all:
	gcc init.c -o init
	gcc twin.c -DRID=\"w1\" -DWIFI=\"wlx9cefd5feb371\" -DPGS=40 -lpcap -o wifi-1
	gcc twin.c -DRID=\"w2\" -DWIFI=\"wlx28a1eb9aa738\" -DPGS=40 -lpcap -o wifi-2
	gcc twin.c -DRID=\"z1\" -DZIGBEE=\"\/dev\/ttyUSB0\" -DPGS=5 -DZIGPKTUS=3500 -o zigbee-1
	gcc twin.c -DRID=\"z2\" -DZIGBEE=\"\/dev\/ttyUSB2\" -DPGS=5 -DZIGPKTUS=4500 -o zigbee-2
