/*
	build : gcc -o server_chat server_chat.c -lpthread	
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
#define MAX_SOCK 	(1024*1024)

char *escapechar = "exit";
int getmax(int);
void removeClient(int);
int maxfdp1;
int num_chat = 0;
int client_s[MAX_SOCK] = {0, };


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



int main(int argc , char *argv[])
{
		char rline[MAXLINE], my_msg[MAXLINE];
		char *start = "Connected to chat_server\n";
		int i, j, n;
		int s, client_fd, client;
		
		fd_set read_fds;
		struct sockaddr_in client_addr, server_addr;
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
			
			for(i=0; i<num_chat; i++)
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
							
				client_s[num_chat] = client_fd;
				num_chat++;
				send(client_fd, start, strlen(start), 0);
						
				printf("server : user#%d added!\n", num_chat);
						
			}//if(FD_ISSET
					
			for(i=0; i<num_chat; i++)
			{
				if(FD_ISSET(client_s[i], &read_fds))
				{
					if((n = recv(client_s[i], rline, MAXLINE, 0)) <= 0)
					{
						removeClient(i);
						continue;
					}	//if((n = recv
						
					if(strstr(rline, escapechar) != NULL)
					{
						removeClient(i);
						continue;
					}	//if(strstr
									
					rline[n] = '\0';
					for(j=0; j<num_chat; j++)
					{
						send(client_s[j], rline, n, 0);
					} //for(j=0
					printf("%s\n", rline);
				} //if(FD_ISSET(	

			} //for(i=0
		} //while(1)

		return 0;
}


void removeClient(int i)
{
	close(client_s[i]);
	if(i != num_chat-1)
	{
		client_s[i] = client_s[num_chat-1];
	}
	num_chat--;
	printf("server : user exit, num = %d\n", num_chat);
}	//void removeClient


int getmax(int k)
{
	int max = k;
	int r;
	
	for(r=0; r<num_chat; r++)
	{
		if(client_s[r] > max)
		{
			max = client_s[r];
		}
	}
	return max;
}	//int getmax

