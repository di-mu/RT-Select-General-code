#include "radio.h"

float D = 10.0;
pcap_t *wifi_pcap[N_IR];
uint8_t wifi_packet[N_IR][1024];
char lora_packet[256];  //makes 128-byte payload
int  extern_radio_uart[N_ER];
char extern_radio_devname[N_ER][32] = {
#if N_ER == 1
	"/dev/ttyUSB0"
#else
	"/dev/ttyUSB0", "/dev/ttyUSB2"
#endif
};
char wifi_devname[N_IR][32] = {
#if N_IR == 1
	"wlan0"
#else
	"wlan0", "wlan1"
#endif
};
const uint8_t wifi_localmac[N_IR][6] = {
#if N_IR == 1
	{0x28,0xa1,0xeb,0x9a,0xa7,0x38}
#else
	{0x28,0xa1,0xeb,0x9a,0xa7,0x38}, {0xb8,0x27,0xeb,0x5a,0xd9,0x0e}
#endif
};
const uint8_t wifi_targetmac[N_IR][6] = {
#if N_IR == 1
	{0x9c,0xef,0xd5,0xfe,0xae,0x54}
#else
	{0x9c,0xef,0xd5,0xfe,0xae,0x54}, {0xb8,0x27,0xeb,0xe5,0xf5,0xde}
#endif
};

#if M == 2
float TH[M] = {4000, 150};
float ETX[M] = {1.5, 1.0};
float Esw[M] = {0.32, 6.7e-6};
float Tsw[M] = {0.62, 0.3e-3};
float Prb[M] = {0.29, 0.09};
float Eta[M] = {31e-6, 24e-6};
#endif

#if M == 5
float TH[M] = {3000,	1250,	150,	115,	4};
float ETX[M] = {1.5,	1.5,	1.0,	1.0,	1.0};
float Esw[M] = {0.32,	0.10,	6.7e-6,	23e-6,	0.0};
float Tsw[M] = {0.62,	0.80,	0.3e-3,	1.4e-3,	0.0};
float Prb[M] = {0.29,	0.16,	0.09,	0.03,	0.0};
float Eta[M] = {31e-6, 13e-6, 24e-6, 39e-6, 47e-3};
#endif

/* Old
float TH[M] = {2500.0, 2500.0, 80.0, 300.0, 2.3};
float ETX[M] = {1.0, 1.0, 1.0, 1.0, 1.0};
float Esw[M] = {0.315, 0.327, 6.72E-6, 22.6E-6, 0.0};
float Tsw[M] = {0.619, 0.697, 0.264E-3, 1.4E-3, 0.0};
float Prb[M] = {0.287, 0.361, 0.086, 0.025, 0.0};
float Eta[M] = {30.8E-6, 23.9E-6, 23.6E-6, 39.1E-6, 47.4E-3};
*/
/*
const uint32_t DATLBL = 0x12345678;
const uint32_t ENDLBL = 0x87654321;
const uint32_t ACKLBL = 0xac0ac0ac;
const size_t offset_lbl = 40, offset_data = 50;

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
	radio_feedback_t radio_feedback;
	radio_command_t radio_command;
} uart_buffer_t;
*/

int bytes2int(const uint8_t *bytes, uint8_t size) {
	int result = 0; uint8_t i;
	for(i=0; i<size; ++i) { result <<= 8; result |= bytes[i]; }
	return result;
}

void int2bytes(int num, uint8_t *bytes, uint8_t size) {
	while(size) { bytes[--size] = num & 0xFF; num >>= 8; }
}

unsigned long us_stopwatch(uint8_t idx, uint8_t cmd) {
	static struct timespec ts[2][2];
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts[idx][cmd]);
	if (cmd==0) return (ts[idx][0].tv_sec - ts[idx][1].tv_sec) * 1000000
		+ (ts[idx][0].tv_nsec - ts[idx][1].tv_nsec) / 1000;
	return 0;
}

void ms_sleep(unsigned long ms) {
	struct timespec t = {ms/1000, (ms%1000)*1000000};
	nanosleep(&t, NULL);
}

/*
void wifi_get_stats() {
	char str[128]; char *substr;
	int i = 0; FILE *fp;
	do {
		sprintf(str, "tail -%u /var/log/messages > tail.log", ++i);
		system(str);
		fp = fopen("tail.log", "r");
		fgets(str, sizeof(str), fp);
		fclose(fp);
		substr = strstr(str, "WiFi packet#");
	} while (!substr);
	wifi_stats.packet_num = atoi(substr + 12);
	wifi_stats.retries_total = atoi(strrchr(substr, ' '));
	wifi_stats.timestamp_ms = (unsigned long) (atof(strstr(str, "kernel:") + 10) * 1000.0f);
}
*/

void radio_test() {
#if M == 2
	unsigned long x[M] = {5000, 200};
#else
	unsigned long x[M] = {5000, 5000, 200, 200, 10};
#endif
	m_radio_send(x);
}

void extern_radio_init(int erid) {
	char *devpath = extern_radio_devname[erid];
	if (-1 == (extern_radio_uart[erid] = open(devpath, O_RDWR | O_NOCTTY | O_NDELAY))) {
		fprintf(stderr, "Unable to open %s\n", devpath); exit(1);
	} fcntl(extern_radio_uart[erid], F_SETFL, 0); //O_NONBLOCK
}

void wifi_radio_init(int irid) {
	int i; char pcap_error[PCAP_ERRBUF_SIZE], cmd[32];
	sprintf (cmd, "sudo ifconfig %s up", wifi_devname[irid]);
	system (cmd); delay (250);
	memcpy (wifi_packet[irid], wifi_targetmac[irid], 6);
	memcpy (wifi_packet[irid] + 6, wifi_localmac[irid], 6);
	for (i=12; i < sizeof(wifi_packet) / N_IR; i++) wifi_packet[irid][i] = rand();
	wifi_pcap[irid] = pcap_open_live(wifi_devname[irid], 2048, 0, 250, pcap_error);
	sprintf (cmd, "sudo ifconfig %s down", wifi_devname[irid]); system (cmd);
	if (!wifi_pcap[irid]) { fprintf(stderr, "Unable to open WiFi adapter: %s\n", pcap_error); exit(1); }
}

void lora_radio_init() {
	int i;
	for (i=0; i < sizeof(lora_packet) - 1; i++)
		lora_packet[i] = (rand() % 10) + '0';
	lora_packet[sizeof(lora_packet)-1] = 0;
	LoRaWAN.ON(SOCKET0);
	LoRaWAN.macPause();
	LoRaWAN.setRadioPower(2);
	LoRaWAN.setRadioFreq(911000000);
	LoRaWAN.setRadioSF((char*)"sf7");
	LoRaWAN.setRadioCR((char*)"4/5");
	LoRaWAN.setRadioBW(250);
	LoRaWAN.setRadioCRC((char*)"on");
}

void radio_init() {
	int i;
	for (i=0; i < N_ER; i++) extern_radio_init(i);
	for (i=0; i < N_IR; i++) wifi_radio_init(i);
	if (M > N_IR + N_ER) lora_radio_init();
	radio_test();
}

void radio_end() {
	int i;
	for (i=0; i<N_IR; i++) pcap_close(wifi_pcap[i]);
	for (i=0; i<N_ER; i++) unistd::close(extern_radio_uart[i]);
}

void radio_write_result(int rid, int res) {
	char fname[2] = {'0' + rid};
	FILE *fp = fopen (fname, "w");
	fputc (res, fp);
	fclose (fp);
}

void extern_radio_send(radio_cmd_t* rcmd) {
	int erid = rcmd->rid - N_IR;
	uint32_t uart_buffer[2] = {rcmd->x, D * 1e3};
	IOPUT (PORT_ER(erid), unistd::write);
	unistd::fsync (extern_radio_uart[erid]);
	uint8_t scs;
	unistd::read (extern_radio_uart[erid], &scs, 1); //wait until radio finishes
	/*res.scs = * (uint16_t *) (bf); //uart_buffer[erid].radio_feedback.Success;
	res.thpt = * (uint32_t *) (bf + 2); //uart_buffer[erid].radio_feedback.Thruput;
	res.etx = (* (uint16_t *) (bf + 6)) * 1e-3; //uart_buffer[erid].radio_feedback.ETX * 0.001f;
	//USE BYTE ARRAY THAN STRUCT IF PRECISE ADDRESS IS REQUIRED!*/
	radio_write_result (rcmd->rid, scs > 0);
}

void wifi_radio_send(radio_cmd_t* rcmd) {
	struct pcap_pkthdr *header;
	const u_char *packet_recv;
	uint16_t irid = rcmd->rid;
	uint32_t i, deadline = D * 1e6;
	char cmd[32];
	us_stopwatch (0, 1);
	sprintf (cmd, "sudo ifconfig %s up", wifi_devname[irid]);
	system (cmd); delay (250);
	//us_stopwatch (1, 1);
	for (i = 0; i < rcmd->x; i++) {
		IOPUT (PORT_IR(irid), pcap_inject);
		if (us_stopwatch (0, 0) > deadline) break;
	}
	//res.etx = ETX[rcmd->rid];
	//res.thpt = i * 8e6 / us_stopwatch (1, 0);
	sprintf (cmd, "sudo ifconfig %s down", wifi_devname[irid]);
	system (cmd);
	radio_write_result (rcmd->rid, i == rcmd->x);
}

void lora_radio_send(radio_cmd_t* rcmd) {
	uint32_t i, deadline = D * 1e6;
	//radio_result_t res = {1};
	us_stopwatch (0, 1);
	for (i = 0; i < rcmd->x; i++) {
		delay(50);
		LoRaWAN.sendRadio (lora_packet);
		if (us_stopwatch (0, 0) > deadline) break;
	}
	//res.etx = ETX[rcmd->rid];
	//res.thpt = i * 1e6 / us_stopwatch (0, 0);
	radio_write_result (rcmd->rid, i == rcmd->x);
}

uint16_t m_radio_send(unsigned long *x) {
	uint16_t i, scs = 0;
	void (*radio_send_func) (radio_cmd_t *);
	radio_cmd_t rcmd[M];
	char fname[2] = {0};
	FILE *fp;
	for (i=0; i<M; i++) if (x[i] > 0) {
		rcmd[i].rid = i;
		rcmd[i].x = i < N_IR ? (x[i] + 7) / 8 : x[i];
		radio_send_func = i < N_IR ? wifi_radio_send :
				(i < N_IR+N_ER ? extern_radio_send :
				lora_radio_send);
		if (unistd::fork() == 0) { radio_send_func (rcmd + i); exit (0); }
	}
	for (i=0; i<M; i++) {
		if (x[i] == 0) { scs |= 1 << i; continue; }
		fname[0] = '0' + i;
		while (unistd::access (fname, F_OK) == -1) unistd::usleep (1000);
		fp = fopen (fname, "r");
		scs |= (fgetc (fp) != 0) << i;
		fclose (fp); remove (fname);
		//TH[i]  = HoltWinter_Predictor (i*2 + TH_PREDICT, thpt);
		//ETX[i] = HoltWinter_Predictor (i*2 + ETX_PREDICT, etx);
		//if (ETX[i] < 1.0f) ETX[i] = 1.0f;
	}
	return scs;
}
