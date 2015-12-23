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
#include<time.h>

#define MAXLINE		(1024)
#define MAX_SOCK 	(1024)

int getmax(int);
void removeClient(int);

int maxfdp1;
int num_client = 0;
int client_s[MAX_SOCK] = {0, };
char *escapechar = "exit";

pthread_mutex_t m_lock;

struct __message {
	unsigned int owner;
	unsigned int cnt;
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

//struct __message msg;


int handleMessage(struct __message *msg)
{
	
	struct timeval timeRecv, timeStart, timeEnd;
	
	gettimeofday(&timeRecv, NULL);
	pthread_mutex_lock(&m_lock);
    		
	//TODO: handle packet
	gettimeofday(&timeStart, NULL);
	usleep(1000*1000);	
    		
	pthread_mutex_unlock(&m_lock);
	gettimeofday(&timeEnd, NULL);
	
	printf("[%d]%d-%d\n", msg->owner, msg->cnt, msg->req_dur);
	msg->server_recved.tv_sec = timeRecv.tv_sec;
	msg->server_recved.tv_usec = timeRecv.tv_usec;
	msg->server_started.tv_sec = timeStart.tv_sec;
	msg->server_started.tv_usec = timeStart.tv_usec;
	msg->server_finished.tv_sec = timeEnd.tv_sec;
	msg->server_finished.tv_usec = timeEnd.tv_usec;

	msg->rsp_dur = 1000;	
	
	return 0;
}

int main(int argc , char *argv[])
{
		char rline[MAXLINE], my_msg[MAXLINE];
		char *start = "Connected to server\n";
		int i, j, n;
		int s, client_fd, client;
		fd_set read_fds;
		struct sockaddr_in client_addr, server_addr;
		
		struct __message msg;
		
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
		setsockopt( s, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option) );
			
		bzero((char *)&server_addr, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		server_addr.sin_port = htons(atoi(argv[1]));
			
		if(bind(s, (struct sockaddr  *)&server_addr, sizeof(server_addr)) < 0)
			{
				printf("server : bind failed!\n");
				exit(0);
			}
		
		listen(s, 5);
			
		maxfdp1 = s + 1;
		
		while(1)
		{
			FD_ZERO(&read_fds);
			FD_SET(s, &read_fds);
			
			for(i=0; i<num_client; i++)
			{
				FD_SET(client_s[i], &read_fds);
			}	//for(i=0;
			
			maxfdp1 = getmax(s) + 1;
				
			if(select(maxfdp1, &read_fds, (fd_set *)0, (fd_set *)0, (struct timeval *)0) < 0)
			{
				printf("server : select failed\n");
				exit(0);
			}//if(select
				
				
			if(FD_ISSET(s, &read_fds))
			{
				client = sizeof(client_addr);
				client_fd = accept(s, (struct sockaddr *)&client_addr, &client);
				if(client_fd == -1)
				{
					printf("server : accept failed!\n");
					exit(0);
				}
							
				client_s[num_client] = client_fd;
				num_client++;
				send(client_fd, start, strlen(start), 0);
						
				printf("server : user#%d added!\n", num_client);
						
			}//if(FD_ISSET
					
			for(i=0; i<num_client; i++)
			{
				if(FD_ISSET(client_s[i], &read_fds))
				{
#if 1
					if((n = recv(client_s[i], &msg, sizeof(msg), 0)) <= 0)
#else					
					if((n = recv(client_s[i], rline, MAXLINE, 0)) <= 0)
#endif						
					{
						removeClient(i);
						continue;
					}	//if((n = recv
			
#if 1
					handleMessage(&msg);
					send(client_s[i], &msg, sizeof(msg), 0);
#else
					if(strstr(rline, escapechar) != NULL)
					{
						removeClient(i);
						continue;
					}	//if(strstr
					
					rline[n] = '\0';
					for(j=0; j<num_client; j++)
					{
						send(client_s[j], rline, n, 0);
					} //for(j=0
					printf("%s\n", rline);
#endif
				} //if(FD_ISSET(	

			} //for(i=0
		} //while(1)

		return 0;
}


void removeClient(int i)
{
	close(client_s[i]);
	if(i != num_client-1)
	{
		client_s[i] = client_s[num_client-1];
	}
	num_client--;
	printf("server : user exit, num = %d\n", num_client);
}	//void removeClient


int getmax(int k)
{
	int max = k;
	int r;
	
	for(r=0; r<num_client; r++)
	{
		if(client_s[r] > max)
		{
			max = client_s[r];
		}
	}
	return max;
}	//int getmax

