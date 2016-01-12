#define ENABLE_LINKEDLIST				0		//1:using linked list for managing clients
#define ENABLE_HANDLETHREAD			1		//1:handling messages with thread
#define ENABLE_MUTEX						1		//1:using mutex for handling resource

#define MAXLINE									(1024)
#define MAX_SOCK 								(1024)
#define RESOURCE_DEFAULT_DELAY	(500*1000)		//MIN:10ms

struct __message {
	int iFd;
	unsigned int owner;
	unsigned int cnt;
	unsigned int cmd;
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

struct __client {
	int iId;
	int iFd;
	struct __client *next;
};


struct __resource_cached {
	int iData;
	int iAga;		/* milisecond */
};

struct __resource {
	char strName[128];
	int iCachedAge;
	int iCachedResource;
	unsigned int iClientNumber;
	struct __client *observer;
};

enum {
	RESOURCE_CMD_GET			= 0x0,
	RESOURCE_CMD_REGISTER,
	RESOURCE_CMD_SET,
	RESOURCE_CMD_MAX
};