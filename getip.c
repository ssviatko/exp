#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>

#define MAX_IFS 16

int main(int argc, char **argv)
{
	int ret, fd;
	struct ifconf ifc;
	struct ifreq ifr[MAX_IFS];
	int num_interfaces, i;
	struct sockaddr_in *ipv4_addr;
	char addr_str[INET_ADDRSTRLEN];

	ifc.ifc_buf = (char *)ifr;
	ifc.ifc_len = sizeof(struct ifreq);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	ioctl(fd, SIOCGIFCONF, &ifc);
	close(fd);

	num_interfaces = ifc.ifc_len / sizeof(struct ifreq);
	printf("%d interfaces found.\n", num_interfaces);
    
	for (i = 0; i < num_interfaces; i++) {
		ipv4_addr = (struct sockaddr_in *)&ifr[i].ifr_addr;
		inet_ntop(AF_INET, &ipv4_addr->sin_addr, addr_str, INET_ADDRSTRLEN);
		printf("%s\t%s\n", ifr[i].ifr_name, addr_str);
	}

	return 0;
}

