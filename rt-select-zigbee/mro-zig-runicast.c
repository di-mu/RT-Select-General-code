#define THIS_IS_2650

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "contiki.h"
#include "dev/radio.h"
#include "net/netstack.h"
#include "net/rime/rime.h"

#ifdef THIS_IS_2650
#include "dev/cc26xx-uart.h"
#define UART_WB cc26xx_uart_write_byte
#endif
#ifdef THIS_IS_SKY
#include "dev/uart1.h"
#define UART_WB uart1_writeb
#endif

#define MAX_RETRANSMISSIONS 7

static struct runicast_conn runicast;
static const linkaddr_t recv_addr = {.u16 = 0};
static unsigned long n_packets;
static clock_time_t ts_start, ts_deadline;
static uint8_t payload[64];
static uint8_t Ready = 0;

PROCESS(test_runicast_process, "runicast test");
AUTOSTART_PROCESSES(&test_runicast_process);

#define RADIO_SW(X) NETSTACK_RADIO.set_value(RADIO_PARAM_POWER_MODE, X)

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

static radio_feedback_t feedback;

static void radio_feedback() {
	int i; uint8_t *buffer = (uint8_t *) &feedback;
	for (i=0; i<sizeof(uart_buffer_t); i++) UART_WB(buffer[i]);
}

static void recv_runicast(struct runicast_conn *c, const linkaddr_t *from, uint8_t seqno) {
	static uint8_t last_seqno = 0xFF;
	static unsigned long rxcnt = 0;
	unsigned char *data;
	if (last_seqno == seqno) return;
	last_seqno = seqno;
	rxcnt++;
	data = packetbuf_dataptr();
	if (data[0]=='E' && data[1]=='n' && data[2]=='d') {
		printf("%lu packets received\n", rxcnt);
		last_seqno = 0xFF;
		rxcnt = 0;
	}
}

static void sent_runicast(struct runicast_conn *c, const linkaddr_t *to, uint8_t retransmissions) {
	static unsigned long pktcnt = 0, txcnt = 0;
	clock_time_t ts_now = clock_time();
	txcnt += retransmissions - 1;
	if (ts_now > ts_deadline || pktcnt == n_packets) {
		feedback.Success = (pktcnt == n_packets);
		if (ts_now > ts_start) feedback.Thruput = pktcnt / 2 * CLOCK_SECOND / (ts_now - ts_start);
		feedback.ETX = 1000 * txcnt / pktcnt;
		pktcnt = 0; txcnt = 0;
		RADIO_SW(0);
		radio_feedback();
		return;
	}
	pktcnt ++;
	runicast_send(&runicast, &recv_addr, MAX_RETRANSMISSIONS);
}

static void timedout_runicast(struct runicast_conn *c, const linkaddr_t *to, uint8_t retransmissions) {
	RADIO_SW(0);
	radio_feedback();
}

static void sendStart() {
	RADIO_SW(1);
	runicast_send(&runicast, &recv_addr, MAX_RETRANSMISSIONS);
}

static int sgetc(unsigned char data) {
	static uart_buffer_t uart_buffer;
	static int uart_count = 0;
	uart_buffer.buffer[uart_count++] = data;
	if (uart_count == sizeof(uart_buffer_t)) {
		uart_count = 0;
		n_packets = uart_buffer.command.n_packets * 2;
		ts_deadline = ts_start + uart_buffer.command.ms_deadline * CLOCK_SECOND / 1000;
		if (n_packets > 0) {
			ts_start = clock_time();
			Ready = 1;
		}
	}
	return 0;
}

static void uart_init() {
	#ifdef THIS_IS_2650
		cc26xx_uart_init();
		cc26xx_uart_set_input (sgetc);
	#endif
	#ifdef THIS_IS_SKY
		uart1_set_input (sgetc);
	#endif
}

static const struct runicast_callbacks runicast_callbacks = {recv_runicast, sent_runicast, timedout_runicast};

PROCESS_THREAD(test_runicast_process, ev, data) {
	int i; radio_value_t value;
	PROCESS_BEGIN();
	//watchdog_stop();
	runicast_open(&runicast, 144, &runicast_callbacks);
	NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &value);
	printf("TX power: %3d dBm\n", value);
	printf("%d bytes local address: %X\n", sizeof(linkaddr_t), linkaddr_node_addr.u16);
	if (linkaddr_node_addr.u16 == recv_addr.u16) {
		printf("I'm receiver\n");
		PROCESS_WAIT_EVENT_UNTIL(0);
	}
	RADIO_SW(0);
	for (i=0; i<sizeof(payload); i++) payload[i] = rand();
	packetbuf_copyfrom(payload, sizeof(payload));
	uart_init();
	PROCESS_WAIT_EVENT_UNTIL(0);
	while(1) {
		static struct etimer et1;
		etimer_set(&et1, 1);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et1));
		if (Ready) { Ready = 0; sendStart(); }
	}
	PROCESS_END();
}
