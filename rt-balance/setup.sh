#!/bin/bash

ifconfig wlan0 down

ifconfig wlx9cefd5feb371 down
iwconfig wlx9cefd5feb371 mode Ad-hoc essid ad_hoc_1 channel 1
ifconfig wlx9cefd5feb371 inet 192.168.22.1/24 up
iwconfig wlx9cefd5feb371 txpower 20

ifconfig wlx28a1eb9aa738 down
iwconfig wlx28a1eb9aa738 mode Ad-hoc essid ad_hoc_2 channel 6
ifconfig wlx28a1eb9aa738 inet 192.168.22.2/24 up
iwconfig wlx28a1eb9aa738 txpower 20

modprobe ftdi_sio vendor=0x0403 product=0xa6d1
chmod 666 /sys/bus/usb-serial/drivers/ftdi_sio/new_id
echo 0403 a6d1 > /sys/bus/usb-serial/drivers/ftdi_sio/new_id

screen /dev/ttyUSB0 115200
screen /dev/ttyUSB2 115200
