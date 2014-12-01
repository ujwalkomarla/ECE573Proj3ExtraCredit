int getMyIp(void){
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
				freeifaddrs(addrs);
				return inet_addr(pAddr->sin_addr);
			}
		}

		tmp = tmp->ifa_next;
	}

	freeifaddrs(addrs);
	return -1;
}

void DieWithError(char *msg){
	perror(msg);
	exit(EXIT_FAILURE);
}

int createUDPsock(int tSocket){
//if Input parameter is Zero, we let the OS to choose a port on behalf of us
	int sockID;
	if ((sockID = socket(AF_INET, SOCK_DGRAM, 0)) < 0) DieWithError("Unable to create a socket");
	struct sockaddr_in myAddr;
	memset((struct sockaddr_in *)&myAddr, 0 , sizeof(myAddr));
	myAddr.sin_family = AF_INET;
	myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myAddr.sin_port = htons(tSocket);
	if(bind(sockID,(struct sockaddr *)&myAddr, sizeof(myAddr))<0) DieWithError("Unable to do a bind on socket"); 
	return sockID;
}