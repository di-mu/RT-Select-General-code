#undef DEBUG
#define DEBUG 0
#define DEV_2420

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "contiki.h"
#include "dev/watchdog.h"
#include "dev/radio.h"
#include "net/netstack.h"
#include "net/rime/rime.h"

#ifdef DEV_2420
#include "dev/uart1.h"
#define UART_WB uart1_writeb
#else
#include "dev/cc26xx-uart.h"
#define UART_WB cc26xx_uart_write_byte
#endif

#define SEND NETSTACK_RADIO.send
#define RECV NETSTACK_RADIO.read
#define TURN(X) NETSTACK_RADIO.set_value(RADIO_PARAM_POWER_MODE,X)

static uint8_t packet[64];
static uint8_t rem = 0;

static int ugetc(unsigned char c) {
	//if (c == 0x00) TURN(0);
	//else if (c == 0xff) TURN(1);
	//while (c--) SEND (packet, sizeof(packet)); //3.228ms/pkt for 2420
	rem = c;
	return 0;
}

static void uart_init() {
#ifdef DEV_2420
	uart1_set_input (ugetc);
#else
	cc26xx_uart_init();
	cc26xx_uart_set_input (ugetc);
	printf("uart_inited\n");
#endif
}


PROCESS(mro_zigbee_process, "mro+");
AUTOSTART_PROCESSES(&mro_zigbee_process);

PROCESS_THREAD(mro_zigbee_process, ev, data)
{
	int i; radio_value_t value;
	clock_time_t ts;
	PROCESS_BEGIN();
	watchdog_stop();
	NETSTACK_RADIO.set_value (RADIO_PARAM_RX_MODE, 0);
	NETSTACK_RADIO.get_value (RADIO_PARAM_TXPOWER, &value);
	printf("TX power: %3d dBm\n", value);
	for (i=0; i<sizeof(packet); i++) packet[i] = rand();
	uart_init();
	TURN(1);
	printf("Ready\n");
	while(1) {
		if(rem) {
			ts = clock_time();
			while(rem) { SEND (packet, sizeof(packet)); rem--; } //4.196ms/pkt for 2650
			printf("%lu\n", (clock_time() - ts) * 1000 / CLOCK_SECOND);
		}
		else {
			clock_wait(1); //must
		}
	}
	//PROCESS_WAIT_EVENT_UNTIL(0);
	PROCESS_END();
}
