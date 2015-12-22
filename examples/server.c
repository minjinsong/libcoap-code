/* coap -- simple implementation of the Constrained Application Protocol (CoAP)
 *         as defined in draft-ietf-core-coap
 *
 * Copyright (C) 2010--2014 Olaf Bergmann <bergmann@tzi.org>
 *
 * This file is part of the CoAP library libcoap. Please see
 * README for terms of use. 
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>

#include "config.h"
#include "resource.h"
#include "coap.h"

#define COAP_RESOURCE_CHECK_TIME 2

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

/* temporary storage for dynamic resource representations */
static int quit = 0;

/* changeable clock base (see handle_put_time()) */
static time_t my_clock_base = 0;

struct coap_resource_t *time_resource = NULL;

#ifndef WITHOUT_ASYNC
/* This variable is used to mimic long-running tasks that require
 * asynchronous responses. */
static coap_async_state_t *async = NULL;
#endif /* WITHOUT_ASYNC */

/* SIGINT handler: set quit to 1 for graceful termination */
void
handle_sigint(int signum) {
  quit = 1;
}

#define INDEX "This is a test server made with libcoap (see http://libcoap.sf.net)\n" \
   	      "Copyright (C) 2010--2013 Olaf Bergmann <bergmann@tzi.org>\n\n"

#define OEM_DEFINED 			1
#define OEM_THREAD				1
#define OEM_MUTEX					1
#define OEM_CACHE					1
//#define OEM_CACHE_VALIDATION_DELAY		(10*1000)		/* 10ms */
#define OEM_LOCK_DELAY		(10*1000)		/* 10ms */
#define OEM_STRING				"minjinsong@hotmail.com"

#if OEM_THREAD

static int g_cnt = 0;

#include <pthread.h>

pthread_mutex_t m_lock;

void *pthread_func(void *arg)
{
	//coap_context_t *ctx = (coap_context_t *)arg; 
	
	//printf("%s:+++\n", __func__);
	
	coap_read((coap_context_t *)arg);
	
	//pthread_exit(NULL);
	
	return NULL;
}

#endif //OEM_THREAD

void 
hnd_get_index(coap_context_t  *ctx, struct coap_resource_t *resource, 
	      const coap_endpoint_t *local_interface,
	      coap_address_t *peer, coap_pdu_t *request, str *token,
	      coap_pdu_t *response) {
  unsigned char buf[3];

  response->hdr->code = COAP_RESPONSE_CODE(205);

  coap_add_option(response, COAP_OPTION_CONTENT_TYPE,
	  coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);

  coap_add_option(response, COAP_OPTION_MAXAGE,
	  coap_encode_var_bytes(buf, 0x2ffff), buf);
    
  coap_add_data(response, strlen(INDEX), (unsigned char *)INDEX);
}

#if OEM_DEFINED
FILE *file; 
int g_fValid = 0;
struct timeval g_tv;
	
int setCached(struct timeval timestamp)
{
	g_fValid = 1;
	g_tv = timestamp;
	//printf("%s:g_tv.tv_sec=%d, g_tv.tv_usec=%d\n", __func__, g_tv.tv_sec, g_tv.tv_usec);
	
	return 1;
}	

int unsetCached()
{
	g_fValid = 0;
	memset(&g_tv, 0x0, sizeof(struct timeval));
}
	
int isCached(struct timeval now)
{
	int ret = 0;
	struct timeval interval;
	//gettimeofday(&now, NULL);
	interval.tv_sec  = now.tv_sec  - g_tv.tv_sec;
	interval.tv_usec = now.tv_usec - g_tv.tv_usec;
	if( interval.tv_usec < 0 ) {interval.tv_sec=interval.tv_sec-1; interval.tv_usec=interval.tv_usec + 1000000;	}	
		
	if((interval.tv_sec==0) && (interval.tv_usec<OEM_LOCK_DELAY) /*&& g_fCached*/ )
	{
			//printf("1-interval.tv_sec=%d, interval.tv_usec=%d(%d)\n", interval.tv_sec, interval.tv_usec, OEM_LOCK_DELAY);
			ret = 1;
	}
	else
	{
		unsetCached();
		//printf("2-unsetCached\n");
	}

	return ret;
}
	
void 
hnd_get_test(coap_context_t  *ctx, struct coap_resource_t *resource,
	     const coap_endpoint_t *local_interface,
	     coap_address_t *peer, coap_pdu_t *request, str *token,
	     coap_pdu_t *response) 
{
	int fCached = 0;
	struct timeval timeRecv, timeStart, timeEnd;
	struct timeval timeWait, timeProcess, timeTotal;
	gettimeofday(&timeRecv, NULL);

#if OEM_CACHE	
	if(isCached(timeRecv))	
	{
		gettimeofday(&timeStart, NULL);
		fCached = 1;
		
		unsigned char buf[3];
	
	  response->hdr->code = COAP_RESPONSE_CODE(205);
	
	  coap_add_option(response, COAP_OPTION_CONTENT_TYPE,
		  coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);
	
	  coap_add_option(response, COAP_OPTION_MAXAGE,
		  coap_encode_var_bytes(buf, 0x2ffff), buf);
	    
	  coap_add_data(response, strlen(OEM_STRING), (unsigned char *)OEM_STRING);	
	}
	else
#endif		
	{
	#if OEM_MUTEX
			pthread_mutex_lock(&m_lock);
	#endif	//OEM_MUTEX	
	
		fCached = 0;
	
		gettimeofday(&timeStart, NULL);
		setCached(timeStart);
		
		usleep(OEM_LOCK_DELAY);
			
	  unsigned char buf[3];
	
	  response->hdr->code = COAP_RESPONSE_CODE(205);
	
	  coap_add_option(response, COAP_OPTION_CONTENT_TYPE,
		  coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);
	
	  coap_add_option(response, COAP_OPTION_MAXAGE,
		  coap_encode_var_bytes(buf, 0x2ffff), buf);
	    
	  coap_add_data(response, strlen(OEM_STRING), (unsigned char *)OEM_STRING);	
	
	#if OEM_MUTEX
			pthread_mutex_unlock(&m_lock);
	#endif //OEM_MUTEX
	}	//if(isCached())	

	gettimeofday(&timeEnd, NULL);  
	timeTotal.tv_sec  = timeEnd.tv_sec  - timeRecv.tv_sec;
	timeTotal.tv_usec = timeEnd.tv_usec - timeRecv.tv_usec;
	timeWait.tv_sec  = timeStart.tv_sec  - timeRecv.tv_sec;
	timeWait.tv_usec = timeStart.tv_usec - timeRecv.tv_usec;
	timeProcess.tv_sec  = timeEnd.tv_sec  - timeStart.tv_sec;
	timeProcess.tv_usec = timeEnd.tv_usec - timeStart.tv_usec;
	if( timeTotal.tv_usec < 0 ) {timeTotal.tv_sec=timeTotal.tv_sec-1; timeTotal.tv_usec=timeTotal.tv_usec + 1000000;	}	
	if( timeWait.tv_usec < 0 ) {timeWait.tv_sec=timeWait.tv_sec-1; timeWait.tv_usec=timeWait.tv_usec + 1000000;	}	
	if( timeProcess.tv_usec < 0 ) {timeProcess.tv_sec=timeProcess.tv_sec-1; timeProcess.tv_usec=timeProcess.tv_usec + 1000000;	}	
		
	//printf(";%d ;%ld.%06ld; %ld.%06ld; %ld.%06ld; %ld.%06ld; %ld.%06ld; %ld.%06ld\n", 
	fprintf(file, ";%d ;%ld.%06ld; %ld.%06ld; %ld.%06ld; %ld.%06ld; %ld.%06ld; %ld.%06ld; %d\n", 
		++g_cnt,
		timeRecv.tv_sec, timeRecv.tv_usec,
		timeStart.tv_sec, timeStart.tv_usec,
		timeEnd.tv_sec, timeEnd.tv_usec,
		timeTotal.tv_sec, timeTotal.tv_usec,
		timeWait.tv_sec, timeWait.tv_usec,
		timeProcess.tv_sec, timeProcess.tv_usec,
		fCached
		);
}

int 
resolve_address(const str *server, struct sockaddr *dst) {
  
  struct addrinfo *res, *ainfo;
  struct addrinfo hints;
  static char addrstr[256];
  int error, len=-1;
printf("%s:+++\n", __func__);
  memset(addrstr, 0, sizeof(addrstr));
  if (server->length)
    memcpy(addrstr, server->s, server->length);
  else
    memcpy(addrstr, "localhost", 9);
    
  memset ((char *)&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_family = AF_UNSPEC;
  error = getaddrinfo(addrstr, "", &hints, &res);

  if (error != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
    return error;
  }

  for (ainfo = res; ainfo != NULL; ainfo = ainfo->ai_next) {
    switch (ainfo->ai_family) {
    case AF_INET6:
    case AF_INET:
      len = ainfo->ai_addrlen;
      memcpy(dst, ainfo->ai_addr, len);
      goto finish;
    default:
      ;
    }
  }

 finish:
  freeaddrinfo(res);
  return len;
}

#if 0
void 
hnd_get_resource_proxy(coap_context_t  *ctx, struct coap_resource_t *resource,
	     const coap_endpoint_t *local_interface,
	     coap_address_t *peer, coap_pdu_t *request, str *token,
	     coap_pdu_t *response) 
{
	coap_pdu_t  *pdu;
	int result;
	int res;
	struct timeval tv;
	fd_set readfds;
	
	static str server;
	unsigned short port = COAP_DEFAULT_PORT+1;
	coap_address_t dst;
	//str proxy = { 0, NULL };
	
printf("%s:+++\n", __func__);
#define OEM_SERVER_STR		"192.168.0.6"
	server.length = strlen(OEM_SERVER_STR);
	server.s = coap_malloc(server.length + 1);
	memcpy(server.s, OEM_SERVER_STR, server.length+1);

	res = resolve_address(&server, &dst.addr.sa);
	if (res < 0) {
    fprintf(stderr, "failed to resolve address\n");
    exit(-1);
  }	

	dst.size = res;
  dst.addr.sin.sin_port = htons(port);

  //ctx = get_context(node_str[0] == 0 ? "0.0.0.0" : node_str, port_str);
	//coap_register_option(ctx, COAP_OPTION_BLOCK2);
  //coap_register_response_handler(ctx, message_handler);

printf("%s:5\n", __func__);

	coap_send_confirmed(ctx, ctx->endpoint, &dst, pdu);
	
	FD_ZERO(&readfds);
	FD_SET( ctx->sockfd, &readfds );
	
	result = select(ctx->sockfd + 1, &readfds, 0, 0, NULL);
printf("%s:6:result=%d\n", __func__, result);
	if ( result < 0 ) {		/* error */
		perror("select");
	} else if ( result > 0 ) {	/* read from socket */
		if ( FD_ISSET( ctx->sockfd, &readfds ) ) {
			coap_read( ctx );	/* read received data */
			/* coap_dispatch( ctx );	/\* and dispatch PDUs from receivequeue *\/ */
		}
	}
	
	  unsigned char buf[3];
	
	  response->hdr->code = COAP_RESPONSE_CODE(205);
	
	  coap_add_option(response, COAP_OPTION_CONTENT_TYPE,
		  coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);
	
	  coap_add_option(response, COAP_OPTION_MAXAGE,
		  coap_encode_var_bytes(buf, 0x2ffff), buf);
	    
	  coap_add_data(response, strlen(OEM_STRING), (unsigned char *)OEM_STRING);	

	printf("%s:---\n", __func__);	  
}
#else

static int cnt = 0;
int sock;
struct sockaddr_in server;
void 
hnd_get_resource_proxy(coap_context_t  *ctx, struct coap_resource_t *resource,
	     const coap_endpoint_t *local_interface,
	     coap_address_t *peer, coap_pdu_t *request, str *token,
	     coap_pdu_t *response) 
{
	//int sock;
	//struct sockaddr_in server;
	char message[1000] = { '1', '2', '3', '4', '5', '6', '7', '8', 0x0};
	char server_reply[2000] = { 0x0, };
	
	printf("%s:+++:cnt=%d\n", __func__, cnt++);
	
	//pthread_mutex_lock(&m_lock);
	/*
	//Create socket
	//if(sock == 0)
	{
		sock = socket(AF_INET , SOCK_STREAM , 0);
		if (sock == -1)
		{
			printf("Could not create socket");
		}
	}	
	printf("%s:Socket created(%d)\n", __func__, cnt++);
     
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8888 );

	//Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        //return 1;
    }
    
    puts("Connected\n");
*/
    //while(1)
    {
    //Send some data
        if( send(sock , message , strlen(message) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }

        //Receive a reply from the server
        if( recv(sock , server_reply , 2000 , 0) < 0)
        {
            puts("recv failed");
            //break;
        }
         
        printf("Server reply :%s\n", server_reply);
		}
		//close(sock);
		//sock = 0;
		//pthread_mutex_unlock(&m_lock);
     
	  unsigned char buf[3];
	
	  response->hdr->code = COAP_RESPONSE_CODE(205);
	
	  coap_add_option(response, COAP_OPTION_CONTENT_TYPE,
		  coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);
	
	  coap_add_option(response, COAP_OPTION_MAXAGE,
		  coap_encode_var_bytes(buf, 0x2ffff), buf);
	    
	  coap_add_data(response, strlen(OEM_STRING), (unsigned char *)OEM_STRING);	
         
    return;        
}

void socket_init()
{
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1)
	{
		printf("Could not create socket");
	}
	
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1)
	{
		printf("Could not create socket");
	}
	
	//printf("%s:Socket created(%d)\n", __func__, cnt++);
     
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
  
}
#endif

#endif	//#if OEM_DEFINED

void 
hnd_get_time(coap_context_t  *ctx, struct coap_resource_t *resource,
	     const coap_endpoint_t *local_interface,
	     coap_address_t *peer, coap_pdu_t *request, str *token,
	     coap_pdu_t *response) {
  coap_opt_iterator_t opt_iter;
  coap_opt_t *option;
  unsigned char buf[40];
  size_t len;
  time_t now;
  coap_tick_t t;

#if OEM_DEFINED
	struct timeval timeStart, timeEnd, timeDiff;
	gettimeofday(&timeStart, NULL);
#endif //OEM_DEFINED
#if OEM_MUTEX
	pthread_mutex_lock(&m_lock);
#endif	//OEM_MUTEX	
#if OEM_DEFINED
	usleep(OEM_LOCK_DELAY);
#endif //OEM_DEFINED
  /* FIXME: return time, e.g. in human-readable by default and ticks
   * when query ?ticks is given. */

  /* if my_clock_base was deleted, we pretend to have no such resource */
  response->hdr->code = 
    my_clock_base ? COAP_RESPONSE_CODE(205) : COAP_RESPONSE_CODE(404);

  if (coap_find_observer(resource, peer, token)) {
    /* FIXME: need to check for resource->dirty? */
    coap_add_option(response, COAP_OPTION_OBSERVE, 
		    coap_encode_var_bytes(buf, ctx->observe), buf);
  }

  if (my_clock_base)
    coap_add_option(response, COAP_OPTION_CONTENT_FORMAT,
		    coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);

  coap_add_option(response, COAP_OPTION_MAXAGE,
	  coap_encode_var_bytes(buf, 0x01), buf);

  if (my_clock_base) {

    /* calculate current time */
    coap_ticks(&t);
    now = my_clock_base + (t / COAP_TICKS_PER_SECOND);
    
    if (request != NULL
	&& (option = coap_check_option(request, COAP_OPTION_URI_QUERY, &opt_iter))
	&& memcmp(COAP_OPT_VALUE(option), "ticks",
		  min(5, COAP_OPT_LENGTH(option))) == 0) {
      /* output ticks */
      len = snprintf((char *)buf, 
	   min(sizeof(buf), response->max_size - response->length),
		     "%u", (unsigned int)now);
      coap_add_data(response, len, buf);

    } else {			/* output human-readable time */
      struct tm *tmp;
      tmp = gmtime(&now);
      len = strftime((char *)buf, 
		     min(sizeof(buf), response->max_size - response->length),
		     "%b %d %H:%M:%S", tmp);
      coap_add_data(response, len, buf);
    }
  }
#if OEM_MUTEX
	pthread_mutex_unlock(&m_lock);
#endif //OEM_MUTEX
#if OEM_DEFINED
	gettimeofday(&timeEnd, NULL);  
	timeDiff.tv_sec  = timeEnd.tv_sec  - timeStart.tv_sec;
	timeDiff.tv_usec = timeEnd.tv_usec - timeStart.tv_usec;
	if( timeDiff.tv_usec < 0 ) 
	{
	    timeDiff.tv_sec  = timeDiff.tv_sec  - 1;
	    timeDiff.tv_usec = timeDiff.tv_usec + 1000000;
	}	
	printf("\n###S(%ld.%06ld)-E(%ld.%06ld)=D(%ld.%06ld)\n", 
		timeStart.tv_sec, timeStart.tv_usec,
		timeEnd.tv_sec, timeEnd.tv_usec,
		timeDiff.tv_sec, timeDiff.tv_usec);
#endif //OEM_MUTEX
  
}

void 
hnd_put_time(coap_context_t  *ctx, struct coap_resource_t *resource,
	     const coap_endpoint_t *local_interface,
	     coap_address_t *peer, coap_pdu_t *request, str *token,
	     coap_pdu_t *response) {
  coap_tick_t t;
  size_t size;
  unsigned char *data;

  /* FIXME: re-set my_clock_base to clock_offset if my_clock_base == 0
   * and request is empty. When not empty, set to value in request payload
   * (insist on query ?ticks). Return Created or Ok.
   */

  /* if my_clock_base was deleted, we pretend to have no such resource */
  response->hdr->code = 
    my_clock_base ? COAP_RESPONSE_CODE(204) : COAP_RESPONSE_CODE(201);

  resource->dirty = 1;

  coap_get_data(request, &size, &data);
  
  if (size == 0)		/* re-init */
    my_clock_base = clock_offset;
  else {
    my_clock_base = 0;
    coap_ticks(&t);
    while(size--) 
      my_clock_base = my_clock_base * 10 + *data++;
    my_clock_base -= t / COAP_TICKS_PER_SECOND;
  }
}

void 
hnd_delete_time(coap_context_t  *ctx, struct coap_resource_t *resource,
		const coap_endpoint_t *local_interface,
		coap_address_t *peer, coap_pdu_t *request, str *token,
		coap_pdu_t *response) {
  my_clock_base = 0;		/* mark clock as "deleted" */
  
  /* type = request->hdr->type == COAP_MESSAGE_CON  */
  /*   ? COAP_MESSAGE_ACK : COAP_MESSAGE_NON; */
}

#ifndef WITHOUT_ASYNC
void
hnd_get_async(coap_context_t  *ctx, struct coap_resource_t *resource,
	      const coap_endpoint_t *local_interface,
	      coap_address_t *peer, coap_pdu_t *request, str *token,
	      coap_pdu_t *response) {
  coap_opt_iterator_t opt_iter;
  coap_opt_t *option;
  unsigned long delay = 5;
  size_t size;

  if (async) {
    if (async->id != request->hdr->id) {
      coap_opt_filter_t f;
      coap_option_filter_clear(f);
      response->hdr->code = COAP_RESPONSE_CODE(503);
    }
    return;
  }

  option = coap_check_option(request, COAP_OPTION_URI_QUERY, &opt_iter);
  if (option) {
    unsigned char *p = COAP_OPT_VALUE(option);

    delay = 0;
    for (size = COAP_OPT_LENGTH(option); size; --size, ++p)
      delay = delay * 10 + (*p - '0');
  }

  async = coap_register_async(ctx, peer, request, 
			      COAP_ASYNC_SEPARATE | COAP_ASYNC_CONFIRM,
			      (void *)(COAP_TICKS_PER_SECOND * delay));
}

void 
check_async(coap_context_t  *ctx, const coap_endpoint_t *local_if,
	    coap_tick_t now) {
  coap_pdu_t *response;
  coap_async_state_t *tmp;

  size_t size = sizeof(coap_hdr_t) + 13;

  if (!async || now < async->created + (unsigned long)async->appdata) 
    return;

  response = coap_pdu_init(async->flags & COAP_ASYNC_CONFIRM 
			   ? COAP_MESSAGE_CON
			   : COAP_MESSAGE_NON,
			   COAP_RESPONSE_CODE(205), 0, size);
  if (!response) {
    debug("check_async: insufficient memory, we'll try later\n");
    async->appdata = 
      (void *)((unsigned long)async->appdata + 15 * COAP_TICKS_PER_SECOND);
    return;
  }
  
  response->hdr->id = coap_new_message_id(ctx);

  if (async->tokenlen)
    coap_add_token(response, async->tokenlen, async->token);

  coap_add_data(response, 4, (unsigned char *)"done");

  if (coap_send(ctx, local_if, &async->peer, response) == COAP_INVALID_TID) {
    debug("check_async: cannot send response for message %d\n", 
	  response->hdr->id);
  }
  coap_delete_pdu(response);
  coap_remove_async(ctx, async->id, &tmp);
  coap_free_async(async);
  async = NULL;
}
#endif /* WITHOUT_ASYNC */

void
init_resources(coap_context_t *ctx) {
  coap_resource_t *r;

  r = coap_resource_init(NULL, 0, 0);
  coap_register_handler(r, COAP_REQUEST_GET, hnd_get_index);

  coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
  coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"General Info\"", 14, 0);
  coap_add_resource(ctx, r);

  /* store clock base to use in /time */
  my_clock_base = clock_offset;

  r = coap_resource_init((unsigned char *)"time", 4, 0);
  coap_register_handler(r, COAP_REQUEST_GET, hnd_get_time);
  coap_register_handler(r, COAP_REQUEST_PUT, hnd_put_time);
  coap_register_handler(r, COAP_REQUEST_DELETE, hnd_delete_time);

  coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
  coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"Internal Clock\"", 16, 0);
  coap_add_attr(r, (unsigned char *)"rt", 2, (unsigned char *)"\"Ticks\"", 7, 0);
  r->observable = 1;
  coap_add_attr(r, (unsigned char *)"if", 2, (unsigned char *)"\"clock\"", 7, 0);

  coap_add_resource(ctx, r);
  time_resource = r;

#ifndef WITHOUT_ASYNC
  r = coap_resource_init((unsigned char *)"async", 5, 0);
  coap_register_handler(r, COAP_REQUEST_GET, hnd_get_async);

  coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
  coap_add_resource(ctx, r);
#endif /* WITHOUT_ASYNC */
#if OEM_DEFINED
  r = coap_resource_init((unsigned char *)"test", 4, 0);
  coap_register_handler(r, COAP_REQUEST_GET, hnd_get_test);
  coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
  coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"minin7.song@samsung.com\"", 16, 0);  
  coap_add_resource(ctx, r);
  
  r = coap_resource_init((unsigned char *)"resource_proxy", 14, 0);
  coap_register_handler(r, COAP_REQUEST_GET, hnd_get_resource_proxy);
  coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
  coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"Get Resource through Proxy\"", 16, 0);  
  coap_add_resource(ctx, r);
#endif //#if OEM_DEFINED
}

void
usage( const char *program, const char *version) {
  const char *p;

  p = strrchr( program, '/' );
  if ( p )
    program = ++p;

  fprintf( stderr, "%s v%s -- a small CoAP implementation\n"
	   "(c) 2010,2011 Olaf Bergmann <bergmann@tzi.org>\n\n"
	   "usage: %s [-A address] [-p port]\n\n"
	   "\t-A address\tinterface address to bind to\n"
	   "\t-p port\t\tlisten on specified port\n"
	   "\t-v num\t\tverbosity level (default: 3)\n",
	   program, version, program );
}

coap_context_t *
get_context(const char *node, const char *port) {
  coap_context_t *ctx = NULL;  
  int s;
  struct addrinfo hints;
  struct addrinfo *result, *rp;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_DGRAM; /* Coap uses UDP */
  hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST;
  
  s = getaddrinfo(node, port, &hints, &result);
  if ( s != 0 ) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return NULL;
  } 

  /* iterate through results until success */
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    coap_address_t addr;

    if (rp->ai_addrlen <= sizeof(addr.addr)) {
      coap_address_init(&addr);
      addr.size = rp->ai_addrlen;
      memcpy(&addr.addr, rp->ai_addr, rp->ai_addrlen);

      ctx = coap_new_context(&addr);
      if (ctx) {
	/* TODO: output address:port for successful binding */
	goto finish;
      }
    }
  }
  
  fprintf(stderr, "no context available for interface '%s'\n", node);

 finish:
  freeaddrinfo(result);
  return ctx;
}

int
main(int argc, char **argv) {
  coap_context_t  *ctx;
  fd_set readfds;
  struct timeval tv, *timeout;
  int result;
  coap_tick_t now;
  coap_queue_t *nextpdu;
  char addr_str[NI_MAXHOST] = "::";
  char port_str[NI_MAXSERV] = "5683";
  int opt;
  coap_log_t log_level = LOG_WARNING;
  
#if OEM_DEFINED
	socket_init();
	
	file = fopen("/tmp/log.txt", "w+");
	
	pthread_t ptId = 0;
	if(pthread_mutex_init(&m_lock, NULL) != 0) 
	{
		printf("%s:pthread_mutex_init error!\n", __func__);
	}
#endif  

  while ((opt = getopt(argc, argv, "A:p:v:")) != -1) {
    switch (opt) {
    case 'A' :
      strncpy(addr_str, optarg, NI_MAXHOST-1);
      addr_str[NI_MAXHOST - 1] = '\0';
      break;
    case 'p' :
      strncpy(port_str, optarg, NI_MAXSERV-1);
      port_str[NI_MAXSERV - 1] = '\0';
      break;
    case 'v' :
      log_level = strtol(optarg, NULL, 10);
      break;
    default:
      usage( argv[0], PACKAGE_VERSION );
      exit( 1 );
    }
  }

  coap_set_log_level(log_level);

  ctx = get_context(addr_str, port_str);
  if (!ctx)
    return -1;

  init_resources(ctx);

  signal(SIGINT, handle_sigint);

  while ( !quit ) {
    FD_ZERO(&readfds);
    FD_SET( ctx->sockfd, &readfds );

    nextpdu = coap_peek_next( ctx );

    coap_ticks(&now);
    while (nextpdu && nextpdu->t <= now - ctx->sendqueue_basetime) {
      coap_retransmit( ctx, coap_pop_next( ctx ) );
      nextpdu = coap_peek_next( ctx );
    }

    if ( nextpdu && nextpdu->t <= COAP_RESOURCE_CHECK_TIME ) {
      /* set timeout if there is a pdu to send before our automatic timeout occurs */
      tv.tv_usec = ((nextpdu->t) % COAP_TICKS_PER_SECOND) * 1000000 / COAP_TICKS_PER_SECOND;
      tv.tv_sec = (nextpdu->t) / COAP_TICKS_PER_SECOND;
      timeout = &tv;
    } else {
      tv.tv_usec = 0;
      tv.tv_sec = COAP_RESOURCE_CHECK_TIME;
      timeout = &tv;
    }
//printf("%s:select timeout=%ld.%06ld\n", __func__,  timeout->tv_sec, timeout->tv_usec);	
    result = select( FD_SETSIZE, &readfds, 0, 0, timeout );

    if ( result < 0 ) {		/* error */
      if (errno != EINTR)
	perror("select");
    } else if ( result > 0 ) {	/* read from socket */
      if ( FD_ISSET( ctx->sockfd, &readfds ) ) {
#if OEM_THREAD
	pthread_create(&ptId, NULL, pthread_func, (void *)ctx);
	usleep(1);
#else      	
	coap_read( ctx );	/* read received data */
#endif	//OEM_THREAD
	/* coap_dispatch( ctx );	/\* and dispatch PDUs from receivequeue *\/ */
      }
    } else {			/* timeout */
      if (time_resource) {
	time_resource->dirty = 1;
      }
    }

#ifndef WITHOUT_ASYNC
    /* check if we have to send asynchronous responses */
    check_async(ctx, ctx->endpoint, now);
#endif /* WITHOUT_ASYNC */

#ifndef WITHOUT_OBSERVE
    /* check if we have to send observe notifications */
    coap_check_notify(ctx);
#endif /* WITHOUT_OBSERVE */
  }
  
#if OEM_DEFINED
	fclose(file);
	close(sock);
#endif	//#if OEM_DEFINED
  coap_free_context( ctx );

  return 0;
}
