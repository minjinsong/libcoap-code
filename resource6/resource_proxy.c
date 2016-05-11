/*
	build : gcc -o resource_proxy2 resource_proxy2.c -lpthread	
	execution : ./resource_proxy2 2048 127.0.0.1 1024 1 0
	1. ./resource_proxy 2048 127.0.0.1 1024 0[cache off] 0[align off]
	2. ./resource_proxy 2048 127.0.0.1 1024 1[cache on] 0[align off]
	3. ./resource_proxy 2048 127.0.0.1 1024 0[cache on] 0[align off]
	4. ./resource_proxy 2048 127.0.0.1 1024 1[cache on] 0[align off]
	5. ./resource_proxy 2048 127.0.0.1 1024 1[cache on] 1[align on]
	6. ./resource_proxy 2048 127.0.0.1 1024 1[cache on] 2[new alogrithm]
1. �׳��Ҷ�
2. cache �̿��� ��
3. interval ����� ��
4. interval + cache ����� ��
5. interval align + cache ����� �� (�ִ� �����)
6. interval align + cache + prefetch ����� ��
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
static char g_strProxyLog[128] = {0, };

pthread_mutex_t m_lock;
static int g_iTotal = 0;

unsigned int g_uiCacheMode = 0;
unsigned int g_uiCacheAlgorithm = 0;
int g_iSocketServer = 0;
struct __resource1 g_Resource1;

int initResource()
{
	g_Resource1.strName[0] = '\0';
	g_Resource1.uiCachedAge = 0;
	g_Resource1.iCachedResource = 0;
	g_Resource1.iClientNumber = 0;
	g_Resource1.next = NULL;
}

int dumpObserver()
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

int dumpMessage(struct __message msg)
{
	struct timeval timeTrans;
	struct timeval tSend1;
	struct timeval tSend2;
	struct timeval tServer;
	struct timeval tRecv2;
	struct timeval tRecv1;
	int iCached;
	
	
	//TODO: dump response
	if(msg.proxy_started.tv_sec)
		subTimeValue(&timeTrans, msg.proxy_finished, msg.proxy_started);	

	if( (msg.server_started.tv_sec>0) && (msg.server_finished.tv_sec>0) )
	{
		subTimeValue(&tSend2, msg.server_started, msg.proxy_started);
		subTimeValue(&tServer, msg.server_finished, msg.server_started);
		subTimeValue(&tRecv2, msg.proxy_finished, msg.server_finished);
		iCached = 0;
	}
	else
	{
		tSend2.tv_sec = 0;
		tSend2.tv_usec = 0;
		tServer.tv_sec = 0;
		tServer.tv_usec = 0;
		tRecv2.tv_sec = 0;
		tRecv2.tv_usec = 0;
		iCached = 1;
	}
	
	subTimeValue(&tRecv1, msg.client_finished, msg.proxy_finished);
	
	printf("[%d-%d]R=%d; MaxAge=%d; Cached=%d, Rsp=%ld.%06ld;(%ld.%06ld)(%ld.%06ld)(%ld.%06ld)\n", 
			msg.owner,	\
			msg.cnt,	\
			msg.resource,	\
			msg.uiMaxAge,	\
			iCached,	\
			timeTrans.tv_sec%1000,	\
			timeTrans.tv_usec,	\
			tSend2.tv_sec%1000,	\
			tSend2.tv_usec,	\
			tServer.tv_sec%1000,	\
			tServer.tv_usec,	\
			tRecv2.tv_sec%1000,	\
			tRecv2.tv_usec
	);
	
	FILE *pfileLog = fopen(g_strProxyLog, "a");
	
	fprintf(pfileLog, "[%d-%d]R=%d; MaxAge=%d; Cached=%d, Rsp=%ld.%06ld, %ld.%06ld, %ld.%06ld\n", 
			msg.owner,	\
			msg.cnt,	\
			msg.resource,	\
			msg.uiMaxAge,	\
			iCached,	\
			timeTrans.tv_sec%1000,	\
			timeTrans.tv_usec,	\
			msg.proxy_started.tv_sec%1000,	\
			msg.proxy_started.tv_usec,	\
			msg.proxy_finished.tv_sec%1000,	\
			msg.proxy_finished.tv_usec
	);

	fclose(pfileLog);
	
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

int getGCD(int a, int b)
{
	if(b==0) 
		return a;
	else
		return getGCD(b, a%b);
}

int getLCM(int a, int b)
{
	return (a*b)/getGCD(a, b);
}

int updateBaseTime()
{
	int ret;
	struct __client *pClient;
	
	pClient = g_Resource1.next;
	ret = pClient->uiReqInterval;
		
	while(pClient)
	{
		if((pClient->next!=NULL) && (pClient->next->uiReqInterval))
		{
			ret = getLCM(ret, pClient->next->uiReqInterval);
		}
		pClient = pClient->next;
	}

	struct timeval tTemp;
	setTimeValue(&tTemp, ret/1000, (ret%1000)*1000);
	addTimeValue(&(g_Resource1.tBaseTime), g_Resource1.tBaseTime, tTemp);

	//printf("ret=%d\n", ret);
	return ret;
}

int getSchedTime(struct timeval *tRet, struct timeval tNow, unsigned int uiReqInterval)
{
	struct __client *pClient = g_Resource1.next;
	struct timeval tReqInterval;
	
	tReqInterval.tv_sec = uiReqInterval/1000;
	tReqInterval.tv_usec = uiReqInterval%1000*1000;
	
	if(g_Resource1.iClientNumber == 0)
	{
		tRet->tv_sec = tNow.tv_sec+1;
		tRet->tv_usec = 0;

		addTimeValue(&(g_Resource1.tBaseTime), *tRet, tReqInterval);

	}
	else 
	{
		//struct timeval tSched = pClient->tSched;
		struct timeval tSched;
		setTimeValue(&tSched, g_Resource1.tBaseTime.tv_sec, g_Resource1.tBaseTime.tv_usec);
		
		while(1)
		{
			struct timeval tTemp = tSched;
			subTimeValue(&tTemp, tTemp, tReqInterval);
			
			if(isBiggerThan(tTemp, tNow))
			{
				tSched = tTemp;
			}
			else
			{
				break;
			}
		}
		setTimeValue(tRet, tSched.tv_sec, tSched.tv_usec);
	}
		
	printf("Req=%d;S(%ld.%06ld);B(%ld.%06ld);N(%ld.%06ld)\n", 
			uiReqInterval,
			tRet->tv_sec, tRet->tv_usec, 
			g_Resource1.tBaseTime.tv_sec, g_Resource1.tBaseTime.tv_usec,
			tNow.tv_sec, tNow.tv_usec);
}

//TODO: first schedule time = candidate
int addObserver1(int iId, int iFd, unsigned int uiResource, unsigned int uiReqInterval, struct timeval tNow)
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
	getSchedTime(&(pNew->tSched), tNow, uiReqInterval);
		
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
#if 0
	if(g_uiCacheAlgorithm == 2)
		g_Resource1.uiMaxAge = DELAY_PROXY_TIMESLICE;
	else
		g_Resource1.uiMaxAge = RESOURCE_DEFAULT_DELAY;
#else
		//g_Resource1.uiMaxAge = msg.uiMaxAge;
		g_Resource1.uiMaxAge = RESOURCE_DEFAULT_DELAY;
#endif		
	//g_Resource1.uiCachedAge = 0;
	setTimeValue(&(g_Resource1.tCachedTime), tNow.tv_sec, tNow.tv_usec);
	
}

int isCachedDataValid(struct timeval curTime)
{
	struct timeval timeB;
	struct timeval timeRet;
	int ret = 0;
//printf("%s:g_Resource1.uiCachedAge=%d, g_Resource1.uiMaxAge=%d\n", __func__, g_Resource1.uiCachedAge, g_Resource1.uiMaxAge);

	if( (g_Resource1.uiCachedAge<g_Resource1.uiMaxAge) && (g_Resource1.tCachedTime.tv_sec>0) )
	{
		struct timeval tMaxAge;
		struct timeval tTemp;
		setTimeValue(&tMaxAge, g_Resource1.uiMaxAge/1000, (g_Resource1.uiMaxAge%1000)*1000);
		addTimeValue(&tTemp, g_Resource1.tCachedTime, tMaxAge);
		//printf("%s:tMaxAge=%6ld.%06ld\n", __func__, tMaxAge.tv_sec, tMaxAge.tv_usec);
		printf("%s:tTemp=%6ld.%06ld,curTime=%6ld.%06ld\n", __func__, 
			tTemp.tv_sec, tTemp.tv_usec,
			curTime.tv_sec, curTime.tv_usec);	
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
	//gettimeofday(&tStart, NULL);

	//TODO: wait until other request responded
	pthread_mutex_lock(&m_lock);
	
	gettimeofday(&tStart, NULL);

	//TODO: use cached resource
	if(msg.cmd == RESOURCE_CMD_REGISTER)
	{
		if(g_uiCacheAlgorithm == 0)
		{
			addObserver0(msg.owner, msg.iFd, msg.resource, msg.req_dur);
		}
		else if(g_uiCacheAlgorithm == 1)
		{
			addObserver1(msg.owner, msg.iFd, msg.resource, msg.req_dur, tStart);
		}
		else if(g_uiCacheAlgorithm == 2)
		{
			addObserver1(msg.owner, msg.iFd, msg.resource, msg.req_dur, tStart);
		}
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
#if 1
	dumpMessage(msg);
#endif
	}
	
	pthread_mutex_unlock(&m_lock);
//printf("---\n");		
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

//struct timeval g_tMin;

int getEarlistSchedTime(struct timeval *ptMin, struct timeval curTime)
{
	int ret = 0;
	struct __client *pClient;
	
	pClient = g_Resource1.next;
		
	while(pClient)
	{
		//if(pClient->tSched)
		{
			if(isBiggerThan(*ptMin, pClient->tSched))
			{
				setTimeValue(ptMin, pClient->tSched.tv_sec, pClient->tSched.tv_usec);
			}
		}
				
		pClient = pClient->next;
	}

	return ret;
}

#if 1
int setSchedTime(struct timeval tMin)
{
	int ret = 0;
	struct __client *pClient;
	struct timeval tRet;
	
	pClient = g_Resource1.next;
		
	while(pClient)
	{
		struct timeval tCPaddMA; 
		struct timeval tMaxAge; 
		setTimeValue(&tMaxAge, g_Resource1.uiMaxAge/1000, (g_Resource1.uiMaxAge%1000)*1000);
		addTimeValue(&tCPaddMA, g_Resource1.tBaseTime, tMaxAge);
		
		if(isBiggerThan(pClient->tSched, tCPaddMA))
		{
			struct timeval tTemp;
			struct timeval tAllow;
			subTimeValue(&tTemp, pClient->tSched, tCPaddMA);
			
			setTimeValue(&tAllow, (pClient->uiReqInterval*2/10)/1000, ((pClient->uiReqInterval*2/10)%1000)*1000);
			//setTimeValue(&tAllow, (pClient->uiReqInterval*3/10)/1000, ((pClient->uiReqInterval*3/10)%1000)*1000);
			//setTimeValue(&tAllow, (pClient->uiReqInterval/10)/1000, ((pClient->uiReqInterval/10)%1000)*1000);
			//setTimeValue(&tAllow, (pClient->uiReqInterval/5)/1000, ((pClient->uiReqInterval/5)%1000)*1000);
			//setTimeValue(&tAllow, (pClient->uiReqInterval/4)/1000, ((pClient->uiReqInterval/4)%1000)*1000);
			//setTimeValue(&tAllow, (pClient->uiReqInterval/2)/1000, ((pClient->uiReqInterval/2)%1000)*1000);
			//setTimeValue(&tAllow, 0, 10*1000);
			if(isBiggerThan(tAllow, tTemp))
			{
				//TODO: set new schedule time
				//setTimeValue(&(pClient->tSched), tCPaddMA.tv_sec , tCPaddMA.tv_usec);
				setTimeValue(&(pClient->tSched), g_Resource1.tBaseTime.tv_sec , g_Resource1.tBaseTime.tv_usec);
			}
			else
			{
				//TODO: leave it for next processing
				;
			}
		}
		else
		{
			setTimeValue(&(pClient->tSched), g_Resource1.tBaseTime.tv_sec , g_Resource1.tBaseTime.tv_usec);
		}
					
		pClient = pClient->next;
	}

	return ret;
}
#else
int setSchedTime(struct timeval tMin)
{
	int ret = 0;
	struct __client *pClient;
	struct timeval tRet;
	
	pClient = g_Resource1.next;
		
	while(pClient)
	{
		if(isEqualTo(pClient->tSched, tMin))
		{
			;
			//printf("Equal!\n");
		}
		else
		{
			subTimeValue(&tRet, pClient->tSched, tMin);
			addTimeValue(&tRet, tRet, tRet);
			
			struct timeval tB;
			tB.tv_sec = pClient->uiReqInterval/1000;
			tB.tv_usec = (pClient->uiReqInterval%1000)*1000;
			
			//printf("NOT Equal!\n");
			
			if(isBiggerThan(tB, tRet))
			{
				setTimeValue(&(pClient->tSched), tMin.tv_sec , tMin.tv_usec);
			}
		}
					
		pClient = pClient->next;
	}

	return ret;
}
#endif

int setSchedule(struct timeval curTime)
{
	struct timeval tRet;

	setTimeValue(&tRet, 0x7FFFFFFF, 0x7FFFFFFF);
		
	//TODO: get the earliest schedule time
	getEarlistSchedTime(&tRet, curTime);
	
	//TODO: set base time
	setTimeValue(&g_Resource1.tBaseTime, tRet.tv_sec, tRet.tv_usec);
				
	//TODO: reschedule
	setSchedTime(tRet);
}

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

					//TODO: dump message
					dumpMessage(msg);
					
					//TODO: send information as response from proxy to client
					int temp = send(client->iFd, &msg, sizeof(struct __message), 0);
					
					//TODO: set a new scheduled time of client
					struct timeval tB;
					tB.tv_sec = client->uiReqInterval/1000;
					tB.tv_usec = (client->uiReqInterval%1000)*1000;
					addTimeValue(&(client->tSched), client->tSched, tB);
					
//printf("tStart=%ld.%06ld, tSched=%ld.%06ld\n", 
//	tStart.tv_sec, tStart.tv_usec,
//	client->tSched.tv_sec, client->tSched.tv_usec);
				}
				client = client->next;
//printf("%s:client=0x%x\n", __func__, (unsigned int)client);
			}
			
			//TODO: inser client node as time sequence
			//updateClient();
			
			//TODO: 
			if(g_uiCacheAlgorithm == 1)
			{
				if(isBiggerThan(tStart, g_Resource1.tBaseTime))
				{
					//TODO: update check point
					updateBaseTime();
				}
			}
			else if(g_uiCacheAlgorithm == 2)
			{
				struct timeval tCPaddMA; 
				struct timeval tMaxAge; 
				setTimeValue(&tMaxAge, g_Resource1.uiMaxAge/1000, (g_Resource1.uiMaxAge%1000)*1000);
				addTimeValue(&tCPaddMA, g_Resource1.tBaseTime, tMaxAge);				
				if(isBiggerThan(tStart, tCPaddMA))
				{
					setSchedule(tStart);
				}
			}			
			
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
struct timeval tNow;
gettimeofday(&tNow, NULL);
addObserver1(200, 2, 1000, 800, tNow);
usleep(10*1000);
gettimeofday(&tNow, NULL);
addObserver1(300, 3, 1000, 2000, tNow);
usleep(25*1000);
gettimeofday(&tNow, NULL);
addObserver1(100, 1, 1000, 400, tNow);
//dumpObserver();
//dumpObserver();
dumpObserver();
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
	pthread_create(&threadId, NULL, pthreadWatchResource, (void *)NULL);
	
	sprintf(g_strProxyLog, "/tmp/proxy_%d.log", (int)getpid());

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

