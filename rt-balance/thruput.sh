#!/bin/bash
> ts_w1
for ii in {1..10}
do
	./init 1000
	cat /proc/uptime >> ts_w1
	./wifi-1
	cat /proc/uptime >> ts_w1
done

> ts_w2
for ii in {1..10}
do
	./init 1000
	cat /proc/uptime >> ts_w2
	./wifi-2
	cat /proc/uptime >> ts_w2
done

> ts_z1
for ii in {1..10}
do
	./init 250
	cat /proc/uptime >> ts_z1
	./zigbee-1
	cat /proc/uptime >> ts_z1
done

> ts_z2
for ii in {1..10}
do
	./init 250
	cat /proc/uptime >> ts_z2
	./zigbee-2
	cat /proc/uptime >> ts_z2
done
