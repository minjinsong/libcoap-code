/*
	build : gcc -o client_chat client_chat.c
	execution : ./resource_client2 127.0.0.1 2048 1 200
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

#define ENABLE_REPEATE				1		//1:send message to server repeatly

int initMessage(struct __message *msg)
{
	struct timeval timeStart;
	//struct timeval timeTrans;
	
	//memset(msg, 0x0, sizeof(struct __message));
	//srand(time(NULL));
	//msg->owner = random()%10000;
	
	msg->cnt++;
	msg->req_dur = 100;

	gettimeofday(&timeStart, NULL);
	msg->client_started.tv_sec = timeStart.tv_sec;
	msg->client_started.tv_usec = timeStart.tv_usec;

	return 0;
}

int subTime(struct timeval *tRet, struct timeval val1, struct timeval val2)
{
	struct timeval tTemp;
	
	tRet->tv_sec = val1.tv_sec - val2.tv_sec;
	tRet->tv_usec = val1.tv_usec - val2.tv_usec;
	if( tRet->tv_usec < 0 ) {tRet->tv_sec=tRet->tv_sec-1; tRet->tv_usec=tRet->tv_usec + 1000000;	}	
}

int dumpMessage(struct __message msg)
{
	struct timeval timeTrans;
	struct timeval tSend1;
	struct timeval tSend2;
	struct timeval tServer;
	struct timeval tRecv2;
	struct timeval tRecv1;

	//TODO: dump response
	/*
	timeTrans.tv_sec = msg.client_finished.tv_sec - msg.client_started.tv_sec;
	timeTrans.tv_usec = msg.client_finished.tv_usec - msg.client_started.tv_usec;
	if( timeTrans.tv_usec < 0 ) {timeTrans.tv_sec=timeTrans.tv_sec-1; timeTrans.tv_usec=timeTrans.tv_usec + 1000000;	}	
	*/
	subTime(&timeTrans, msg.client_finished, msg.client_started);
	subTime(&tSend1, msg.proxy_started, msg.client_started);
	if( (msg.server_started.tv_sec>0) && (msg.server_finished.tv_sec>0) )
	{
		subTime(&tSend2, msg.server_started, msg.proxy_started);
		subTime(&tServer, msg.server_finished, msg.server_started);
		subTime(&tRecv2, msg.proxy_finished, msg.server_finished);
	}
	else
	{
		tSend2.tv_sec = 0;
		tSend2.tv_usec = 0;
		tServer.tv_sec = 0;
		tServer.tv_usec = 0;
		tRecv2.tv_sec = 0;
		tRecv2.tv_usec = 0;
	}
	
	subTime(&tRecv1, msg.client_finished, msg.proxy_finished);
	/*
	printf("[%d|%d]resp=%ld.%06ldus,age=%d(%ld.%03ld|%ld.%03ld)(%ld.%03ld|%ld.%03ld)(%ld.%03ld|%ld.%03ld)\n", \
		msg.owner,	\
		msg.cnt,	\
		timeTrans.tv_sec%1000,	\
		timeTrans.tv_usec,	\
		msg.age,	\
		msg.server_started.tv_sec%1000,	\
		msg.server_started.tv_usec,	\
		msg.server_finished.tv_sec%1000,	\
		msg.server_finished.tv_usec,	\
		msg.proxy_started.tv_sec%1000,	\
		msg.proxy_started.tv_usec,	\
		msg.proxy_finished.tv_sec%1000,	\
		msg.proxy_finished.tv_usec,	\
		msg.client_started.tv_sec%1000,	\
		msg.client_started.tv_usec,	\
		msg.client_finished.tv_sec%1000,	\
		msg.client_finished.tv_usec
		);
	*/
	printf("[%d|%d]age=%d, resp=%ld.%06ldms(%ld.%06ld)(%ld.%06ld)(%ld.%06ld)(%ld.%06ld)(%ld.%06ld)\n", 
		msg.owner,	\
		msg.cnt,	\
		msg.age,	\
		timeTrans.tv_sec%1000,	\
		timeTrans.tv_usec,	\
		tSend1.tv_sec%1000,	\
		tSend1.tv_usec,	\
		tSend2.tv_sec%1000,	\
		tSend2.tv_usec,	\
		tServer.tv_sec%1000,	\
		tServer.tv_usec,	\
		tRecv2.tv_sec%1000,	\
		tRecv2.tv_usec,	\
		tRecv1.tv_sec%1000,	\
		tRecv1.tv_usec
	);
	
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
	else if(argc == 5)
	{
		g_monMode = atoi(argv[3]);
		g_monInterval = atoi(argv[4]);
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
	srand(time(NULL));
	msg.owner = random()%10000;
	
//#if !ENABLE_REPEATE
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
//#endif	//#if 0

	while(1)
	{
		FD_SET(0, &read_fds);
		FD_SET(s, &read_fds);

//#if ENABLE_REPEATE	
		if(g_monMode == RESOURCE_CMD_GET)
		{
			initMessage(&msg);
			msg.cmd = RESOURCE_CMD_GET;
			
			if(send(s, &msg, sizeof(struct __message), 0) < 0)
			{
				printf("client : send failed!\n");
			}	//if(send(
		}
//#endif			
		
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
		
		if(g_monMode == RESOURCE_CMD_GET)
		{
			//usleep(g_monInterval*1000);
			usleep(g_monInterval*1000);
		}
	}	//while(1)
	
	return 0;
}	//int main(