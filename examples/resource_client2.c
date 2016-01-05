/*
	build : gcc -o client_chat client_chat.c
*/
 
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>


#define MAXLINE			512
#define MAX_SOCK		(1024*1024)

char *escapechar = "exit";
char name[10];


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

int initMessage(struct __message *msg)
{
	struct timeval timeStart;
	struct timeval timeTrans;
	
	srand(time(NULL));
	
	memset(msg, 0x0, sizeof(struct __message));
	
	msg->owner = random()%10000;
	msg->cnt++;
	msg->req_dur = 100;
	
	gettimeofday(&timeStart, NULL);
	msg->proxy_started.tv_sec = timeStart.tv_sec;
	msg->proxy_started.tv_usec = timeStart.tv_usec;
	
	return 0;
}

int dumpMessage(struct __message msg)
{
	struct timeval timeTrans;

	//TODO: dump response
	timeTrans.tv_sec = msg.server_recved.tv_sec - msg.proxy_started.tv_sec;
	timeTrans.tv_usec = msg.server_recved.tv_usec - msg.proxy_started.tv_usec;
	if( timeTrans.tv_usec < 0 ) {timeTrans.tv_sec=timeTrans.tv_sec-1; timeTrans.tv_usec=timeTrans.tv_usec + 1000000;	}	
	
	printf("[%d|%d|%d|%d][%ld.%06ld|%ld.%06ld|%ld.%06ld][%ld.%06ld]%ld.%06ld\n", \
		msg.owner,	\
		msg.cnt,	\
		msg.req_dur,	\
		msg.rsp_dur,	\
		msg.server_recved.tv_sec%1000,	\
		msg.server_recved.tv_usec,	\
		msg.server_started.tv_sec%1000,	\
		msg.server_started.tv_usec,	\
		msg.server_finished.tv_sec%1000,	\
		msg.server_finished.tv_usec,	\
		msg.proxy_started.tv_sec%1000,	\
		msg.proxy_started.tv_usec,	\
		timeTrans.tv_sec%1000,	\
		timeTrans.tv_usec	\
		);
		
		return 0;
}


int main(int argc, char *argv[])
{
	char line[MAXLINE], message[MAXLINE+1];
	int n, pid;
	struct sockaddr_in server_addr;
	int maxfdp1;
	int s;
	fd_set read_fds;
	
	struct __message msg = {0x0, };
	struct __message resp = {0x0, };
	
	if(argc != 4)
	{
		printf("usage : %s [server_ip#] [port#] [name]\n", argv[0]);
		exit(0);
	}	//if(arvc
			
	sprintf(name, "[%s]", argv[3]);
	
	if( (s=socket(PF_INET, SOCK_STREAM, 0)) < 0 )
	{
		printf("client : socket failed!\n");
		exit(0);
	}	//if( (s=socket
	
	bzero((char *)&server_addr, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));
	
	if(connect(s, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) < 0)
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
	
	initMessage(&msg);
	
	while(1)
	{
		FD_SET(0, &read_fds);
		FD_SET(s, &read_fds);
#if 1
		if(send(s, &msg, sizeof(struct __message), 0) < 0)
		{
			printf("client : send failed!\n");
		}	//if(send(
#endif
		
		if(select(maxfdp1, &read_fds, (fd_set *)0, (fd_set *)0, (struct timeval *)0) < 0)
		{
			printf("client : select failed\n");
			exit(0);
		}//if(select
		
		if(FD_ISSET(s, &read_fds))
		{
			int size;
#if 1
			if((size = recv(s, &resp, sizeof(struct __message), 0)) > 0)
			{
				dumpMessage(resp);
			}
#else			
			if((size = recv(s, message, MAXLINE, 0)) > 0)
			{
				message[size] = '\0';
				printf("%s \n", message);
			}	//if((size = recv(				
#endif				
		}	//if(FD_ISSET(

#if 1
		;
#else					
		if(FD_ISSET(0, &read_fds)) 
		{
			if(fgets(message, MAXLINE, stdin))
			{				
				sprintf(line, "%s %s", name, message);
				if(send(s, line, strlen(line), 0) < 0)
				{
					printf("client : send failed!\n");
				}	//if(send(
				
				if(strstr(message, escapechar) != NULL)
				{
					printf("Good-bye!\n");
					close(s);
					exit(0);
				}	//if(strstr(
			}	//if(fgets(
		}	//if(FD_ISSET(
#endif		
	}	//while(1)
	
	return 0;
}	//int main(