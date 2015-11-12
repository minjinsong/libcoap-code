/*
    C socket server example
    http://www.binarytides.com/server-client-example-c-sockets-linux/
*/
 
#include<stdio.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
 
static int cnt = 0;
char client_message[2000];
int client_sock;
pthread_mutex_t m_lock;

FILE *file;

void dump_time(struct timeval timeRecv, struct timeval timeStart, struct timeval timeEnd)
{
	struct timeval timeWait, timeProcess, timeTotal;
	
	timeTotal.tv_sec  = timeEnd.tv_sec  - timeRecv.tv_sec;
	timeTotal.tv_usec = timeEnd.tv_usec - timeRecv.tv_usec;
	timeWait.tv_sec  = timeStart.tv_sec  - timeRecv.tv_sec;
	timeWait.tv_usec = timeStart.tv_usec - timeRecv.tv_usec;
	timeProcess.tv_sec  = timeEnd.tv_sec  - timeStart.tv_sec;
	timeProcess.tv_usec = timeEnd.tv_usec - timeStart.tv_usec;
	if( timeTotal.tv_usec < 0 ) {timeTotal.tv_sec=timeTotal.tv_sec-1; timeTotal.tv_usec=timeTotal.tv_usec + 1000000;	}	
	if( timeWait.tv_usec < 0 ) {timeWait.tv_sec=timeWait.tv_sec-1; timeWait.tv_usec=timeWait.tv_usec + 1000000;	}	
	if( timeProcess.tv_usec < 0 ) {timeProcess.tv_sec=timeProcess.tv_sec-1; timeProcess.tv_usec=timeProcess.tv_usec + 1000000;	}	
		
	if( timeWait.tv_sec*1000 + timeWait.tv_usec > 100)
	{
		
		
		printf(";%d; %ld.%06ld; %ld.%06ld; %ld.%06ld; %ld.%06ld; %ld.%06ld; %ld.%06ld\n", 
		//fprintf(file, ";%d ;%ld.%06ld; %ld.%06ld; %ld.%06ld; %ld.%06ld; %ld.%06ld; %ld.%06ld\n", 
		cnt,
		timeRecv.tv_sec, timeRecv.tv_usec,
		timeStart.tv_sec, timeStart.tv_usec,
		timeEnd.tv_sec, timeEnd.tv_usec,
		timeTotal.tv_sec, timeTotal.tv_usec,
		timeWait.tv_sec, timeWait.tv_usec,
		timeProcess.tv_sec, timeProcess.tv_usec
		);
		
		fprintf(file, "minjin!!!\n");
		
	}
		
}

void *pthread_func(void *arg)
{
	struct timeval timeRecv, timeStart, timeEnd;
	
	printf("+++:%d, %s\n", cnt++, client_message);
	
	gettimeofday(&timeRecv, NULL);
	pthread_mutex_lock(&m_lock);
    		
	//TODO: handle packet 
	gettimeofday(&timeStart, NULL);
	usleep(50*1000);	
    		
	pthread_mutex_unlock(&m_lock);
	gettimeofday(&timeEnd, NULL);
	dump_time(timeRecv, timeStart, timeEnd);
		
  write(client_sock , client_message , strlen(client_message));
  printf("---:\n");
	return 0;
}

int main(int argc , char *argv[])
{
    //int socket_desc , client_sock , c , read_size;
    int socket_desc , c , read_size;
    struct sockaddr_in server , client;
    //char client_message[2000];
    pthread_t ptId = 0;
    
    pthread_mutex_init(&m_lock, NULL);
    file = fopen("/tmp/log.txt", "w+");
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
     
    //accept connection from an incoming client
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
    puts("Connection accepted");
     
    //Receive a message from client
    while( (read_size = recv(client_sock , client_message , 2000 , 0)) > 0 )
    {
    	//printf("+++\n");
#if 0    		
        //Send the message back to client
        write(client_sock , client_message , strlen(client_message));
#else
				pthread_create(&ptId, NULL, pthread_func, (void *)NULL);
#endif        
			//printf("---\n");
    }
     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
    
    fclose(file);
     
    return 0;
}