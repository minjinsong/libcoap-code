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
#include "resource.h"

extern int g_iSocketServer;

int getResourceFromServer(struct __message *msg)
{
	struct __message resp;
	struct timeval timeEnd;
	struct timeval timeStart;
			
	gettimeofday(&timeStart, NULL);
			
	//TODO: send information request from a proxy to a resource server
	//memset(&msg, 0x0, sizeof(struct __message));
	if(send(g_iSocketServer, msg, sizeof(struct __message), 0) < 0)
	{
		printf("proxy : send failed!\n");
	}	//if(send(

	//TODO: get information from a resource server to proxy
	int size;
	memset(&resp, 0x0, sizeof(struct __message));
	if((size = recv(g_iSocketServer, &resp, sizeof(struct __message), 0)) > 0)
	{
		setTimeValue(&(msg->server_recved), resp.server_recved.tv_sec, resp.server_recved.tv_usec);
		setTimeValue(&(msg->server_started), resp.server_started.tv_sec, resp.server_started.tv_usec);
		setTimeValue(&(msg->server_finished), resp.server_finished.tv_sec, resp.server_finished.tv_usec);
	} //if((size
		
	msg->resource = resp.resource;
	msg->uiMaxAge = resp.uiMaxAge;

	//pthread_mutex_unlock(&m_lock);
			
	return 0;
}