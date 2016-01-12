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
#include "resource.h"

int getFdMax(int);
void removeClient(int);

int g_piFdMax;
int g_iClientMax = 0;
int g_piSocketClient[MAX_SOCK] = {0, };

pthread_mutex_t m_lock;
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
		g_Resource.iClientNumber++;
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
		{
			pPrev->next = pNew;
			g_Resource.iClientNumber++;
		}
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

int addTimeValue(struct timeval *timeR, struct timeval timeA, struct timeval timeB)
{
	if(timeA.tv_usec+timeB.tv_usec > 1000000)
	{
		timeR->tv_usec = timeA.tv_usec + timeB.tv_usec - 1000000;
		timeR->tv_sec = timeA.tv_sec + timeB.tv_sec + 1;
	}
	else
	{
		timeR->tv_usec = timeA.tv_usec + timeB.tv_usec;
		timeR->tv_sec = timeA.tv_sec + timeB.tv_sec;
	}
	
	return 0;
}

int isBiggerThan(struct timeval timeA, struct timeval timeB)
{
	int ret = 0;
	
	if(timeA.tv_sec > timeB.tv_sec )
	{
		ret = 1;
	}
	else if(timeA.tv_sec == timeB.tv_sec)
	{
		if(timeA.tv_usec > timeB.tv_usec)
			ret = 1;
		else
			ret = 0;
	}
	return ret;
}


int isCachedDataValid(struct timeval curTime)
{
	struct timeval timeB;
	struct timeval timeRet;
	int ret = 0;
//printf("g_Resource.iCachedAge=%d, g_Resource.tCachedTime.tv_sec=%ld\n", g_Resource.iCachedAge, g_Resource.tCachedTime.tv_sec);

	if( (g_Resource.iCachedAge>0) && (g_Resource.tCachedTime.tv_sec!=0) )
	{
		timeB.tv_sec = 0;
		timeB.tv_usec = g_Resource.iCachedAge*1000;
		
		addTimeValue(&timeRet, g_Resource.tCachedTime, timeB);

//printf("g_Resource.tCachedTime=%ld.%ld\n", g_Resource.tCachedTime.tv_sec, g_Resource.tCachedTime.tv_usec);
//printf("timeRet=%ld.%ld\n", timeRet.tv_sec, timeRet.tv_usec);
//printf("curTime=%ld.%ld\n", curTime.tv_sec, curTime.tv_usec);		
		if(isBiggerThan(timeRet, curTime))
		{
			ret = 1;
		}
		else
		{
			g_Resource.tCachedTime.tv_sec = 0;
			g_Resource.tCachedTime.tv_usec = 0;
			g_Resource.iCachedAge = 0;
			ret = 0;
		}
	}
	else
	{
		ret = 0;
	}
	return ret;
}

int updateCache(struct timeval curTime)
{
	struct timeval temp;
	
	if(g_Resource.tCachedTime.tv_usec+(g_Resource.iCachedAge*1000) > 1000000)
	{
		temp.tv_sec = g_Resource.tCachedTime.tv_sec + 1;
		temp.tv_usec = g_Resource.tCachedTime.tv_usec+(g_Resource.iCachedAge*1000) - 1000000;
	}
	else
	{
		temp.tv_sec = g_Resource.tCachedTime.tv_sec;
		temp.tv_usec = g_Resource.tCachedTime.tv_usec+(g_Resource.iCachedAge*1000);
	}
//printf("temp=%ld.%ld\n", temp.tv_sec, temp.tv_usec);
//printf("cur=%ld.%ld\n", curTime.tv_sec, curTime.tv_usec);
	if(temp.tv_usec < curTime.tv_usec)
	{
		g_Resource.iCachedAge = (temp.tv_sec-curTime.tv_sec-1)*1000 + (temp.tv_usec+1000000-curTime.tv_usec)/1000;
	}
	else
	{
		g_Resource.iCachedAge = (temp.tv_sec - curTime.tv_sec)*1000 + (temp.tv_usec - curTime.tv_usec)/1000;
	}
	g_Resource.tCachedTime.tv_sec = curTime.tv_sec;
	g_Resource.tCachedTime.tv_usec = curTime.tv_usec;
//printf("g_Resource.iCachedAge=%d\n", g_Resource.iCachedAge);
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

	//TODO: use cached resource
	if(msg.cmd == RESOURCE_CMD_REGISTER)
	{
		addObserver(msg.owner, msg.iFd);
		dumpObserver();
	}
	else if (msg.cmd == RESOURCE_CMD_GET)
	{
		//TODO: get cached resource
		//if(timeStart < g_Resource.tCachedTime + maxAge)
		//if(g_Resource.iCachedResource  && (g_Resource.iCachedAge>0) )
		if(isCachedDataValid(timeStart))
		{
			/*
			printf("%s2:iCachedResource=%d, iCachedAge=%d, tCachedTime=%ld\n", 
				__func__, 
				g_Resource.iCachedResource, 
				g_Resource.iCachedAge, 
				g_Resource.tCachedTime.tv_sec%1000);
			*/
			//	maxAge = g_Resource.tCachedTime + maxAge - timeStart
			//	g_Resource.tCachedTime = timeStart
			//	return cached_value
			
			updateCache(timeStart);
			
			//TODO: set message with time information		
			gettimeofday(&timeEnd, NULL);
			//printf("[%d]%d-%d\n", msg.owner, msg.cnt, msg.req_dur);
			//msg.proxy_recved.tv_sec = timeRecv.tv_sec;
			//msg.proxy_recved.tv_usec = timeRecv.tv_usec;
			msg.proxy_started.tv_sec = timeStart.tv_sec;
			msg.proxy_started.tv_usec = timeStart.tv_usec;
			msg.proxy_finished.tv_sec = timeEnd.tv_sec;
			msg.proxy_finished.tv_usec = timeEnd.tv_usec;
			
			//msg.rsp_dur = resp.rsp_dur;
			//msg.resource = resp.resource;
			msg.rsp_dur = g_Resource.iCachedAge;
			msg.resource = g_Resource.iCachedResource;
			
			//TODO: set cached with information
			//g_Resource.iCachedResource = resp.resource;
			//g_Resource.iCachedAge = resp.rsp_dur;
			memcpy(&g_Resource.tCachedTime, &timeEnd, sizeof(struct timeval));
			/*
			printf("%s3:iCachedResource=%d, iCachedAge=%d, tCachedTime=%ld\n", 
				__func__, 
				g_Resource.iCachedResource, 
				g_Resource.iCachedAge, 
				g_Resource.tCachedTime.tv_sec%1000);
			*/
//usleep(500*1000);
			//TODO: send information as response from proxy to client
			int temp = send(msg.iFd, &msg, sizeof(struct __message), 0);
			
		}
		else
		{
			//TODO: send information request from a proxy to a resource server
			if(send(g_iSocketServer, &msg, sizeof(struct __message), 0) < 0)
			{
				printf("proxy : send failed!\n");
			}	//if(send(
		
			//TODO: get information from a resource server to proxy
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

			//TODO: set message with time information			
			gettimeofday(&timeEnd, NULL);
			//printf("[%d]%d-%d\n", msg.owner, msg.cnt, msg.req_dur);
			//msg.proxy_recved.tv_sec = timeRecv.tv_sec;
			//msg.proxy_recved.tv_usec = timeRecv.tv_usec;
			msg.proxy_started.tv_sec = timeStart.tv_sec;
			msg.proxy_started.tv_usec = timeStart.tv_usec;
			msg.proxy_finished.tv_sec = timeEnd.tv_sec;
			msg.proxy_finished.tv_usec = timeEnd.tv_usec;
			
			msg.rsp_dur = resp.rsp_dur;
			msg.resource = resp.resource;
			
			//TODO: set cached with information
			g_Resource.iCachedResource = resp.resource;
			g_Resource.iCachedAge = resp.rsp_dur;
			memcpy(&g_Resource.tCachedTime, &timeEnd, sizeof(struct timeval));
			printf("%s1:iCachedResource=%d, iCachedAge=%d, tCachedTime=%ld\n", 
				__func__, 
				g_Resource.iCachedResource, 
				g_Resource.iCachedAge, 
				g_Resource.tCachedTime.tv_sec%1000);
			
			//TODO: send information as response from proxy to client
			int temp = send(msg.iFd, &msg, sizeof(struct __message), 0);
		}
	}
		
	return 0;
}

void *pthreadHandleMessage(void *arg)
{
	handleMessage((struct __message *)arg);
	
	return NULL;
}

int g_iExit = 0;

void dumpCurrentTime()
{
	struct timeval timeCurrent;	
	
	gettimeofday(&timeCurrent, NULL);
				
	//printf("%s:current time = %ld.%ld us\n", __func__, timeCurrent.tv_sec, timeCurrent.tv_usec);
}

void *pthreadWatchResource(void *arg)
{
	while(!g_iExit)
	{
		//printf("%s:1-g_Resource.iClientNumber=%d\n", __func__, g_Resource.iClientNumber);
		
		if(g_Resource.iClientNumber)
		{
			dumpCurrentTime();
		}
		usleep(1000*1000);
	}
		
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
	pthread_t threadId = 0;
	pthread_create(&threadId, NULL, pthreadWatchResource, (void *)NULL);

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
					pthread_create(&ptId, NULL, pthreadHandleMessage, (void *)&rcv);
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

