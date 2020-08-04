#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#ifdef WIFI
#include <pcap.h>
#endif

#define MIN(X,Y) X<Y ? X:Y

int main() {
	int ii, fd, rem, ass, sent = 0;
	char cmdtime[64];
	sprintf(cmdtime, "cat /proc/uptime >> ts_%s", RID);
	system(cmdtime);
	#ifdef WIFI
	pcap_t *pcap; char pcap_error[PCAP_ERRBUF_SIZE];
	uint8_t packet[256] = {0xff,0xff,0xff,0xff,0xff,0xff};
	for(ii=12; ii<sizeof(packet); ii++) packet[ii] = rand();
	pcap = pcap_open_live(WIFI, 1024, 0, 250, pcap_error);
	if(pcap == NULL) return 1;
	#endif
	#ifdef ZIGBEE
	int uart_fd;
	uint8_t buf;
	uart_fd = open(ZIGBEE, O_WRONLY);
	if(uart_fd == -1) return 1;
	#endif
	fd = open("rem", O_RDWR);
	while(1) {
		read(fd, &rem, sizeof(rem)); lseek(fd, 0, SEEK_SET);
		if (rem <= 0) break;
		ass = MIN(rem, PGS);
		rem -= ass;
		write(fd, &rem, sizeof(rem)); lseek(fd, 0, SEEK_SET);
		#ifdef WIFI
		for(ii=0; ii<(ass+3)/4; ii++) pcap_inject(pcap, packet, sizeof(packet));
		#endif
		#ifdef ZIGBEE
		buf = (uint8_t)ass;
		write(uart_fd, &buf, 1); fsync(uart_fd);
		usleep((useconds_t)ass * ZIGPKTUS);
		#endif
		sent += ass;
	}
	system(cmdtime);
	close(fd);
	printf("%s:\t%d\n", RID, sent);
	return 0;
}
