#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>
void main(void){
	struct ifaddrs *addrs, *tmp;
	getifaddrs(&addrs);
	tmp = addrs;

	while (tmp) 
	{
		if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET)
		{
			if(strcmp(tmp->ifa_name,"wlan0") == 0){
				struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
				printf("%s: %s\n", tmp->ifa_name, inet_ntoa(pAddr->sin_addr));
			}
		}

		tmp = tmp->ifa_next;
	}

	freeifaddrs(addrs);
	return;
}