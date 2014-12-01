#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>
#include<pthread.h>
#include<signal.h>

#define itimerspec linux_itimerspec
#define timespec linux_timespec
//#define timeval linux_timeval
#include<sys/time.h>
//#undef timeval
#undef timespec
#undef itimerspec
#define INFINITY 99.9

int getMyIp(void);
void DieWithError(char *);
int createUDPsock(int);
