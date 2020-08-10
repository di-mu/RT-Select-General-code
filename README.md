# RT-Select-General

RT-Select-General is the system implementation of the radio selection and data partitioning solution presented in papers
"Energy-Efficient Radio Selection and Data Partitioning for Real-Time Data Transfer" (DCOSS'19) and
"Radio Selection and Data Partitioning for Energy-Efficient Wireless Data Transfer in Real-Time IoT Applications" (Ad Hoc Networks '20).

## Setup on Raspberry Pi

1. Install package "libpcap-dev" on Raspberry Pi;
2. To compile RT-Select-General, run "make ondevice" in this directory;
3. To compile RT-Balance, run "make" in "rt-balance" directory.

## Setup on ZigBee devices

1. Download Contiki OS system from https://github.com/contiki-os/contiki ;
2. Copy "rt-select-zigbee" or "rt-balance-zigbee" directory to "contiki/examples/";
3. Replace the defined macro in the first line of "mro-zig.c" or "mro+.c" with DEV_SKY for TelosB (CC2420) device or DEV_NEW for CC2650 device;
4. Run "make TARGET=sky" for TelosB device or "make TARGET=srf06-cc26xx BOARD=srf06/cc26xx" for CC2650 device;
5. Program the flash memory of the target device with the generated binary (*.hex or *.sky) files.

## Execution of RT-Select-General

```bash
# ./init-serial.sh
# ./mro
```

## Execution of RT-Balance

```bash
# ./setup.sh
# ./run.sh
```
