/*
	build : gcc -o resource_proxy2 resource_proxy2.c -lpthread	
	execution : ./resource_proxy2 2048 127.0.0.1 1024 1 0
	1. ./resource_proxy 2048 127.0.0.1 1024 0[cache off] 0[align off]
	2. ./resource_proxy 2048 127.0.0.1 1024 1[cache on] 0[align off]
	3. ./resource_proxy 2048 127.0.0.1 1024 0[cache on] 0[align off]
	4. ./resource_proxy 2048 127.0.0.1 1024 1[cache on] 0[align off]
	5. ./resource_proxy 2048 127.0.0.1 1024 1[cache on] 1[align on]
	6. ./resource_proxy 2048 127.0.0.1 1024 1[cache on] 2[new alogrithm]
1. 그냥할때
2. cache 이용할 때
3. interval 사용할 때
4. interval + cache 사용할 때
5. interval align + cache 사용할 때 (최대 공약수)
6. interval align + cache + prefetch 사용할 때
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

static int g_iExit = 0;
static int g_piFdMax;
static int g_iClientMax = 0;
static int g_piSocketClient[MAX_SOCK] = {0, };

pthread_mutex_t m_lock;

unsigned int g_uiCacheMode = 0;
unsigned int g_uiCacheAlgorithm = 0;
int g_iSocketServer = 0;
struct __resource1 g_Resource1;
struct __resource2 g_Resource2;

int initResource()
{
	g_Resource1.strName[0] = '\0';
	g_Resource1.uiCachedAge = 0;
	g_Resource1.iCachedResource = 0;
	g_Resource1.iClientNumber = 0;
	g_Resource1.next = NULL;
}

int getResource(struct __resource1 *resource, int *iResource, int iCached)
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

int dumpObserver1()
{
	struct __client *client; 
	client = g_Resource1.next;
	
	while(client)
	{
		printf("%s:id=%d, fd=%d, uiReqInterval=%d\n", __func__, client->iId, client->iFd, client->uiReqInterval);
		client = client->next;
	}
		
	return 0;
}


//TODO: first schedule time = now + interval
int addObserver0(int iId, int iFd, unsigned int uiResource, unsigned int uiReqInterval)
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
	
	//
	//TODO: take look later
	//
	gettimeofday(&tNow, NULL);
	addTimeValue(&(pNew->tSched), tNow, tB);
	pNew->tSched.tv_usec = pNew->tSched.tv_usec/(DELAY_PROXY_TIMESLICE*1000)*(DELAY_PROXY_TIMESLICE*1000);

	//printf("tNow=%3ld.%06ld\n", tNow.tv_sec, tNow.tv_usec);
	//printf("pNew->tSched=%3ld.%06ld\n", pNew->tSched.tv_sec, pNew->tSched.tv_usec);
		
	if(g_Resource1.next == NULL)
	{
		g_Resource1.next = pNew;
		g_Resource1.iClientNumber++;
	}
	else
	{	
		pObserver = g_Resource1.next;
				
		fAdd = 1;
		pPrev = NULL;
		while(pObserver)
		{	
			//printf("pNew->uiReqInterval=%d, pObserver->uiReqInterval=%d\n", pNew->uiReqInterval, pObserver->uiReqInterval);
			if(pNew->uiReqInterval < pObserver->uiReqInterval)
			{
				if(pPrev == NULL)
				{
					pNew->next = g_Resource1.next;
					g_Resource1.next = pNew;
				}
				else
				{
					pNew->next = pPrev->next;
					pPrev->next = pNew;
				}
				g_Resource1.iClientNumber++;
				break;
			}
			pPrev = pObserver;
			pObserver = pObserver->next;
			
			if(pObserver == NULL)
			{
					pPrev->next = pNew;
					g_Resource1.iClientNumber++;
			}
		}
	}
		
	return 0;
}


int isMultipleValue(struct timeval *tRet, unsigned int uiReqInterval)
{
	struct __client *pClient = g_Resource1.next;
	int iRet = 0;
	while(pClient)
	{
		if(uiReqInterval == pClient->uiReqInterval)
		{
			setTimeValue(tRet, pClient->tSched.tv_sec, pClient->tSched.tv_usec);
			return  pClient->uiReqInterval;
		}
		else if( (pClient->uiReqInterval/uiReqInterval>1) && (pClient->uiReqInterval%uiReqInterval==0) )
		{
			if(iRet == 0)
			{
				setTimeValue(tRet, pClient->tSched.tv_sec, pClient->tSched.tv_usec);
				iRet = pClient->uiReqInterval;
			}
			else if(iRet > pClient->uiReqInterval)
			{
				setTimeValue(tRet, pClient->tSched.tv_sec, pClient->tSched.tv_usec);
				iRet = pClient->uiReqInterval;
			}
		}
		pClient = pClient->next;
	}
	return iRet;
}

int isDividableValue(struct timeval *tRet, unsigned int uiReqInterval)
{
	struct __client *pClient = g_Resource1.next;
	int iRet = 0;
	while(pClient)
	{
		if(uiReqInterval == pClient->uiReqInterval)
		{
			setTimeValue(tRet, pClient->tSched.tv_sec, pClient->tSched.tv_usec);
			return  pClient->uiReqInterval;
		}
		else if( (uiReqInterval/pClient->uiReqInterval>1) && (uiReqInterval%pClient->uiReqInterval==0) )
		{
			if(iRet == 0)
			{
				setTimeValue(tRet, pClient->tSched.tv_sec, pClient->tSched.tv_usec);
				iRet = pClient->uiReqInterval;
			}
			else if(iRet < pClient->uiReqInterval)
			{
				setTimeValue(tRet, pClient->tSched.tv_sec, pClient->tSched.tv_usec);
				iRet = pClient->uiReqInterval;
			}
		}
		pClient = pClient->next;
	}
	return iRet;
}

int getSchedTime1(struct timeval *tRet, struct timeval tNow, unsigned int uiReqInterval)
{
	struct __client *pClient = g_Resource1.next;
	
	if(g_Resource1.iClientNumber == 0)
	{
		struct timeval tReqInterval;
		tReqInterval.tv_sec = uiReqInterval/1000;
		tReqInterval.tv_usec = uiReqInterval%1000*1000;
		addTimeValue(tRet, tNow, tReqInterval);
		
		printf("0pNew->tSched=%ld.%06ld(%ld.%06ld)\n", tRet->tv_sec, tRet->tv_usec, tNow.tv_sec, tNow.tv_usec);
	}
	else if(g_Resource1.iClientNumber == 1)
	{
		struct timeval tSched = pClient->tSched;
		setTimeValue(tRet, tSched.tv_sec, tSched.tv_usec);
		
		printf("1pNew->tSched=%ld.%06ld\n", tRet->tv_sec, tRet->tv_usec);
	}
	else
	{
		//TODO: the lowest common multiple
		struct timeval tTemp;
		
		if(isMultipleValue(&tTemp, uiReqInterval))
		{
			setTimeValue(tRet, tTemp.tv_sec, tTemp.tv_usec);
		}
		else if(isDividableValue(&tTemp, uiReqInterval))
		{
			setTimeValue(tRet, tTemp.tv_sec, tTemp.tv_usec);
		}
		else
		{
			unsigned int iX = pClient->uiReqInterval;
			struct timeval tAdd;
			struct timeval tArr[10];
			//struct timeval tTemp;
			//addTimeValue(&tTemp, tNow, tAdd);

			int i = 0;
			//TODO: the largest 
			/*
			for(i=0; i<10; i++)
			{
				tB.tv_sec = (iX * i)/1000;
				tB.tv_usec = (iX * i)%1000*1000;
				addTimeValue(&(tArr[i]), tSched, tB);
				printf("%ld.%06ld\n", tArr[i].tv_sec, tArr[i].tv_usec);
			}
			pClient = pClient->next;
			
			while(pClient)
			{
				for(i=0; i<10; i++)
				{
					tB.tv_sec = (iX * i)/1000;
					tB.tv_usec = (iX * i)%1000*1000;
					addTimeValue(&(tArr[i]), tNow, tB);
					//printf("%ld.%06ld\n", tArr[i].tv_sec, tArr[i].tv_usec);
				}
				
				pClient = pClient->next;
			}	
			*/
		}
		printf("2pNew->tSched=%ld.%06ld\n", tRet->tv_sec, tRet->tv_usec);		
	}
}

//TODO: first schedule time = candidate
int addObserver1(int iId, int iFd, unsigned int uiResource, unsigned int uiReqInterval)
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
	setTimeValue(&(pNew->tSched), 0, 0);
	
	struct timeval tNow, tB;
	tB.tv_sec = uiReqInterval/1000;
	tB.tv_usec = (uiReqInterval%1000)*1000;

	//
	//TODO: take look later
	//
	gettimeofday(&tNow, NULL);
	tNow.tv_usec = tNow.tv_usec/1000*1000;
	//addTimeValue(&(pNew->tSched), tNow, tB);
	
	//printf("tNow=%3ld.%06ld\n", tNow.tv_sec, tNow.tv_usec);
	//printf("pNew->tSched=%3ld.%06ld\n", pNew->tSched.tv_sec, pNew->tSched.tv_usec);
	
	setTimeValue(&(pNew->tSched), 0, 0);
	getSchedTime1(&(pNew->tSched), tNow, uiReqInterval);

	if(g_Resource1.next == NULL)
	{
		//setTimeValue(&(pNew->tSched), 0, 0);
		//getSchedTime1(&(pNew->tSched), tNow, uiReqInterval);
		//addTimeValue(&(pNew->tSched), tNow, tB);
		//pNew->tSched.tv_usec = pNew->tSched.tv_usec/(DELAY_PROXY_TIMESLICE*1000)*(DELAY_PROXY_TIMESLICE*1000);
		g_Resource1.next = pNew;
		g_Resource1.iClientNumber++;
	}
	else
	{	
		pObserver = g_Resource1.next;
				
		fAdd = 1;
		pPrev = NULL;
		while(pObserver)
		{	
			//struct timeval temp;
			
			//printf("pNew->uiReqInterval=%d, pObserver->uiReqInterval=%d\n", pNew->uiReqInterval, pObserver->uiReqInterval);
			if(pNew->uiReqInterval < pObserver->uiReqInterval)
			{
				if(pPrev == NULL)
				{
					pNew->next = g_Resource1.next;
					g_Resource1.next = pNew;
				}
				else
				{
					pNew->next = pPrev->next;
					pPrev->next = pNew;
				}
				//setTimeValue(&(pNew->tSched), 0, 0);
				//getSchedTime1(&(pNew->tSched), tNow, uiReqInterval);
				g_Resource1.iClientNumber++;
				break;
			}
			pPrev = pObserver;
			pObserver = pObserver->next;
			
			if(pObserver == NULL)
			{
				//setTimeValue(&(pNew->tSched), 0, 0);
				//getSchedTime1(&(pNew->tSched), tNow, uiReqInterval);
				pPrev->next = pNew;
				g_Resource1.iClientNumber++;
			}
		}
		
		//getSchedTime1(&temp, tNow, uiReqInterval);
		//setTimeValue(&(pNew->tSched), temp.tv_sec, temp.tv_usec);
	}
	
	//printf("pNew->tSched=%3ld.%06ld\n", pNew->tSched.tv_sec, pNew->tSched.tv_usec);
		
	return 0;
}

struct __cache *createCache(void)
{
	struct __cache *pNewCache;
	
	pNewCache = (struct __cache *)malloc(sizeof(struct __cache));
	pNewCache->uiState = CACHE_STATE_INVALID;
	pNewCache->uiMaxAge = 0;
	pNewCache->uiCachedAge = 0;
	setTimeValue(&(pNewCache->tCachedTime), 0, 0);
	pNewCache->nextCache = NULL;
	//pNewCache->uiClientNumber = 0;
	//pNewCache->nextClient = NULL;
	
	return pNewCache;
}

struct __client *createClient(void)
{
	struct __client *pClient;
	
	pClient = (struct __client *)malloc(sizeof(struct __client));
	pClient->iId = 0;
	pClient->iFd = 0;
	pClient->uiReqInterval = 0;
	setTimeValue(&(pClient->tSched), 0, 0);
	pClient->next = NULL;	
		
	return pClient;
}

struct __sched *createSched(struct timeval timeSched)
{
	struct __sched *pSched;
	
	pSched = (struct __sched *)malloc(sizeof(struct __sched));
	setTimeValue(&(pSched->tSchedTime), timeSched.tv_sec, timeSched.tv_usec);
	pSched->uiClientNumber = 0;
	pSched->nextClient = NULL;
	
	return pSched;
}

int dumpObserver2(void)
{
	struct __sched *pSched = g_Resource2.nextSched;

	while(pSched)
	{
		struct __client *pClient = pSched->nextClient;
		while(pClient)
		{
			printf("(id=%d, fd=%d) ", pClient->iId, pClient->iFd);
			pClient = pClient->nextClient;
		}
		pSched = pSched->nextSched;
	}	
	return 0;
}

int removeClientFromObserverList(struct __client *client)
{
	
}

int insertClientToObserverList(struct __client *client)
{
	struct __sched *pSched = g_Resource2.nextSched;
	int iFound = 0;
		
	while(pSched)
	{
		if(isEqualTo(client->tSched, pSched->tSchedTime))
		{
			struct __client *pClient = pSched->nextClient;
			pSched->nextClient = client;
			client->nextClient = pClient;
			pSched->uiClientNumber++;
			iFound = 1;
			break;
		}
		else if(isBiggerThan(pSched->tSchedTime, client->tSched))
		{
			//TODO: create sched and add client
			iFound = 1;
			break;
		}
		else	//if client->tsched < psched->tSched
		{
			pSched = pSched->nextSched;
			
			if( (pSched==NULL) && (iFound==0) )
			{
				//TODO: 
			}
		}
	}
	
	
	
	return 0;
}

//TODO: add observer into any list
int addObserver2(int iId, int iFd, unsigned int uiResource, unsigned int uiReqInterval, struct timeval tStart)
{
	struct __cache *pCache;
	struct __sched *pSched;
	struct __client *pNewClient;
	struct __client *pClient;
	struct __client *pPrev;
	int fAdd;
	
	struct __message msg = {0x0, };
	
	//TODO: if there is no cache
	if( (g_Resource2.uiSchedNumber==0) && (g_Resource2.nextSched==NULL) )
	{
		struct timeval tAdd, tResult;
		setTimeValue(&tAdd, uiReqInterval/1000, (uiReqInterval%1000)*1000);
		//gettimeofday(&tNow, NULL);
		addTimeValue(&tResult, tStart, tAdd);
		
		g_Resource2.nextSched = (struct __sched *)createSched(tResult);
		g_Resource2.uiSchedNumber++;
	}	
	pSched = g_Resource2.nextSched;
	
	//while(pSched)
	{
		pNewClient = (struct __client *)createClient();
		pNewClient->uiReqInterval = uiReqInterval;
		pNewClient->iId = iId;
		pNewClient->iFd = iFd;
	
		if(pSched->nextClient == NULL)
		{
			pSched->nextClient = pNewClient;
			pSched->uiClientNumber++;
		}
		else
		{	
			pClient = pSched->nextClient;
			fAdd = 1;
			while(pClient)
			{
				if(pClient->iId == iId)
				{
					if(pClient->iFd != iFd)
						pClient->iFd = iFd;
					fAdd = 0;
					break;
				}
				else 
				{
					pPrev = pClient;
					pClient = pClient->nextClient;
				}
				
			}	//while(pClient)
			
			if(fAdd)
			{
				pPrev->nextClient = pNewClient;
				pSched->uiClientNumber++;
				
				//TODO: if threre is no scheduled observer, then set first schedule
				/*
				if( (pSched->tSchedTime.tv_sec==0) && (pSched->tSchedTime.tv_usec==0) )
				{
					struct timeval tNow, tAdd;
					setTimeValue(&tAdd, uiReqInterval/1000, (uiReqInterval%1000)*1000);
					gettimeofday(&tNow, NULL);
					addTimeValue(&(pSched->tSchedTime), tNow, tAdd);
				}
				*/
			}
		}
	}//while(pSched)
		
	return 0;
}

void *pthreadWatchResource2(void *arg)
{
	//struct timeval timeSched;
	struct timeval tStart;
			
	while(!g_iExit)
	{
		if( (g_Resource2.uiSchedNumber>0) && (g_Resource2.nextSched) )
		{
			struct __sched *sched = g_Resource2.nextSched;
			
			gettimeofday(&tStart, NULL);
			
			while(sched)
			{
				//TODO: find scheduled client
				if(isBiggerThan(tStart, sched->tSchedTime)) 
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
						//msg.resource = g_Resource2.iCachedResource;
						//msg.uiMaxAge = g_Resource2.uiMaxAge - g_Resource2.uiCachedAge;
						
						//TODO: get current time
						gettimeofday(&tEnd, NULL);
					}
					else
					{
						//TODO: init cache
						//initCache();
						
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
					
					
					struct __client *client = sched->nextClient;
					while(client)
					{
						//TODO: send information as response from proxy to client
						int temp = send(client->iFd, &msg, sizeof(struct __message), 0);
						
						
						//TODO: remove client
						
						//TODO: update client
						
						//TODO: insert client
						struct __client *tmpClient = client;
						struct timeval tNew, tTemp;
						setTimeValue(&tTemp, tmpClient->uiReqInterval/1000, (tmpClient->uiReqInterval%1000)*1000);
						addTimeValue(&tNew, tStart, tTemp);
						setTimeValue(&tmpClient->tSched, tNew.tv_sec, tNew.tv_usec);
						//insertObserver(tmpClient);
						
						
						client = client->nextClient;
					}
					
					//TODO: set a new scheduled time of client
					/*
					struct timeval tB;
					tB.tv_sec = client->uiReqInterval/1000;
					tB.tv_usec = (client->uiReqInterval%1000)*1000;
					addTimeValue(&(client->tSched), client->tSched, tB);
					*/
					
					//TODO: remove this sched
					
				}
				sched = sched->nextSched;
			}//while(sched)
		}
		
		usleep(5*1000);
	}
	
}

int removeObserver(int iFd)
{
	struct __client *pClient; 
	struct __client *pFront; 
	struct __client *pRear; 
	struct __client *pTemp;
	
	if(g_Resource1.next == NULL)
		return 0;

	pClient = g_Resource1.next;

	if(iFd == pClient->iFd)
	{
		g_Resource1.next = pClient->next;
		free(pClient);
		g_Resource1.iClientNumber--;
		return 0;
	}

	while(pClient)
	{
		if(iFd == pClient->iFd)
		{
			pFront->next = pClient->next;
			free(pClient);
			g_Resource1.iClientNumber--;
			break;
		}
		pFront = pClient;
		pClient = pClient->next;
	}

	return 0;
}

int initCache()
{
	g_Resource1.iCachedResource = 0;
	g_Resource1.uiMaxAge = 0;
	g_Resource1.uiCachedAge = 0;
	setTimeValue(&(g_Resource1.tCachedTime), 0, 0);
	
	return 0;
}

int setCache(struct __message msg, struct timeval tNow)
{
	g_Resource1.iCachedResource = msg.resource;
	g_Resource1.uiMaxAge = msg.uiMaxAge;
	//g_Resource1.uiCachedAge = 0;
	setTimeValue(&(g_Resource1.tCachedTime), tNow.tv_sec, tNow.tv_usec);
	
}

int isCachedDataValid(struct timeval curTime)
{
	struct timeval timeB;
	struct timeval timeRet;
	int ret = 0;
	
	if( (g_Resource1.uiCachedAge<g_Resource1.uiMaxAge) && (g_Resource1.tCachedTime.tv_sec>0) )
	{
		struct timeval tMaxAge;
		struct timeval tTemp;
		setTimeValue(&tMaxAge, g_Resource1.uiMaxAge/1000, (g_Resource1.uiMaxAge%1000)*1000);
		addTimeValue(&tTemp, g_Resource1.tCachedTime, tMaxAge);
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

	if(isBiggerThan(curTime, g_Resource1.tCachedTime))
	{
		subTimeValue(&temp, curTime, g_Resource1.tCachedTime);
		g_Resource1.uiCachedAge = temp.tv_sec*1000 + temp.tv_usec/1000;
	}
	else
	{
		g_Resource1.uiCachedAge = 0;
	}
	
	//printf("%s:g_Resource1.uiCachedAge=%d\n", __func__, g_Resource1.uiCachedAge);

	return ret;
}

int handleMessage(struct __message *arg)
{
	struct timeval tStart;
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
//#if CONFIG_MODE_OBSERVER1
		if(g_uiCacheAlgorithm == 0)
		{
			addObserver0(msg.owner, msg.iFd, msg.resource, msg.req_dur);
		}
		else if(g_uiCacheAlgorithm == 1)
		{
			addObserver1(msg.owner, msg.iFd, msg.resource, msg.req_dur);
		}
		dumpObserver1();
//#else
//		addObserver2(msg.owner, msg.iFd, msg.resource, msg.req_dur, tStart);
//		dumpObserver2();
//#endif		
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
			gettimeofday(&tEnd, NULL);
			
			//TODO: set message with time information		
			msg.resource = g_Resource1.iCachedResource;
			msg.uiMaxAge = g_Resource1.uiMaxAge - g_Resource1.uiCachedAge;

			printf("CACHED! Owner=%d, R=%d, MaxAge=%d, CachedAge=%d(%ld.%06ld)\n", 
				msg.owner,
				g_Resource1.iCachedResource, 
				g_Resource1.uiMaxAge,
				g_Resource1.uiCachedAge,
				tEnd.tv_sec%1000,	\
				tEnd.tv_usec
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
		
			printf("NOT cached! Owner=%d, R=%d, MaxAge=%d, CachedAge=%d(%ld.%06ld)\n",
				msg.owner,
				g_Resource1.iCachedResource, 
				g_Resource1.uiMaxAge,
				g_Resource1.uiCachedAge,
				tEnd.tv_sec%1000,	\
				tEnd.tv_usec
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

void dumpCurrentTime()
{
	struct timeval timeCurrent;	
	
	gettimeofday(&timeCurrent, NULL);
}
#if 1
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
#endif

/*
int getFirstScheduledTime(struct timeval *tRet)
{
	struct __client *client = g_Resource1.next;
	struct timeval tCacheExpired;
	struct timeval tMaxAge;
	
	setTimeValue(&tMaxAge, g_Resource1.uiMaxAge/1000, (g_Resource1.uiMaxAge%1000)*1000);
	
	addTimeValue(&tCacheExpired, g_Resource1.tCachedTime, tMaxAge);
	
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
		if(g_Resource1.iClientNumber)
		{
			//TODO: get current time
			//gettimeofday(&tStart, NULL);

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
			struct __client *client = g_Resource1.next;
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
			struct __client *client = g_Resource1.next;
			while(client)
			{
				gettimeofday(&tStart, NULL);
				
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
						msg.resource = g_Resource1.iCachedResource;
						msg.uiMaxAge = g_Resource1.uiMaxAge - g_Resource1.uiCachedAge;
						//printf("%s:uiMaxAge=%d(%d-%d)\n", __func__, msg.uiMaxAge, g_Resource1.uiMaxAge, g_Resource1.uiCachedAge);	
						
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
		}	//if(g_Resource1.iClientNumber)
		
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
/*
addObserver1(200, 2, 1000, 800);
addObserver1(300, 3, 1000, 1200);
addObserver1(100, 1, 1000, 400);
//dumpObserver1();
//dumpObserver1();
dumpObserver1();
return;//while(1);
*/
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
#if CONFIG_MODE_OBSERVER1
	pthread_create(&threadId, NULL, pthreadWatchResource, (void *)NULL);
#else	
	pthread_create(&threadId, NULL, pthreadWatchResource2, (void *)NULL);
#endif	

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
						dumpObserver1();
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

