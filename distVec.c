#include"Custom.h"
#define NEIGHBOUR_SET_START 3
#define NEIGHBOUR_SET_INFO 4
#define MAX_SIZE 1000
struct neighbourDB{
		int *IPaddr;
		int *ServerPortNos;
		int *NodeNumbInTopology;
};
float *myCostMatrix;
int noOfNodesInTopology;
int noOfNeighbours;
struct neighbourDB *neighbourInfo;
unsigned int SockID;
pthread_mutex_t sendMutex;
float *myNextHopMatrix; 
int DataSize;
FILE *fp, *fd;
char *buf;
//char *sendMyCostMatrix;//6-> 000.0 


//Argument 0: Program name, Argument 1: Self Server Port Number. Argument 2: Self NodeNumbInTopology. Argument 3: Total Number of nodes in topology. 
//One Set(IP PortNo Cost NodeNumbInTopology)


void *timelyUpdates(void *msg)
{
  //keep_going = 0;
  //printf("Alarm event\n");
  struct sockaddr_in sendTo;
  while(1){
  pthread_mutex_lock(&sendMutex);
  int i;
	rewind(fp);
	for(i=0;i<noOfNodesInTopology;i++) fprintf(fp ,"%1.1f ", myCostMatrix[i]);
	rewind(fp);

	fread(buf,sizeof(char),DataSize,fp);
	for(i=0;i<noOfNeighbours;i++){
		sendTo.sin_family = AF_INET;
		sendTo.sin_port = (neighbourInfo->ServerPortNos[i]);
		sendTo.sin_addr.s_addr = neighbourInfo->IPaddr[i];
		
		sendto(SockID,buf,DataSize,0,(struct sockaddr *)&sendTo,sizeof(sendTo));
	}
	pthread_mutex_unlock(&sendMutex);
	sleep(5);
  }
}
void* DV_ALGO(void *msg){

	
	struct sockaddr_in tRecvdFrom;
	unsigned int sizeRecvdFrom = sizeof(tRecvdFrom);
	unsigned int SEND=0;
	int i,j;
	float costToi;
	//char *tempReadVal;
	float tVal;
	int recvdSize;
	while(1){
		if((recvdSize = recvfrom(SockID,buf,MAX_SIZE,0,(struct sockaddr *)&tRecvdFrom,&sizeRecvdFrom))<0){ 
			//printf()
			DieWithError("Server can't receive packets");
			
			}
			buf[DataSize]='\0';
			
			rewind(fd);
			fwrite(buf, 1, DataSize, fd);
			rewind(fd);
			//printf("RCVD %s\r\n",buf);
			//printf("%d Port\r\n", tRecvdFrom.sin_port);
			fflush(stdout);
		for(i=0;i<noOfNeighbours;i++){
			if(tRecvdFrom.sin_port == neighbourInfo->ServerPortNos[i] && tRecvdFrom.sin_addr.s_addr == neighbourInfo->IPaddr[i]){
				//printf("here\r\n");fflush(stdout);
				costToi = myCostMatrix[neighbourInfo->NodeNumbInTopology[i]-1];
					//printf("CostToi %d\r\n",costToi);fflush(stdout);
				pthread_mutex_lock(&sendMutex);
				//printf("Received from %d at %d:",neighbourInfo->IPaddr[i],neighbourInfo->ServerPortNos[i]);
				for(j=0;j<noOfNodesInTopology;j++){
					fscanf(fd,"%f",&tVal);
					//printf("%f ",tVal);
					tVal+=costToi;
					if(tVal<myCostMatrix[j]){
						myCostMatrix[j] = tVal;
						myNextHopMatrix[j] = i;
						SEND=1;
					}
				}
				printf("\r\n");
				pthread_mutex_unlock(&sendMutex);

			}
		}
		if(SEND){
			//Send matrix to neighbours
			struct sockaddr_in sendTo;
			pthread_mutex_lock(&sendMutex);
			rewind(fp);
			for(i=0;i<noOfNodesInTopology;i++) fprintf(fp ,"%1.1f ", myCostMatrix[i]);
			rewind(fp);
	
			fread(buf,sizeof(char),DataSize,fp);
			for(i=0;i<noOfNeighbours;i++){
				sendTo.sin_family = AF_INET;
				sendTo.sin_port = (neighbourInfo->ServerPortNos[i]);
				sendTo.sin_addr.s_addr = neighbourInfo->IPaddr[i];
				
				sendto(SockID,buf,DataSize,0,(struct sockaddr *)&sendTo,sizeof(sendTo));
			}
			
			pthread_mutex_unlock(&sendMutex);
			SEND=0;
		}
		printf("Updated\r\n");
		for(i=0;i<noOfNodesInTopology;i++) printf("%.1f ",myCostMatrix[i]);
		printf("\r\n");
	}
}





int main(int argc, char **argv){
	int i;
	int myServerPort;
	int selfNodeNumbInTopology;
	//float costToi;
	pthread_mutex_init(&sendMutex,NULL);
	myServerPort = atoi(argv[1]);
	selfNodeNumbInTopology = atoi(argv[2]);
	noOfNodesInTopology = atoi(argv[3]);
	noOfNeighbours = (argc - (NEIGHBOUR_SET_START+1))/NEIGHBOUR_SET_INFO; //Argument 0: Program name, Argument 1: Self Server Port Number. Argument 2: Total Number of nodes in topology. One Set(IP PortNo Cost)
	pthread_t thread_DV_ALGO, thread_timer;
	//int myIP;
	neighbourInfo = malloc(sizeof(struct neighbourDB));
	neighbourInfo->IPaddr = malloc(sizeof(int) * noOfNeighbours);
	neighbourInfo->ServerPortNos = malloc(sizeof(int) * noOfNeighbours);
	neighbourInfo->NodeNumbInTopology = malloc(sizeof(int) * noOfNeighbours);
	
	myCostMatrix = malloc(sizeof(float) *  (noOfNodesInTopology)); 
	myNextHopMatrix = malloc(sizeof(float) *  (noOfNodesInTopology)); 
	for(i=0;i<noOfNodesInTopology;i++) myCostMatrix[i] = INFINITY;
	
	myCostMatrix[selfNodeNumbInTopology-1] = 0.0;
	myNextHopMatrix[selfNodeNumbInTopology-1] = selfNodeNumbInTopology;
	//myIP = getMyIP();
	//if(myIP=-1) DieWithError("Getting self IP Address Issue\r\n");
	//UDP socket create
	SockID = createUDPsock(myServerPort);
	
	for(i=0;i<noOfNeighbours;i++){
		neighbourInfo->IPaddr[i] = inet_addr(argv[NEIGHBOUR_SET_START + 1 + (i*NEIGHBOUR_SET_INFO)]);//IP is at 4,8,12
		neighbourInfo->ServerPortNos[i] = atoi(argv[NEIGHBOUR_SET_START + 2 + (i*NEIGHBOUR_SET_INFO)]);//Server port number of IP is at 5,9,13
		neighbourInfo->NodeNumbInTopology[i] = atoi(argv[NEIGHBOUR_SET_START + 4 + (i*NEIGHBOUR_SET_INFO)]);//NodeNumbInTopology is at 7,11,15
		myCostMatrix[neighbourInfo->NodeNumbInTopology[i]-1] = atof(argv[NEIGHBOUR_SET_START + 3 + (i*NEIGHBOUR_SET_INFO)]);//Cost to neighbour links is at 6,10,14
		myNextHopMatrix[neighbourInfo->NodeNumbInTopology[i]-1] = neighbourInfo->NodeNumbInTopology[i] ;
		
	}
	printf("Initial\r\n");
	for(i=0;i<noOfNodesInTopology;i++) printf("%.1f ",myCostMatrix[i]);
		printf("\r\n");
	//Send matrix to neighbours
	struct sockaddr_in sendTo;
	fp = fopen("temp","w+");
	fd = fopen("temp1","w+");

	//sendMyCostMatrix = malloc(6*  (noOfNodesInTopology));//6-> 0.0 
	DataSize = 4*noOfNodesInTopology;
	buf = malloc(DataSize);
	buf[DataSize]='\0';
	for(i=0;i<noOfNodesInTopology;i++) fprintf(fp ,"%1.1f ", myCostMatrix[i]);
	fflush(fp);
	rewind(fp);

	//printf("%d T",fread(buf,1,DataSize,fp));
	fread(buf,sizeof(char),DataSize,fp);
	//printf("Sending %s",buf);
	fflush(stdout);
	sendTo.sin_family = AF_INET;
	for(i=0;i<noOfNeighbours;i++){
		
		sendTo.sin_port = (neighbourInfo->ServerPortNos[i]);
		sendTo.sin_addr.s_addr = neighbourInfo->IPaddr[i];
		
		sendto(SockID,buf,DataSize,0,(struct sockaddr *)&sendTo,sizeof(sendTo));
	}
	
	
	
	
	if(0 != pthread_create( &thread_DV_ALGO, NULL, DV_ALGO, NULL)) DieWithError("Error - pthread_create()");	
	if(0 != pthread_create(&thread_timer, NULL, timelyUpdates, NULL)) DieWithError("Error - pthread create()");
//    if(0 !=pthread_create( &thread_timer, NULL, timerFunc, clientSenderThreadArgument)) DieWithError("Error - pthread_create()");

	pthread_join( thread_DV_ALGO, NULL);
	pthread_mutex_destroy(&sendMutex);

//	pthread_join( thread_timer, NULL);
	close(SockID);
	return 0;
}