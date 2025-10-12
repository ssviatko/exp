#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/time.h>

int sfd; // serial fd
char inb[4096];

void usage()
{
	fprintf(stderr, "usage: tinybbs <serial device>\n");
	exit(EXIT_FAILURE);
}

void sfdprintf(char *fmt, ...)
{
	char buff[4096];
	va_list args;
	va_start(args, fmt);
	vsprintf(buff, fmt, args);
	va_end(args);
	write(sfd, buff, strlen(buff));
	write(1, buff, strlen(buff));
}

void sfdscan()
{
	int res;
	res = read(sfd, inb, 4096);
	if (res < 0) {
		fprintf(stderr, "read error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	inb[res - 1] = 0;
}

void showfile(char *filename)
{
	int fd, res;
	char buff[4096];

	fd = open(filename, O_RDONLY, 0);
	if (fd < 0) {
		fprintf(stderr, "open error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	res = read(fd, buff, 4096);
	write(sfd, buff, res);
	write(1, buff, res);
	close(fd);
}

void showtime()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	sfdprintf("epoch secs:%d.%d\n", tv.tv_sec, tv.tv_usec);
}

void motd()
{
	showfile("motd");
}

int main(int argc, char **argv)
{
	if (argc != 2) usage();

	sfd = open(argv[1], O_RDWR, O_NOCTTY);
	if (sfd < 0) {
		fprintf(stderr, "open error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	sfdprintf("\nHello from TinyBBS!\n");
	showtime();
	motd();

	sfdprintf("What is your name? ");
	sfdscan();
	sfdprintf("well hello there, %s. Your name is %d letters long.\n", inb, strlen(inb));

	int done = 0;
	do {
		sfdprintf("tb_cmd>");
		sfdscan();
		if ((strcmp(inb, "exit") == 0) || (strcmp(inb, "EXIT") == 0)) {
			done = 1;
			continue;
		}
		if ((strcmp(inb, "motd") == 0) || (strcmp(inb, "MOTD") == 0)) {
			motd();
			continue;
		}
		if ((strcmp(inb, "help") == 0) || (strcmp(inb, "HELP") == 0)) {
			showfile("help");
			continue;
		}
		if ((strcmp(inb, "time") == 0) || (strcmp(inb, "TIME") == 0)) {
			showtime();
			continue;
		}
		sfdprintf("you entered: %s\n", inb);
	} while (done == 0);

	sfdprintf("Goodbye from TinyBBS!\n");
	close(sfd);
	return 0;
}

