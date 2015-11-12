/*
    C socket server example, handles multiple clients using threads
*/
 
#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread

pthread_mutex_t m_lock;
static int cnt = 0;
 
//the thread function
void *connection_handler(void *);

void dump_time(struct timeval timeRecv, struct timeval timeStart, struct timeval timeEnd)
{
	struct timeval timeWait, timeProcess, timeTotal;
	//FILE *file = fopen("/tmp/log.txt", "w+");
	
	timeTotal.tv_sec  = timeEnd.tv_sec  - timeRecv.tv_sec;
	timeTotal.tv_usec = timeEnd.tv_usec - timeRecv.tv_usec;
	timeWait.tv_sec  = timeStart.tv_sec  - timeRecv.tv_sec;
	timeWait.tv_usec = timeStart.tv_usec - timeRecv.tv_usec;
	timeProcess.tv_sec  = timeEnd.tv_sec  - timeStart.tv_sec;
	timeProcess.tv_usec = timeEnd.tv_usec - timeStart.tv_usec;
	if( timeTotal.tv_usec < 0 ) {timeTotal.tv_sec=timeTotal.tv_sec-1; timeTotal.tv_usec=timeTotal.tv_usec + 1000000;	}	
	if( timeWait.tv_usec < 0 ) {timeWait.tv_sec=timeWait.tv_sec-1; timeWait.tv_usec=timeWait.tv_usec + 1000000;	}	
	if( timeProcess.tv_usec < 0 ) {timeProcess.tv_sec=timeProcess.tv_sec-1; timeProcess.tv_usec=timeProcess.tv_usec + 1000000;	}	
		
	printf(";%ld.%06ld; %ld.%06ld; %ld.%06ld; %ld.%06ld; %ld.%06ld; %ld.%06ld\n", 
	//fprintf(file, ";%d ;%ld.%06ld; %ld.%06ld; %ld.%06ld; %ld.%06ld; %ld.%06ld; %ld.%06ld; %d\n", 
		//++g_cnt,
		timeRecv.tv_sec, timeRecv.tv_usec,
		timeStart.tv_sec, timeStart.tv_usec,
		timeEnd.tv_sec, timeEnd.tv_usec,
		timeTotal.tv_sec, timeTotal.tv_usec,
		timeWait.tv_sec, timeWait.tv_usec,
		timeProcess.tv_sec, timeProcess.tv_usec
		//fCached
		);
		
		//fclose(file);
}
 
int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
    
		if(pthread_mutex_init(&m_lock, NULL) != 0) 
		{
			printf("%s:pthread_mutex_init error!\n", __func__);
		}
     
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
     
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        //puts("Connection accepted");
         
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;
         
        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
         
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL);
        //puts("Handler assigned");
    }
     
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
    
    close(socket_desc);
    //close(client_sock);
     
    return 0;
}
 
/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , client_message[2000];
    
		struct timeval timeRecv, timeStart, timeEnd;
    
    printf("%s:cnt=%d\n", __func__, cnt++);
    /* 
    //Send some messages to the client
    message = "Greetings! I am your connection handler\n";
    write(sock , message , strlen(message));
     
    message = "Now type something and i shall repeat what you type \n";
    write(sock , message , strlen(message));
    */
//while(1) {     
    //Receive a message from client
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {
    		//puts(client_message);
    		/*
    		gettimeofday(&timeRecv, NULL);
    		pthread_mutex_lock(&m_lock);
    		
    		//TODO: handle packet 
    		gettimeofday(&timeStart, NULL);
    		usleep(10*1000);
    		
    		pthread_mutex_unlock(&m_lock);
    		gettimeofday(&timeEnd, NULL);
				dump_time(timeRecv, timeStart, timeEnd);
				*/
    		    		
        //Send the message back to client
        write(sock , client_message , strlen(client_message));
    }
     
    if(read_size == 0)
    {
        //puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
}         
    //Free the socket pointer
    free(socket_desc);
  	
     
    return 0;
}