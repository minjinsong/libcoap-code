/*
	build : gcc -o resource_proxy resource_proxy.c -lpthread	
	execution : ./resource_proxy 2048 127.0.0.1 1024 
	1. ./resource_proxy 2048 127.0.0.1 1024 0[cache off] 0[align off]
	2. ./resource_proxy 2048 127.0.0.1 1024 1[cache on] 0[align off]
	3. ./resource_proxy 2048 127.0.0.1 1024 0[cache on] 0[align off]
	4. ./resource_proxy 2048 127.0.0.1 1024 1[cache on] 0[align off]
	5. ./resource_proxy 2048 127.0.0.1 1024 1[cache on] 1[align on]
	6. ./resource_proxy 2048 127.0.0.1 1024 1[cache on] 2[new alogrithm]
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

unsigned int g_uiCacheMode = 0;
unsigned int g_uiCacheAlgorithm = 0;
int g_iSocketServer = 0;
struct __resource g_Resource;

int initResource()
{
	g_Resource.strName[0] = '\0';
	g_Resource.uiCachedAge = 0;
	g_Resource.iCachedResource = 0;
	g_Resource.iClientNumber = 0;
	g_Resource.next = NULL;
}

int getResource(struct __resource *resource, int *iResource, int iCached)
{
	if( (iCached) && (resource->uiCachedAge>0) )
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
	client = g_Resource.next;
	
	while(client)
	{
		printf("%s:id=%d, fd=%d\n", __func__, client->iId, client->iFd);
		client = client->next;
	}
		
	return 0;
}

int addObserver(int iId, int iFd, unsigned int uiResource, unsigned int uiReqInterval)
{
	struct __client *pObserver;
	struct __client *pNew;
	struct __client *pPrev;
	int fAdd;
	
	pNew = (struct __client *)malloc(sizeof(struct __client));
	pNew->iId = iId;
	pNew->iFd = iFd;
	pNew->uiReqInterval = uiReqInterval;
	pNew->next = NULL;
	
	struct timeval tNow, tB;
	tB.tv_sec = uiReqInterval/1000;
	tB.tv_usec = (uiReqInterval%1000)*1000;
	
	gettimeofday(&tNow, NULL);
	addTimeValue(&(pNew->tSched), tNow, tB);
	//printf("tNow=%3ld.%06ld\n", tNow.tv_sec, tNow.tv_usec);
	//printf("pNew->tSched=%3ld.%06ld\n", pNew->tSched.tv_sec, pNew->tSched.tv_usec);
		
	if(g_Resource.next == NULL)
	{
		g_Resource.next = pNew;
		g_Resource.iClientNumber++;
	}
	else
	{	
		pObserver = g_Resource.next;
		
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
				pObserver = pObserver->next;
			}
		}
		
		if(fAdd)
		{
			pPrev->next = pNew;
			g_Resource.iClientNumber++;
		}
	}
		
	return 0;
}

int removeObserver(int iFd)
{
	struct __client *pClient; 
	struct __client *pFront; 
	struct __client *pRear; 
	struct __client *pTemp;
	
	if(g_Resource.next == NULL)
		return 0;

	pClient = g_Resource.next;

	if(iFd == pClient->iFd)
	{
		g_Resource.next = pClient->next;
		free(pClient);
		g_Resource.iClientNumber--;
		return 0;
	}

	while(pClient)
	{
		if(iFd == pClient->iFd)
		{
			pFront->next = pClient->next;
			free(pClient);
			g_Resource.iClientNumber--;
			break;
		}
		pFront = pClient;
		pClient = pClient->next;
	}

	return 0;
}

int initCache()
{
	g_Resource.iCachedResource = 0;
	g_Resource.uiMaxAge = 0;
	g_Resource.uiCachedAge = 0;
	setTimeValue(&(g_Resource.tCachedTime), 0, 0);
	
	return 0;
}

int setCache(struct __message msg, struct timeval tNow)
{
	g_Resource.iCachedResource = msg.resource;
	g_Resource.uiMaxAge = msg.uiMaxAge;
	//g_Resource.uiCachedAge = 0;
	setTimeValue(&(g_Resource.tCachedTime), tNow.tv_sec, tNow.tv_usec);
	
}

int isCachedDataValid(struct timeval curTime)
{
	struct timeval timeB;
	struct timeval timeRet;
	int ret = 0;
	
	if( (g_Resource.uiCachedAge<g_Resource.uiMaxAge) && (g_Resource.tCachedTime.tv_sec>0) )
	{
		struct timeval tMaxAge;
		struct timeval tTemp;
		setTimeValue(&tMaxAge, g_Resource.uiMaxAge/1000, (g_Resource.uiMaxAge%1000)*1000);
		addTimeValue(&tTemp, g_Resource.tCachedTime, tMaxAge);
		//printf("%s:tMaxAge=%6ld.%06ld\n", __func__, tMaxAge.tv_sec, tMaxAge.tv_usec);
		//printf("%s:tTemp=%6ld.%06ld\n", __func__, tTemp.tv_sec, tTemp.tv_usec);	
		if(isBiggerThan(tTemp, curTime))
			ret = 1;
		else
			ret = 0;
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
	int ret = 0;

	if(isBiggerThan(curTime, g_Resource.tCachedTime))
	{
		subTimeValue(&temp, curTime, g_Resource.tCachedTime);
		g_Resource.uiCachedAge = temp.tv_sec*1000 + temp.tv_usec/1000;
	}
	else
	{
		g_Resource.uiCachedAge = 0;
	}
	
	//printf("%s:g_Resource.uiCachedAge=%d\n", __func__, g_Resource.uiCachedAge);

	return ret;
}

int handleMessage(struct __message *arg)
{
	struct timeval tStart, timeEnd;
	struct __message msg = {0x0, };
	struct __message resp = {0x0, };

	memcpy(&msg, arg, sizeof(struct __message));

	//TODO: handle packet
	gettimeofday(&tStart, NULL);

	//TODO: wait until other request responded
	//pthread_mutex_lock(&m_lock);

	//TODO: use cached resource
	if(msg.cmd == RESOURCE_CMD_REGISTER)
	{
		addObserver(msg.owner, msg.iFd, msg.resource, msg.req_dur);
		dumpObserver();
	}
	else if (msg.cmd == RESOURCE_CMD_GET)
	{
		struct timeval tEnd;
		
		//TODO: get cached resource
		if(g_uiCacheMode && isCachedDataValid(tStart))
		{
			//TODO: update cache timing information
			updateCache(tStart);
			
			//TODO: set server process time with zero
			setTimeValue(&(msg.server_recved), 0, 0);
			setTimeValue(&(msg.server_started), 0, 0);
			setTimeValue(&(msg.server_finished), 0, 0);
			
			//TODO: set message with time information		
			gettimeofday(&timeEnd, NULL);
			
			//TODO: set message with time information		
			msg.resource = g_Resource.iCachedResource;
			msg.uiMaxAge = g_Resource.uiMaxAge - g_Resource.uiCachedAge;

			printf("CACHED! Owner=%d, R=%d, MaxAge=%d, CachedAge=%d\n", 
				msg.owner,
				g_Resource.iCachedResource, 
				g_Resource.uiMaxAge,
				g_Resource.uiCachedAge
				);
		}
		else
		{
			//TODO: init cache
			initCache();
	
			//TODO: get a fresh resource from a server
			getResourceFromServer(&msg);

			//TODO: get current time
			gettimeofday(&tEnd, NULL);
	
			//TODO: set cached with information
			setCache(msg, tEnd);
		
			printf("NOT cached! Owner=%d, R=%d, MaxAge=%d, CachedAge=%d\n",
				msg.owner,
				g_Resource.iCachedResource, 
				g_Resource.uiMaxAge,
				g_Resource.uiCachedAge
				);
		}
		
		//TODO: set client process time
		//setTimeValue(&(msg.client_recved), 0, 0);
		//setTimeValue(&(msg.client_started), 0, 0);
					
		//TODO: set proxy process time
		//setTimeValue(&(msg.proxy_recved), tStart.tv_sec, tStart.tv_usec);
		setTimeValue(&(msg.proxy_started), tStart.tv_sec, tStart.tv_usec);
		setTimeValue(&(msg.proxy_finished), tEnd.tv_sec, tEnd.tv_usec);

		//TODO: send information as response from proxy to client
		int temp = send(msg.iFd, &msg, sizeof(struct __message), 0);

		//TODO: set a new scheduled time of client
		/*
		struct timeval tB;
		tB.tv_sec = client->uiReqInterval/1000;
		tB.tv_usec = (client->uiReqInterval%1000)*1000;
			addTimeValue(&(client->tSched), client->tSched, tB);
		*/
	}
	
	//pthread_mutex_unlock(&m_lock);
		
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
}

int getResourceFromServer(struct __message *msg)
{
	struct __message resp;
	struct timeval timeEnd;
	struct timeval timeStart;
			
	gettimeofday(&timeStart, NULL);
			
	//TODO: send information request from a proxy to a resource server
	//memset(&msg, 0x0, sizeof(struct __message));
	if(send(g_iSocketServer, msg, sizeof(struct __message), 0) < 0)
	{
		printf("proxy : send failed!\n");
	}	//if(send(

	//TODO: get information from a resource server to proxy
	int size;
	memset(&resp, 0x0, sizeof(struct __message));
	if((size = recv(g_iSocketServer, &resp, sizeof(struct __message), 0)) > 0)
	{
		setTimeValue(&(msg->server_recved), resp.server_recved.tv_sec, resp.server_recved.tv_usec);
		setTimeValue(&(msg->server_started), resp.server_started.tv_sec, resp.server_started.tv_usec);
		setTimeValue(&(msg->server_finished), resp.server_finished.tv_sec, resp.server_finished.tv_usec);
	} //if((size
		
	msg->resource = resp.resource;
	msg->uiMaxAge = resp.uiMaxAge;

	//pthread_mutex_unlock(&m_lock);
			
	return 0;
}
/*
int getFirstScheduledTime(struct timeval *tRet)
{
	struct __client *client = g_Resource.next;
	struct timeval tCacheExpired;
	struct timeval tMaxAge;
	
	setTimeValue(&tMaxAge, g_Resource.uiMaxAge/1000, (g_Resource.uiMaxAge%1000)*1000);
	
	addTimeValue(&tCacheExpired, g_Resource.tCachedTime, tMaxAge);
	
	while(client)
	{
				if(isBiggerThan(tTemp, client->tSched))
				{
					cTemp = client;
					tTemp = client->tSched;
				}
	}
}
*/
void *pthreadWatchResource(void *arg)
{
	struct timeval timeSched;
	struct timeval tStart;
			
	while(!g_iExit)
	{
		if(g_Resource.iClientNumber)
		{
			//TODO: get current time
			gettimeofday(&tStart, NULL);

#if 0
			//TODO: check cache resource age and next scheduled message
			if(isBiggerThan(getFirstScheduledTime(), getCacheExpiredTime))
			{
				struct timeval tTemp;
				int iTemp = (DELAY_SERVER_RX+DELAY_SERVER_TX+DELAY_SERVER_PROCESS)*RESOURCE_DELAY_TRUST;
				temp.tv_sec = iTemp/1000;
				temp.tv_usec = (iTemp%1000)*1000
				setPreFetchTime(subTimeValue(getFirstScheduledTime(), iTemp));
			}
			
			getCacheExpiredTime();
			
			//TODO: add sceduled time
			
			//TODO: 
#elif 0
			struct __client *client = g_Resource.next;
			struct __client *cTemp = NULL;
			struct timeval tTemp = NULL;
			tTemp.tv_sec = tv_sec.tv_sec + (60*60);
			int iCnt = 0;
			int iSum = 0;
			while(client)
			{
				if(isBiggerThan(tTemp, client->tSched))
				{
					cTemp = client;
					tTemp = client->tSched;
				}
				
				iCnt++;
				client = client->next;
			}
			
			struct timeval tTemp2;
			struct timeval tRet;
			setTimeValue(&tTemp2, 0, 260*1000);
			subTimeValue(&tRet, tTemp, tTemp2);
			if(isBiggerThan(tStart, client->tSched))
			{
				
			}
			
#endif	//test			
			
			//TODO: search all clients registered
			struct __client *client = g_Resource.next;
			while(client)
			{
				//TODO: find scheduled client
				if(isBiggerThan(tStart, client->tSched)) 
				{
					struct __message msg;
					struct timeval tEnd;
					
					//TODO: if cached resource is valid, then use it
					if(g_uiCacheMode && isCachedDataValid(tStart) )
					{
						//TODO: update cache timing information
						updateCache(tStart);
						
						//TODO: set server process time with zero
						setTimeValue(&(msg.server_recved), 0, 0);
						setTimeValue(&(msg.server_started), 0, 0);
						setTimeValue(&(msg.server_finished), 0, 0);
						
						//TODO: set message with time information		
						msg.resource = g_Resource.iCachedResource;
						msg.uiMaxAge = g_Resource.uiMaxAge - g_Resource.uiCachedAge;
						//printf("%s:uiMaxAge=%d(%d-%d)\n", __func__, msg.uiMaxAge, g_Resource.uiMaxAge, g_Resource.uiCachedAge);	
						
						//TODO: get current time
						gettimeofday(&tEnd, NULL);
					}
					else
					{
						//TODO: init cache
						initCache();
						
						//TODO: get a fresh resource from a server
						getResourceFromServer(&msg);
						
						//TODO: get current time
						gettimeofday(&tEnd, NULL);
					
						//TODO: set cached with information
						setCache(msg, tEnd);
					}
					
					//TODO: set client process time
					setTimeValue(&(msg.client_recved), 0, 0);
					setTimeValue(&(msg.client_started), 0, 0);
					
					//TODO: set proxy process time
					setTimeValue(&(msg.proxy_recved), tStart.tv_sec, tStart.tv_usec);
					setTimeValue(&(msg.proxy_started), tStart.tv_sec, tStart.tv_usec);
					setTimeValue(&(msg.proxy_finished), tEnd.tv_sec, tEnd.tv_usec);
					
					//TODO: send information as response from proxy to client
					int temp = send(client->iFd, &msg, sizeof(struct __message), 0);
					
					//TODO: set a new scheduled time of client
					struct timeval tB;
					tB.tv_sec = client->uiReqInterval/1000;
					tB.tv_usec = (client->uiReqInterval%1000)*1000;
					addTimeValue(&(client->tSched), client->tSched, tB);
					
				}
				client = client->next;
			}
			
			//TODO: inser client node as time sequence
			//updateClient();
			
			usleep(5*1000);
		}	//if(g_Resource.iClientNumber)
		
		usleep(50*1000);
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
		
	pthread_mutex_init(&m_lock, NULL);
		
	if(argc < 4)
	{
		printf("usage : %s [proxy_port#] [server_ip#] [server_port#] [cache_mode#] [cache_algorithm#]\n", argv[0]);
		exit(0);
	}	//if(argc
	else if(argc < 6)
	{
		g_uiCacheMode = 0;
		g_uiCacheAlgorithm = 0;
	}
	else
	{
		g_uiCacheMode = atoi(argv[4]);
		g_uiCacheAlgorithm = atoi(argv[5]);
	}
	
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
		printf("proxy : socket failed!\n");
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
		printf("proxy : bind failed!\n");
		exit(0);
	}
	
	listen(s, 5);
			
	g_piFdMax = s + 1;

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
				printf("proxy : select failed\n");
				exit(0);
			}//if(select
				
			if(FD_ISSET(s, &read_fds))
			{
				client = sizeof(struct sockaddr_in);
				client_fd = accept(s, (struct sockaddr *)&client_addr, &client);
				if(client_fd == -1)
				{
					printf("proxy : accept failed!\n");
					exit(0);
				}

				g_piSocketClient[g_iClientMax] = client_fd;
				g_iClientMax++;

				//printf("client_fd=%d, g_iClientMax=%d\n", client_fd, g_iClientMax);		
				//send(client_fd, start, strlen(start), 0);
				printf("proxy : user#%d added!\n", g_iClientMax);
			}//if(FD_ISSET

			for(i=0; i<g_iClientMax; i++)
			{
				if(FD_ISSET(g_piSocketClient[i], &read_fds))
				{
					memset(&rcv, 0x0, sizeof(struct __message));
					if((n = recv(g_piSocketClient[i], &rcv, sizeof(struct __message), 0)) <= 0)
					{
						removeObserver(g_piSocketClient[i]);
						dumpObserver();
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
	printf("proxy : user exit, g_piSocketClient[%d]=%d\n", index, g_piSocketClient[index]);
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

