/*
	build : gcc -o resource_proxy resource_proxy.c -lpthread	
	execution : ./resource_proxy 2048 127.0.0.1 1024 
*/
 

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define ENABLE_LINKEDLIST				0		//1:using linked list for managing clients
#define ENABLE_HANDLETHREAD			1		//1:handling messages with thread
#define ENABLE_MUTEX						1		//1:using mutex for handling resource

#define MAXLINE		(1024)
#define MAX_SOCK 	(1024)
#define DELAY_DUMMY		(50*1000)

int getFdMax(int);
void removeClient(int);

int g_piFdMax;
int g_iClientMax = 0;
int g_piSocketClient[MAX_SOCK] = {0, };
char *escapechar = "exit";

//pthread_mutex_t m_lock;

struct __message {
	int iFd;
	unsigned int owner;
	unsigned int cnt;
	unsigned int cmd;
	unsigned int req_dur;
	unsigned int rsp_dur;
	struct timeval server_recved;
	struct timeval server_started;
	struct timeval server_finished;
	struct timeval proxy_recved;
	struct timeval proxy_started;
	struct timeval proxy_finished;
	struct timeval client_recved;
	struct timeval client_started;
	struct timeval client_finished;
};

struct __client {
	int iId;
	int iFd;
	struct __client *next;
};


struct __resource_cached {
	int iData;
	int iAga;		/* milisecond */
};

struct __resource {
	char strName[128];
	int iCachedAge;
	int iCachedResource;
	unsigned int iClientNumber;
	struct __client *observer;
};

int g_iSocketServer = 0;

struct __resource g_Resource;


int initResource()
{
	g_Resource.strName[0] = '\0';
	g_Resource.iCachedAge = 0;
	g_Resource.iCachedResource = 0;
	g_Resource.iClientNumber = 0;
	g_Resource.observer = NULL;
}

int getResource(struct __resource *resource, int *iResource, int iCached)
{
	if( (iCached) && (resource->iCachedAge>0) )
	{
		//TODO: get resource from cache
		*iResource = resource->iCachedResource;
	}
	else
	{
			//TODO: get resource from server
	}
	return 0;
}

int dumpObserver()
{
	struct __client *client; 
	client = g_Resource.observer;
	
//printf("%s1:client=0x%x\n", __func__, client);
	
	while(client)
	{
		//printf("%s:client=0x%x, id=%d, fd=%d\n", __func__, client,  client->iId, client->iFd);
		printf("%s:id=%d, fd=%d\n", __func__, client->iId, client->iFd);
		client = client->next;
	}
		
	return 0;
}

int addObserver(int iId, int iFd)
{
	struct __client *pObserver;
	struct __client *pNew;
	struct __client *pPrev;
	int fAdd;
	
	pNew = (struct __client *)malloc(sizeof(struct __client));
	pNew->iId = iId;
	pNew->iFd = iFd;
	pNew->next = NULL;
	
//printf("%s:+++:pObserver=0x%x\n", __func__, pObserver);	
	if(g_Resource.observer == NULL)
	{
		//printf("%s1:id=%d, fd=%d, g_Resource.observer=0x%x, pNew=0x%x\n", __func__, pNew->iId, pNew->iFd, g_Resource.observer, pNew);
		g_Resource.observer = pNew;
	}
	else
	{	
		pObserver = g_Resource.observer;
		
		fAdd = 1;
		while(pObserver)
		{
			if(pObserver->iId == iId)
			{
				if(pObserver->iFd != iFd)
					pObserver->iFd = iFd;
				
				fAdd = 0;
				break;
			}
			else 
			{
				pPrev = pObserver;
				//printf("%s2:id=%d, fd=%d, g_Resource.observer=0x%x, pNew=0x%x\n", __func__, pNew->iId, pNew->iFd, g_Resource.observer, pNew);
				pObserver = pObserver->next;
			}
		}
		
		if(fAdd)
			pPrev->next = pNew;
	}
		
//printf("%s:---:pObserver=0x%x, g_Resource.observer=0x%x\n", __func__, pObserver, g_Resource.observer);
	
	return 0;
}

int removeObserver(int iId, int iFd)
{
	struct __client *pObserver; 
	struct __client *pFront; 
	struct __client *pRear; 
	struct __client *pTemp; 
	pObserver = g_Resource.observer;
	
	while(pObserver)
	{
		if(iId == pObserver->iId)
		{
			//pTemp = pObserver;
			//pObserver = 
			;
		}
		//printf("id=%d, fd=%d", observer.iId, observer.iFd);
		pObserver = pObserver->next;
	}
	
	return 0;
}

int handleMessage(struct __message *arg)
{
	struct timeval timeStart, timeEnd;
	struct __message msg = {0x0, };
	struct __message resp = {0x0, };

	memcpy(&msg, arg, sizeof(struct __message));

	//TODO: handle packet
	gettimeofday(&timeStart, NULL);

	//TODO: handle packet
	//usleep(DELAY_DUMMY);
#if 1
	//TODO: use cached resource
	addObserver(msg.owner, msg.iFd);
	dumpObserver();
#endif

#if 1
	//TODO: simply get resource form server
	if(send(g_iSocketServer, &msg, sizeof(struct __message), 0) < 0)
	{
		printf("proxy : send failed!\n");
	}	//if(send(

		int size;
		if((size = recv(g_iSocketServer, &resp, sizeof(struct __message), 0)) > 0)
		{
			msg.server_recved.tv_sec = resp.server_recved.tv_sec;
			msg.server_recved.tv_sec = resp.server_recved.tv_sec;
			msg.server_started.tv_sec = resp.server_started.tv_sec;
			msg.server_started.tv_usec = resp.server_started.tv_usec;
			msg.server_finished.tv_sec = resp.server_finished.tv_sec;
			msg.server_finished.tv_usec = resp.server_finished.tv_usec;
		} //if((size
#endif		
    			
	gettimeofday(&timeEnd, NULL);

	printf("[%d]%d-%d\n", msg.owner, msg.cnt, msg.req_dur);
	//msg.proxy_recved.tv_sec = timeRecv.tv_sec;
	//msg.proxy_recved.tv_usec = timeRecv.tv_usec;
	msg.proxy_started.tv_sec = timeStart.tv_sec;
	msg.proxy_started.tv_usec = timeStart.tv_usec;
	msg.proxy_finished.tv_sec = timeEnd.tv_sec;
	msg.proxy_finished.tv_usec = timeEnd.tv_usec;

	msg.rsp_dur = 1000;	
	
	int temp = send(msg.iFd, &msg, sizeof(struct __message), 0);

	return 0;
}

void *pthread_func(void *arg)
{
	handleMessage((struct __message *)arg);
	
	return NULL;
}

int initServerConnection(struct sockaddr_in server_addr)
{
	if((g_iSocketServer=socket(PF_INET, SOCK_STREAM, 0)) < 0 )
	{
		printf("proxy : socket failed!\n");
		return 0;
	}	//if( (s=socket
	
	if(connect(g_iSocketServer, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) < 0)
	{
		printf("proxy : connect failed!\n");
		return 0;
	}
	else
	{
		printf("proxy : connected server!\n");
	}	//if(connect(
	
	return 1;
}

int main(int argc , char *argv[])
{
	char rline[MAXLINE], my_msg[MAXLINE];
	//char *start = "Connected to server\n";
	int i, j, n;
	int s, client_fd, client;
	fd_set read_fds;
	struct sockaddr_in server_addr;
	struct sockaddr_in proxy_addr;
	struct sockaddr_in client_addr;
		
	struct __message rcv = { 0x0, };
	struct __message trans = { 0x0, };
		
	//pthread_mutex_init(&m_lock, NULL);
		
	if(argc != 4)
	{
		printf("usage : %s [proxy_port#] [server_ip#] [server_port#] \n", argv[0]);
		exit(0);
	}	//if(argc
	
	//TODO: init timer handler
	//initTimerHandler();
	
	//TODO: init cached resource struct
	initResource();
		
	//TODO: init server connection
	bzero((char *)&proxy_addr, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[2]);
	server_addr.sin_port = htons(atoi(argv[3]));
	initServerConnection(server_addr);
			
	if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("server : socket failed!\n");
		exit(0);
	}	//if((s = socket
		

	//TODO: init proxy connection
	int option = 1;
	setsockopt( s, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int) );
	bzero((char *)&proxy_addr, sizeof(struct sockaddr_in));
	proxy_addr.sin_family = AF_INET;
	proxy_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	proxy_addr.sin_port = htons(atoi(argv[1]));
		
	if(bind(s, (struct sockaddr  *)&proxy_addr, sizeof(struct sockaddr_in)) < 0)
	{
		printf("server : bind failed!\n");
		exit(0);
	}
	
	listen(s, 5);
			
	g_piFdMax = s + 1;


//Minjin, 
/*
addObserver(1001, 9000);
addObserver(1002, 9001);
addObserver(1005, 9003);
addObserver(1003, 9004);
dumpObserver();
*/

	while(1)
	{
			FD_ZERO(&read_fds);
			FD_SET(s, &read_fds);
			
			for(i=0; i<g_iClientMax; i++)
			{
				FD_SET(g_piSocketClient[i], &read_fds);
			}	//for(i=0;
			
			g_piFdMax = getFdMax(s) + 1;
			if(select(g_piFdMax, &read_fds, (fd_set *)0, (fd_set *)0, (struct timeval *)0) < 0)
			{
				printf("server : select failed\n");
				exit(0);
			}//if(select
				
			if(FD_ISSET(s, &read_fds))
			{
				client = sizeof(struct sockaddr_in);
				client_fd = accept(s, (struct sockaddr *)&client_addr, &client);
				if(client_fd == -1)
				{
					printf("server : accept failed!\n");
					exit(0);
				}

				g_piSocketClient[g_iClientMax] = client_fd;
				g_iClientMax++;

				//printf("client_fd=%d, g_iClientMax=%d\n", client_fd, g_iClientMax);		
				//send(client_fd, start, strlen(start), 0);
				printf("server : user#%d added!\n", g_iClientMax);
			}//if(FD_ISSET

			for(i=0; i<g_iClientMax; i++)
			{
				if(FD_ISSET(g_piSocketClient[i], &read_fds))
				{
					memset(&rcv, 0x0, sizeof(struct __message));
					if((n = recv(g_piSocketClient[i], &rcv, sizeof(struct __message), 0)) <= 0)
					{
						removeClient(i);
						continue;
					}

					if(rcv.cmd == 0x99999999)
					{
						removeClient(i);
						continue;
					}

#if ENABLE_HANDLETHREAD 
					struct timeval timeRecv;
					gettimeofday(&timeRecv, NULL);
					rcv.proxy_recved.tv_sec = timeRecv.tv_sec;
					rcv.proxy_recved.tv_usec = timeRecv.tv_usec;
					
					pthread_t ptId = 0;
					rcv.iFd = g_piSocketClient[i];
					pthread_create(&ptId, NULL, pthread_func, (void *)&rcv);
					//usleep(1);
#else
					handleMessage(&rcv);
					memcpy(&trans, &rcv, sizeof(struct __message));

					int temp = send(g_piSocketClient[i], &trans, sizeof(struct __message), 0);
#endif
				} //if(FD_ISSET(	
			} //for(i=0
	} //while(1)
	
	return 0;
}


void removeClient(int index)
{
	close(g_piSocketClient[index]);
	if(index != g_iClientMax-1)
	{
		g_piSocketClient[index] = g_piSocketClient[g_iClientMax-1];
	}
	g_iClientMax--;
	printf("server : user exit, num = %d\n", g_iClientMax);
}	//void removeClient


int getFdMax(int k)
{
	int max = k;
	int r;
	
	for(r=0; r<g_iClientMax; r++)
	{
		if(g_piSocketClient[r] > max)
		{
			max = g_piSocketClient[r];
		}
	}
	return max;
}	//int getFdMax

