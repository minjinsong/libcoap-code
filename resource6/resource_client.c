/*
	build : gcc -o resource_client2 resource_client2.c
	execution : ./resource_client2 127.0.0.1 2048 1 800
	1. ./resource_client2 127.0.0.1 2048 0[request] 800[interval]
	2. ./resource_client2 127.0.0.1 2048 0[request] 800[interval]
	3. ./resource_client2 127.0.0.1 2048 1[observe] 800[interval]
	4. ./resource_client2 127.0.0.1 2048 1[observe] 800[interval]
	5. ./resource_client2 127.0.0.1 2048 1[observe] 800[interval]
	6. ./resource_client2 127.0.0.1 2048 1[observe] 800[interval] 1[log_file] 20[log_count]
*/
 
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>
#include "resource.h"

unsigned int g_uiOwner;
int g_iLog = 0;

#define ENABLE_REPEATE				1		//1:send message to server repeatly
char g_strLogName[128] = {0, };
//FILE *fLog;
int g_iLogCount = 100;

int initMessage(struct __message *msg)
{
	struct timeval timeStart;
	
	msg->cnt++;
	msg->req_dur = 100;

	gettimeofday(&timeStart, NULL);
	msg->client_started.tv_sec = timeStart.tv_sec;
	msg->client_started.tv_usec = timeStart.tv_usec;
	return 0;
}


static int g_iTotal = 0;

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
	//if(msg.client_started.tv_sec)
	//	subTimeValue(&timeTrans, msg.client_finished, msg.client_started);
	//else if(msg.proxy_recved.tv_sec)
	//	subTimeValue(&timeTrans, msg.client_finished, msg.proxy_recved);
	//else if(msg.proxy_started.tv_sec)
	if(msg.proxy_started.tv_sec)
		subTimeValue(&timeTrans, msg.client_finished, msg.proxy_started);	
	/*
	if(msg.client_started.tv_sec)
		subTimeValue(&tSend1, msg.proxy_started, msg.client_started);
	else 
	{
		tSend1.tv_sec = 0;
		tSend1.tv_usec = 0;
	}
	*/
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
	/*
	printf("tRecv1=%ld.%06ld, client_finished=%ld.%06ld, proxy_finished=%ld.%06ld\n", 
		tRecv1.tv_sec,
		tRecv1.tv_usec,
		msg.client_finished.tv_sec,
		msg.client_finished.tv_usec,
		msg.proxy_finished.tv_sec,
		msg.proxy_finished.tv_usec);
	*/
	if(msg.owner == 0)
	{
		msg.owner = g_uiOwner;
	}
#if 0	
	printf("[%d-%d]R=%d, MaxAge=%d, Cached=%d, Rsp=%ld.%06ldms(%ld.%06ld)(%ld.%06ld)(%ld.%06ld)(%ld.%06ld)\n", 
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
		tRecv2.tv_usec,	\
		tRecv1.tv_sec%1000,	\
		tRecv1.tv_usec
	);
#else
	printf("[%d-%d]R=%d, MaxAge=%d, Cached=%d, Rsp=%ld.%06ldms, %ld.%06ld, %ld.%06ld \n", 
		msg.owner,	\
		msg.cnt,	\
		msg.resource,	\
		msg.uiMaxAge,	\
		iCached,	\
		timeTrans.tv_sec%1000,	\
		timeTrans.tv_usec,	\
		msg.client_started.tv_sec%1000,	\
		msg.client_started.tv_usec,	\
		msg.client_finished.tv_sec%1000,	\
		msg.client_finished.tv_usec
	);
#endif	
	
	if((g_iTotal++>=10) && (g_iTotal<=(g_iLogCount+10)) && g_iLog)
	{
		FILE *pfileLog = fopen(g_strLogName, "a");
		/*
		fprintf(pfileLog, "Hello Minjin!MaxAge=%d\n", msg.uiMaxAge);
		fprintf(pfileLog, "Hello Minjin!owner=%d\n", msg.owner);
		*/
#if 0		
		fprintf(pfileLog, "[%d-%d]R=%d; MaxAge=%d; Cached=%d, Rsp=%ld.%06ld;(%ld.%06ld)(%ld.%06ld)(%ld.%06ld)(%ld.%06ld)\n", 
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
			tRecv2.tv_usec,	\
			tRecv1.tv_sec%1000,	\
			tRecv1.tv_usec
		);
#else
		fprintf(pfileLog, "[%d-%d]R=%d; MaxAge=%d; Cached=%d, Rsp=%ld.%06ld, %ld.%06ld, %ld.%06ld\n", 
			msg.owner,	\
			msg.cnt,	\
			msg.resource,	\
			msg.uiMaxAge,	\
			iCached,	\
			timeTrans.tv_sec%1000,	\
			timeTrans.tv_usec,	\
			msg.client_started.tv_sec%1000,	\
			msg.client_started.tv_usec,	\
			msg.client_finished.tv_sec%1000,	\
			msg.client_finished.tv_usec
		);
#endif		
		fclose(pfileLog);
	}

	return 0;
}

int handleMessage(struct __message msg)
{
	struct timeval timeFinished;
	
	gettimeofday(&timeFinished, NULL);
	msg.client_finished.tv_sec = timeFinished.tv_sec;
	msg.client_finished.tv_usec = timeFinished.tv_usec;
	
	dumpMessage(msg);
	
	return 0;
}

int g_monMode = RESOURCE_CMD_GET;
int g_monInterval = 1000;

int main(int argc, char *argv[])
{
	char line[MAXLINE], message[MAXLINE+1];
	int n, pid;
	struct sockaddr_in proxy_addr;
	int maxfdp1;
	int s;
	fd_set read_fds;
	
	struct __message msg = {0x0, };
	struct __message resp = {0x0, };
	
	if(argc < 3)
	{
		printf("usage1 : %s [proxy_ip#] [proxy_port#]\n", argv[0]);
		printf("usage2 : %s [proxy_ip#] [proxy_port#] [monitoring_mode#] [monitoring_interval#]\n", argv[0]);
		exit(0);
	}	//if(argc
	else if(argc >= 5)
	{
		g_monMode = atoi(argv[3]);
		g_monInterval = atoi(argv[4]);
		
		if(argc >= 6)
		{
			//strncpy(g_strLogName, argv[5], strlen(argv[5]));
			g_iLog = atoi(argv[5]);
			printf("g_iLog = %d\n", g_iLog);
			
			if(argc == 7)
			{
				g_iLogCount = atoi(argv[6]);
			}
		}
		else
		{
			strncpy(g_strLogName, CLIENT_LOG_NAME, strlen(CLIENT_LOG_NAME));
		}
	}

	if( (s=socket(PF_INET, SOCK_STREAM, 0)) < 0 )
	{
		printf("client : socket failed!\n");
		exit(0);
	}	//if( (s=socket
	
	
	bzero((char *)&proxy_addr, sizeof(struct sockaddr_in));
	proxy_addr.sin_family = AF_INET;
	proxy_addr.sin_addr.s_addr = inet_addr(argv[1]);
	proxy_addr.sin_port = htons(atoi(argv[2]));
	
	if(connect(s, (struct sockaddr *)&proxy_addr, sizeof(struct sockaddr_in)) < 0)
	{
		printf("client : connect failed!\n");
		exit(0);
	}
	else
	{
		printf("client : connected server!\n");
	}	//if(connect(
	
	maxfdp1 = s + 1;
	FD_ZERO(&read_fds);
	
	memset(&msg, 0x0, sizeof(struct __message));
	msg.owner = (int)getpid();
	
	sprintf(g_strLogName, "/tmp/client_%d.log", (int)getpid());
	//printf("g_strLogName=%s\n", g_strLogName); 
	g_uiOwner = msg.owner;
	//printf("CLIEND:owner=%d\n", msg.owner);
	
	if(g_monMode == RESOURCE_CMD_REGISTER)
	{
		initMessage(&msg);
		msg.cmd = RESOURCE_CMD_REGISTER;
		msg.req_dur = g_monInterval;
		if(send(s, &msg, sizeof(struct __message), 0) < 0)
		{
			printf("client : send failed!\n");
		}	//if(send(
	}

	//while(1)
	while(g_iTotal<=(g_iLogCount+10))
	{
		FD_SET(0, &read_fds);
		FD_SET(s, &read_fds);

		if(g_monMode == RESOURCE_CMD_GET)
		{
			initMessage(&msg);
			msg.cmd = RESOURCE_CMD_GET;

			if(send(s, &msg, sizeof(struct __message), 0) < 0)
			{
				printf("client : send failed!\n");
			}	//if(send(
		}
#if 0
		//usleep(20*1000);

		if(select(maxfdp1, &read_fds, (fd_set *)0, (fd_set *)0, (struct timeval *)0) < 0)
		{
			printf("client : select failed\n");
			exit(0);
		}//if(select

		if(FD_ISSET(s, &read_fds))
		{
			int size;
			if((size = recv(s, &resp, sizeof(struct __message), 0)) > 0)
			{
				handleMessage(resp);
			} //if((size
		}	//if(FD_ISSET(
#else
			while(recv(s, &resp, sizeof(struct __message), 0) < 0)
			{
				usleep(1000);
				printf("waiting..\n");
			} //if((size
			handleMessage(resp);
#endif
		if(g_monMode == RESOURCE_CMD_GET)
		{
			//usleep(g_monInterval*1000);
			struct timeval timeSched;
			struct timeval timeB;
			struct timeval timeNow;
			
			timeB.tv_sec = g_monInterval/1000;
			timeB.tv_usec = 	(g_monInterval%1000)*1000;
			
			addTimeValue(&timeSched, msg.client_started, timeB);
//printf("timeSched:%ld.%06ld\n", timeSched.tv_sec, timeSched.tv_usec);			
			while(1)
			{
				usleep(1000);
				gettimeofday(&timeNow, NULL);
				if(isBiggerThan(timeNow, timeSched)) break;
			}
		}
	}	//while(1)
	
	printf("---%d:finished!--\n", g_uiOwner);
	
	return 0;
}	//int main(
