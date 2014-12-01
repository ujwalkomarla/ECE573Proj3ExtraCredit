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

//Argument 0: Program name, Argument 1: Self Server Port Number. Argument 2: Self NodeNumbInTopology. Argument 3: Total Number of nodes in topology. 
//One Set(IP PortNo Cost NodeNumbInTopology)

void setAlarm(void){
  
  struct itimerval new;
  new.it_interval.tv_sec = 1;
  new.it_interval.tv_usec = 0;
  new.it_value.tv_sec = 1;
  new.it_value.tv_usec = 0;
   
  if (setitimer (ITIMER_REAL, &new, NULL) < 0)
      printf("timer init failed\n");
  else
      printf("timer init succeeded\n");
  return;
}
void catch_alarm (int sig)
{
  //keep_going = 0;
  //printf("Alarm event\n");
  struct sockaddr_in sendTo;
  pthread_mutex_lock(&sendMutex);
	for(i=0;i<noOfNeighbours;i++){
		sendTo.sin_family = AF_INET;
		sendTo.sin_port = htons(neighbourInfo->ServerPortNos[i]);
		sendTo.sin_addr.s_addr = inet_addr(neighbourInfo->IPaddr[i]);
		sendto(SockID,myCostMatrix,sizeof(float) *  (noOfNodesInTopology),0,(struct sockaddr *)&sendTo,sizeof(sendTo));
	}
	pthread_mutex_unlock(&sendMutex);
  signal (sig, catch_alarm);
}
void* DV_ALGO(void *msg){

	float *buf = malloc(sizeof(float) *  (noOfNodesInTopology));
	struct sockaddr_in tRecvdFrom;
	unsigned int sizeRecvdFrom = sizeof(tRecvdFrom);
	unsigned int SEND=0;
	while(1){
		if((sizeof(myCostMatrix) != recvfrom(sockID,buf,MAX_SIZE,0,(struct sockaddr *)&tRecvdFrom,&sizeRecvdFrom))<0) DieWithError("Server can't receive packets");
		for(i=0;i<noOfNeighbours;i++){
			if(tRecvdFrom.sin_port == neighbourInfo->ServerPortNos[i] && tRecvdFrom.sin_addr.s_addr == neighbourInfo->IPaddr[i]){
				costToi = myCostMatrix[i];
				pthread_mutex_lock(&sendMutex);
				for(j=0;j<noOfNodesInTopology;j++){
					buf[j]+=costToi;
					if(buf[j]<myCostMatrix[j]){
						myCostMatrix[j] = buf[j];
						myNextHopMatrix[j] = i;
						SEND=1;
					}
				}
				pthread_mutex_unlock(&sendMutex);

			}
		}
		if(SEND){
			//Send matrix to neighbours
			struct sockaddr_in sendTo;
			for(i=0;i<noOfNeighbours;i++){
				sendTo.sin_family = AF_INET;
				sendTo.sin_port = htons(neighbourInfo->ServerPortNos[i]);
				sendTo.sin_addr.s_addr = inet_addr(neighbourInfo->IPaddr[i]);
				sendto(SockID,myCostMatrix,sizeof(float) *  (noOfNodesInTopology),0,(struct sockaddr *)&sendTo,sizeof(sendTo));
			}
			SEND=0;
		}
		signal (SIGALRM, catch_alarm);
		
		setAlarm();
	}
}





int main(int argc, char **argv){
	int i;
	int myServerPort;
	int selfNodeNumbInTopology;
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
	for(i=0;i<noOfNodesInTopology;i++) myCostMatrix[i] = INFINITY;
	
	myCostMatrix[selfNodeNumbInTopology] = 0.0;
	myNextHopMatrix[selfNodeNumbInTopology] = selfNodeNumbInTopology;
	myIP = getMyIP();
	if(myIP=-1) DieWithError("Getting self IP Address Issue\r\n");
	//UDP socket create
	SockID = createUDPsock(myServerPort);
	for(i=0;i<noOfNeighbours;i++){
		neighbourInfo->IPaddr[i] = inet_addr(argv[NEIGHBOUR_SET_START + 1 + (i*NEIGHBOUR_SET_INFO)]);//IP is at 3,7,11
		neighbourInfo->ServerPortNos[i] = atoi(argv[NEIGHBOUR_SET_START + 2 + (i*NEIGHBOUR_SET_INFO)]);//Server port number of IP is at 4,8,12
		neighbourInfo->NodeNumbInTopology[i] = atoi(argv[NEIGHBOUR_SET_START + 4 + (i*NEIGHBOUR_SET_INFO)]);//NodeNumbInTopology is at 6,10,14
		myCostMatrix[neighbourInfo->NodeNumbInTopology[i]-1] = atof(argv[NEIGHBOUR_SET_START + 3 + (i*NEIGHBOUR_SET_INFO)]);//Cost to neighbour links is at 5,9,13
		myNextHopMatrix[neighbourInfo->NodeNumbInTopology[i]-1] = neighbourInfo->NodeNumbInTopology[i] ;
	}
	//Send matrix to neighbours
	struct sockaddr_in sendTo;
	for(i=0;i<noOfNeighbours;i++){
		sendTo.sin_family = AF_INET;
		sendTo.sin_port = htons(neighbourInfo->ServerPortNos[i]);
		sendTo.sin_addr.s_addr = inet_addr(neighbourInfo->IPaddr[i]);
		sendto(SockID,myCostMatrix,sizeof(float) *  (noOfNodesInTopology),0,(struct sockaddr *)&sendTo,sizeof(sendTo));
	}
	if(0 != pthread_create( &thread_DV_ALGO, NULL, DV_ALGO, clientSenderThreadArgument)) DieWithError("Error - pthread_create()");	
//    if(0 !=pthread_create( &thread_timer, NULL, timerFunc, clientSenderThreadArgument)) DieWithError("Error - pthread_create()");

	pthread_join( thread_DV_ALGO, NULL);
	pthread_mutex_destroy(&sendMutex);

//	pthread_join( thread_timer, NULL);
	close(SockID);
	return 0;
}


