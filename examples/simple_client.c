/*
    C ECHO client example using sockets

	build : gcc -o simple_client simple_client.c
*/
#include<stdio.h> //printf
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include<stdlib.h>

/*
struct __request {
	unsigned int owner;
	unsigned int cnt;
	unsigned int req_dur;
};

struct __response {
	unsigned int owner;
	unsigned int cnt;
	unsigned int max_age;
};
*/

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
	int sock;
	 struct sockaddr_in server;
	//char message[1000] ;
	//unsigned char server_reply[2000];
	//struct __request req;
	//struct __response resp;
	struct __message msg = {0x0, };
	struct __message resp = {0x0, };
	struct timeval timeStart;
	struct timeval timeTrans;
	int temp;
     
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8888 );
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    puts("Connected\n");

	memset(&msg, 0x0, sizeof(msg));
	msg.owner = random()%10000;
   
    //keep communicating with server
    while(1)
    {
#if 0
        printf("Enter message : ");
        scanf("%s" , message);
#else
//        printf("Enter duration : ");
//        scanf("%d\n" , &temp);
//	msg.req_dur = temp;
	msg.cnt++;
	msg.req_dur = 100;
#endif        

#if 0
        //Send some data
        if( send(sock , message , sizeof(message) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
         
        //Receive a reply from the server
        if( recv(sock , server_reply , sizeof(server_reply) , 0) < 0)
        {
            puts("recv failed");
            break;
        }
        puts("Server reply :");
        puts(server_reply);
#else

	gettimeofday(&timeStart, NULL);
	msg.proxy_started.tv_sec = timeStart.tv_sec;
	msg.proxy_started.tv_usec = timeStart.tv_usec;

        //Send some data
        if( send(sock , &msg , sizeof(msg) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
         
        //Receive a reply from the server
        if( recv(sock , &resp , sizeof(resp) , 0) < 0)
        {
            puts("recv failed");
            break;
        }
	
	//TODO: dump response
	timeTrans.tv_sec = resp.server_recved.tv_sec - resp.proxy_started.tv_sec;
	timeTrans.tv_usec = resp.server_recved.tv_usec - resp.proxy_started.tv_usec;
	if( timeTrans.tv_usec < 0 ) {timeTrans.tv_sec=timeTrans.tv_sec-1; timeTrans.tv_usec=timeTrans.tv_usec + 1000000;	}	
	
	printf("[%d|%d|%d|%d][%ld.%06ld|%ld.%06ld|%ld.%06ld][%ld.%06ld]%ld.%06ld\n", \
		resp.owner,	\
		resp.cnt,	\
		resp.req_dur,	\
		resp.rsp_dur,	\
		resp.server_recved.tv_sec%1000,	\
		resp.server_recved.tv_usec,	\
		resp.server_started.tv_sec%1000,	\
		resp.server_started.tv_usec,	\
		resp.server_finished.tv_sec%1000,	\
		resp.server_finished.tv_usec,	\
		resp.proxy_started.tv_sec%1000,	\
		resp.proxy_started.tv_usec,	\
		timeTrans.tv_sec%1000,	\
		timeTrans.tv_usec	\
		);
#endif
       

    }
     
    close(sock);
    return 0;
}
