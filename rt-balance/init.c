#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argn, char *argv[]) {
	int fd, rem;
	if(argn < 2) return 1;
	rem = atoi(argv[1]);
	fd = open("rem", O_WRONLY | O_CREAT);
	write(fd, &rem, sizeof(rem));
	return 0;
}