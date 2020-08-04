#include "radio.h"

int main(int argn, char *argv[]) {
	int rid, Acked = 0; long rxcnt = 0;
	const u_char *packet_recv;
	u_char packet_ack[64] = {0};
	struct pcap_pkthdr *header;
	uint32_t pktlbl;
	if (argn != 2) { printf("usage: %s <radio id>\n", argv[0]); exit(0); }
	rid = atoi (argv[1]);
	if (rid < N_IR) {
		wifi_radio_init (rid);
		while (1) {
			while (1) {
				while (pcap_next_ex (wifi_pcap[rid], &header, &packet_recv) <= 0);
				pktlbl = * (uint32_t *) (packet_recv + offset_lbl);
				if (pktlbl == ENDLBL) break;
				if (pktlbl != DATLBL) continue;
				if (Acked) { Acked = 0; rxcnt = 0; }
				rxcnt ++;
			}
			memcpy (packet_ack, packet_recv + 6, 6);
			memcpy (packet_ack + 6, packet_recv, 6);
			memcpy (packet_ack + offset_lbl, &ACKLBL, sizeof(ACKLBL));
			memcpy (packet_ack + offset_data, &rxcnt, sizeof(rxcnt));
			pcap_inject (wifi_pcap[rid], (void *)packet_ack, 64);
			Acked = 1;
		}
	}
	else if (rid < N_IR + N_ER) {
		extern_radio_init (rid - N_IR);
		unistd::write (extern_radio_uart[rid - N_IR], "R", 1);
	}
	else if (rid < _M_) {
		lora_radio_init();
		packet_recv = LoRaWAN._buffer;
		while (1) {
			while (1) {
				while (LoRaWAN.receiveRadio (250));
				if (packet_recv[0] == 'F') break;
				if (Acked) { Acked = 0; rxcnt = 0; }
				rxcnt ++;
			}
			sprintf ((char *)packet_ack, "%X", rxcnt);
			LoRaWAN.sendRadio ((char *)packet_ack);
			Acked = 1;
		}
	}
}