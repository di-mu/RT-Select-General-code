#!/bin/bash
> ts_w1
> ts_w2
> ts_z1
> ts_z2
for ii in {1..100}
do
	./init 2000
	sleep 1
	./wifi-1 &
	./wifi-2 &
	./zigbee-1 &
	./zigbee-2 &
	sleep 3
done
