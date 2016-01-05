/*
	build : gcc -o resource_server resource_server.c -lpthread	
	execution : ./resource_server 1024
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

#define MAXLINE		(1024)
#define MAX_SOCK 	(1024)

int getFdMax(int);
void removeClient(int);

int g_piFdMax;
int g_iClientMax = 0;
int g_piSocketClient[MAX_SOCK] = {0, };
char *escapechar = "exit";

pthread_mutex_t m_lock;

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
	int socket;
};

struct __client_head {
	int cnt;
	struct __client *client;
};

//struct __message msg;


int handleMessage(struct __message *arg)
{
	struct timeval timeRecv, timeStart, timeEnd;
	struct __message msg;

	memcpy(&msg, arg, sizeof(struct __message));

	gettimeofday(&timeRecv, NULL);
	pthread_mutex_lock(&m_lock);
    		
	//TODO: handle packet
	gettimeofday(&timeStart, NULL);
	usleep(1000*1000);
    		
	pthread_mutex_unlock(&m_lock);
	gettimeofday(&timeEnd, NULL);
	
	printf("[%d]%d-%d\n", msg.owner, msg.cnt, msg.req_dur);
	msg.server_recved.tv_sec = timeRecv.tv_sec;
	msg.server_recved.tv_usec = timeRecv.tv_usec;
	msg.server_started.tv_sec = timeStart.tv_sec;
	msg.server_started.tv_usec = timeStart.tv_usec;
	msg.server_finished.tv_sec = timeEnd.tv_sec;
	msg.server_finished.tv_usec = timeEnd.tv_usec;

	msg.rsp_dur = 1000;	
	
	int temp = send(msg.iFd, &msg, sizeof(struct __message), 0);

	return 0;
}

void *pthread_func(void *arg)
{
	handleMessage((struct __message *)arg);
	
	return NULL;
}

int main(int argc , char *argv[])
{
		char rline[MAXLINE], my_msg[MAXLINE];
		//char *start = "Connected to server\n";
		int i, j, n;
		int s, client_fd, client;
		fd_set read_fds;
		struct sockaddr_in client_addr, server_addr;
		
		struct __message rcv = { 0x0, };
		struct __message trans = { 0x0, };
		
		pthread_mutex_init(&m_lock, NULL);
		
		if(argc != 2)
		{
			printf("usage : %s [port#]\n", argv[0]);
			exit(0);
		}	//if(argc != 2)
			
		if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		{
			printf("server : socket failed!\n");
			exit(0);
		}	//if((s = socket
		
		int option = 1;
		setsockopt( s, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int) );
			
		bzero((char *)&server_addr, sizeof(struct sockaddr_in));
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		server_addr.sin_port = htons(atoi(argv[1]));
			
		if(bind(s, (struct sockaddr  *)&server_addr, sizeof(struct sockaddr_in)) < 0)
		{
			printf("server : bind failed!\n");
			exit(0);
		}
		
		listen(s, 5);
			
		g_piFdMax = s + 1;

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
					pthread_t ptId = 0;
					rcv.iFd = g_piSocketClient[i];
					pthread_create(&ptId, NULL, pthread_func, (void *)&rcv);
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

