#define DEV_SKY

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "contiki.h"
#include "dev/radio.h"
#include "net/netstack.h"
#include "net/rime/rime.h"

#ifdef DEV_NEW
#include "dev/cc26xx-uart.h"
#define UART_WB cc26xx_uart_write_byte
#endif
#ifdef DEV_SKY
#include "dev/uart1.h"
#define UART_WB uart1_writeb
#endif

#define RADIO_SW(X) NETSTACK_RADIO.set_value(RADIO_PARAM_POWER_MODE, X)
#define SEND NETSTACK_RADIO.send
#define RECV NETSTACK_RADIO.read
#define GATEWAY 0

static int32_t n_packets, ts_start, ts_deadline;
static uint8_t Ready = 0;
static uint8_t packet[64];
/*
typedef struct {
	uint16_t Success;
	uint32_t Thruput;
	uint16_t ETX;
} radio_feedback_t;

typedef struct {
	uint32_t n_packets;
	uint32_t ms_deadline;
} radio_command_t;

typedef union {
	uint8_t buffer[8];
	radio_command_t command;
	radio_feedback_t feedback;
} uart_buffer_t;

const uint32_t DATLBL = 0x12345678;
const uint32_t ENDLBL = 0x87654321;
const uint32_t ACKLBL = 0xac0ac0ac;

static void radio_feedback() {
	int i; for (i=0; i<8; i++) UART_WB(feedback.buffer[i]);
}

static void recv(uint8_t *packet) {
	static uint32_t ackpkt[2];
	uint32_t *rcvpkt = (uint32_t *) packet;
	if (rcvpkt[0] == DATLBL) {
		if (Acked) { Acked = 0; rxcnt = 0; }
		rxcnt ++;
	}
	else if (rcvpkt[0] == ENDLBL) {
		ackpkt[0] = ACKLBL;
		ackpkt[1] = rxcnt;
		SEND (ackpkt, sizeof(ackpkt));
		Acked = 1;
	}
	else if (rcvpkt[0] == ACKLBL && Acked == 0) {
		Acked = 1;
		rxcnt = rcvpkt[1];
		remain -= rxcnt;
		if (remain < 0) remain = 0;
		if (remain > 0 && scs) {
			retry += remain;
			Ready = 1;
		}
		else {
			RADIO_SW (0);
			feedback.feedback.Success = scs;
			feedback.feedback.Thruput = (n_packets - remain) / 2 * CLOCK_SECOND / (clock_time() - ts_start);
			feedback.feedback.ETX = 1000 + 1000 * retry / (n_packets - remain);
			radio_feedback();
			remain = 0;
			retry = 0;
			scs = 1;
		}
	}
}
*/
static uint8_t uart_buffer[8];

static int sgetc(unsigned char data) {
	static int uart_count = 0;
	uart_buffer[uart_count++] = data;
	if (uart_count == 8) {
		uart_count = 0;
		n_packets = (* (uint32_t *) (uart_buffer)) * 2;
		ts_start = clock_time();
		ts_deadline = ts_start + (* (uint32_t *) (uart_buffer + 4)) * CLOCK_SECOND / 1000;
		if (n_packets > 0) Ready = 1;
	}
	return uart_count;
}

static void uart_init() {
#ifdef DEV_NEW
	cc26xx_uart_init();
	cc26xx_uart_set_input (sgetc);
#endif
#ifdef DEV_SKY
	uart1_set_input (sgetc);
#endif
}

void startsend() {
	int i;
	//memcpy (packet, &DATLBL, sizeof(DATLBL));
	for (i=0; i < n_packets; i++) {
		if (clock_time() > ts_deadline) break;
		SEND (packet, sizeof(packet));
	}
	RADIO_SW (0);
	UART_WB (i == n_packets ? 0xff : 0x00);
	//printf("%ld\n", (i / 2) * CLOCK_SECOND / (clock_time() - ts_start));
	/*do {
		SEND (&ENDLBL, sizeof(ENDLBL));
		for (i=0; i<250; i++) {
			if (RECV (packet, sizeof(packet)) > 0) {
				recv (packet);
				if (Acked) break;
			}
		}
	} while (Acked == 0);*/
}

PROCESS(mro_zigbee_process, "mro-zigbee");
AUTOSTART_PROCESSES(&mro_zigbee_process);

PROCESS_THREAD(mro_zigbee_process, ev, data)
{
	int i; radio_value_t value;
	PROCESS_BEGIN();
	//watchdog_stop();
	NETSTACK_RADIO.set_value (RADIO_PARAM_RX_MODE, 0);
	NETSTACK_RADIO.get_value (RADIO_PARAM_TXPOWER, &value);
	printf("TX power: %3d dBm\n", value);
	RADIO_SW (0);
	uart_init();
	for (i=0; i<sizeof(packet); i++) packet[i] = rand();
	while (1) {
		static struct etimer t1;
		etimer_set (&t1, 1);
		PROCESS_WAIT_EVENT_UNTIL (etimer_expired(&t1));
		if (Ready == 0 && GATEWAY == 0) continue;
		RADIO_SW (1);
		if (Ready) { Ready = 0; startsend(); }
		//while (GATEWAY) if (RECV (packet, sizeof(packet)) != 0) recv (packet);
	}
	PROCESS_END();
}
