#define ENABLE_LINKEDLIST				0		//1:using linked list for managing clients
#define ENABLE_HANDLETHREAD			1		//1:handling messages with thread
#define ENABLE_MUTEX						1		//1:using mutex for handling resource

#define MAXLINE									(1024)
#define MAX_SOCK 								(1024)
#define RESOURCE_DEFAULT_DELAY	(5)				//ms
#define RESOURCE_DELAY_TRUST		(1.1)					//
#define RESOURCE_DELAY_WATCHER	(1000)				//1ms
#define DELAY_PROXY_RX					(5)
#define DELAY_PROXY_TX					(3)
//#define DELAY_PROXY_PROCESS		()
#define DELAY_SERVER_RX					(12)		//ms
#define DELAY_SERVER_TX					(8)			//ms
#define DELAY_SERVER_PROCESS		(RESOURCE_DEFAULT_DELAY)

enum {
	RESOURCE_CMD_GET			= 0x0,
	RESOURCE_CMD_REGISTER,
	RESOURCE_CMD_SET,
	RESOURCE_CMD_MAX
};

enum {
	CACHE_STATE_INVALID		= 0x0,
	CACHE_STATE_UPDATING,
	CACHE_STATE_VALID,
};

struct __message {
	int iFd;
	unsigned int owner;
	unsigned int cnt;
	unsigned int cmd;
	unsigned int req_dur;
	//unsigned int rsp_dur;
	unsigned int uiMaxAge;
	unsigned int resource;
	//unsigned int age;		//ms
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
	int iId;
	int iFd;
	unsigned int uiReqInterval;
	struct timeval tSched;
//	struct timeval tReqInterval;
	struct __client *next;
	struct __client *nextClient;
};

	struct __cache {
		int iCachedResource;
		unsigned int uiState;
		unsigned int uiMaxAge;
		unsigned int uiCachedAge;
		struct timeval tCachedTime;
		struct __cache *nextCache;
		unsigned int uiClientNumber;
		struct __client *nextClient;
	};
	
	struct __resource2 {
		char strName[128];
		unsigned int uiCacheNumber;
		struct __cache *nextCache;
	};

	struct __resource1 {
		char strName[128];
		int iCachedResource;
		unsigned int uiMaxAge;
		unsigned int uiCachedAge;
		struct timeval tCachedTime;
		//struct timeval tValidTime;
		unsigned int iClientNumber;
		struct __client *next;
	};

/*
int subTime(struct timeval *tRet, struct timeval val1, struct timeval val2)
{
	struct timeval tTemp;
	
	tRet->tv_sec = val1.tv_sec - val2.tv_sec;
	tRet->tv_usec = val1.tv_usec - val2.tv_usec;
	if( tRet->tv_usec < 0 ) {tRet->tv_sec=tRet->tv_sec-1; tRet->tv_usec=tRet->tv_usec + 1000000;	}	
}
*/

int setTimeValue(struct timeval *tRet, int iSecond, int iMicroSecond)
{
	tRet->tv_sec = iSecond;
	tRet->tv_usec = iMicroSecond;
	
	return 0;
}

int addTimeValue(struct timeval *timeR, struct timeval timeA, struct timeval timeB)
{
	if(timeA.tv_usec+timeB.tv_usec >= 1000000)
	{
		timeR->tv_usec = timeA.tv_usec + timeB.tv_usec - 1000000;
		timeR->tv_sec = timeA.tv_sec + timeB.tv_sec + 1;
	}
	else
	{
		timeR->tv_usec = timeA.tv_usec + timeB.tv_usec;
		timeR->tv_sec = timeA.tv_sec + timeB.tv_sec;
	}
	
	return 0;
}

int subTimeValue(struct timeval *tRet, struct timeval timeA, struct timeval timeB)
{
	if(timeA.tv_usec >= timeB.tv_usec)
	{
		tRet->tv_usec = timeA.tv_usec - timeB.tv_usec;
		tRet->tv_sec = timeA.tv_sec - timeB.tv_sec;
	}
	else
	{
		tRet->tv_usec = timeA.tv_usec + 1000000 - timeB.tv_usec;
		tRet->tv_sec = timeA.tv_sec - timeB.tv_sec - 1;
	}
	
	if(timeA.tv_sec < timeB.tv_sec)
	{
		setTimeValue(tRet, 0, 0);
	}
	
	return 0;
}

int isBiggerThan(struct timeval timeA, struct timeval timeB)
{
	int ret = 0;
	
	if(timeA.tv_sec > timeB.tv_sec )
	{
		ret = 1;
	}
	else if(timeA.tv_sec == timeB.tv_sec)
	{
		if(timeA.tv_usec >= timeB.tv_usec)
			ret = 1;
		else
			ret = 0;
	}
	return ret;
}