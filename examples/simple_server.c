/*
    C socket server example
    http://www.binarytides.com/server-client-example-c-sockets-linux/

	build : gcc -o simple_server simple_server.c -lpthread	
*/
 
#include<stdio.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<time.h>

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


static int cnt = 0;
unsigned char client_message[2000];
int client_sock;
pthread_mutex_t m_lock;

struct __message msg;

#if 0
FILE *file;

void dump_time(struct timeval timeRecv, struct timeval timeStart, struct timeval timeEnd)
{
	struct timeval timeWait, timeProcess, timeTotal;
	file = fopen("/tmp/log.txt", "w+");
	
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
	
	fclose(file);	
}
#endif //#if 0

void *pthread_func(void *arg)
{
	struct timeval timeRecv, timeStart, timeEnd;
	
	gettimeofday(&timeRecv, NULL);
	pthread_mutex_lock(&m_lock);
    		
	//TODO: handle packet
	gettimeofday(&timeStart, NULL);
	usleep(1000*1000);	
    		
	pthread_mutex_unlock(&m_lock);
	gettimeofday(&timeEnd, NULL);
	
#if 1
	printf("[%d]%d-%d\n", msg.owner, msg.cnt, msg.req_dur);
	msg.server_recved.tv_sec = timeRecv.tv_sec;
	msg.server_recved.tv_usec = timeRecv.tv_usec;
	msg.server_started.tv_sec = timeStart.tv_sec;
	msg.server_started.tv_usec = timeStart.tv_usec;
	msg.server_finished.tv_sec = timeEnd.tv_sec;
	msg.server_finished.tv_usec = timeEnd.tv_usec;

	msg.rsp_dur = 1000;	
	//write(client_sock , &resp , sizeof(resp));
	send(client_sock , &msg , sizeof(msg), 0);
	//close(client_sock);
#endif
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
//while(1) {
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
     
#if 0
    //Receive a message from client
    memset(client_message, 0x0, sizeof(client_message)); 
    while( (read_size = recv(client_sock , client_message , 2000 , 0)) > 0 )
#else
    memset(&msg, 0x0, sizeof(msg)); 
    while( (read_size = recv(client_sock , &msg, sizeof(msg) , 0)) > 0 )
#endif
    {
    	//printf("+++:msg=%d\n", msg.req_dur);
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
//}	//while(1)
	close(client_sock);

    return 0;
}
