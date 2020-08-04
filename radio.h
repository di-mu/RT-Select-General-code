#ifndef __RADIO_H__
#define __RADIO_H__

#include "mro.h"
#include "lora/arduPiLoRaWAN.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pcap.h>
#include <time.h>
#include <fcntl.h>
#include <termios.h>

#define PORT_ER(ERID) extern_radio_uart[ERID], uart_buffer, sizeof(uart_buffer)
#define PORT_IR(IRID) wifi_pcap[IRID], wifi_packet[IRID], sizeof(wifi_packet) / N_IR
#define IOPUT(PORT,FUN) while(FUN(PORT) <= 0) unistd::usleep(1000)

typedef struct {
	int rid;
	unsigned long x;
} radio_cmd_t;

typedef struct {
	int scs;
	float thpt, etx;
} radio_result_t;

extern pcap_t *wifi_pcap[N_IR];
extern uint8_t wifi_packet[N_IR][1024];
extern char lora_packet[256];
extern int extern_radio_uart[N_ER];
extern char extern_radio_devname[N_ER][32];
extern char wifi_devname[N_IR][32];
extern const uint8_t wifi_localmac[N_IR][6];
extern const uint8_t wifi_targetmac[N_IR][6];

extern void radio_init();
extern void radio_end();
extern void radio_test();
extern uint16_t m_radio_send (unsigned long *);
extern void wifi_radio_init(int);
extern void extern_radio_init(int);
extern void lora_radio_init();

#endif
