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


#define MAXLINE			512
#define MAX_SOCK		(1024*1024)

char *escapechar = "exit";
char name[10];

int main(int argc, char *argv[])
{
	char line[MAXLINE], message[MAXLINE+1];
	int n, pid;
	struct sockaddr_in server_addr;
	int maxfdp1;
	int s;
	fd_set read_fds;
	
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
	
	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));
	
	if(connect(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
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
	
	while(1)
	{
		FD_SET(0, &read_fds);
		FD_SET(s, &read_fds);
		
		if(select(maxfdp1, &read_fds, (fd_set *)0, (fd_set *)0, (struct timeval *)0) < 0)
		{
			printf("client : select failed\n");
			exit(0);
		}//if(select
		
		if(FD_ISSET(s, &read_fds))
		{
			int size;
			if((size = recv(s, message, MAXLINE, 0)) > 0)
			{
				message[size] = '\0';
				printf("%s \n", message);
			}	//if((size = recv(
		}	//if(FD_ISSET(
		
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
	}	//while(1)
	
	return 0;
}	//int main(