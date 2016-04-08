/*
	build : gcc -o opt.c opt
	execution : ./opt {algorithm} {allowed_%} {new_client_interval} {new_client_starttime} {old_client_count} {old_client1_interval} {old_client1_starttime} ... 
	eg. ./a.out 1 0 800 500 2 1000 500 1200 800
	new client만 time shift할 수 있다.
*/
 
#include <stdio.h>
#include <string.h>

#define DUMP					0
#define TIME_MAX			2000
#define INTERVAL_MIN	100
#define SLOT_MAX			(TIME_MAX)/(INTERVAL_MIN)
#define CLIENT_MAX		20
#define QUANTUM				50

int arr[CLIENT_MAX+1][SLOT_MAX+1] = {0,};
int bak[CLIENT_MAX+1][SLOT_MAX+1] = {0,};
int queue[(CLIENT_MAX+1)*(SLOT_MAX+1)] = {0,};
int temp[(CLIENT_MAX+1)*(SLOT_MAX+1)] = {0,};
static int giTimeSlice = 0;

void process();
void optimalresult(int index);
static int giMin = 0x7FFFFFFF;

void addQueue(int iTime)
{
	int iAlready = 0;
	int i;
	for(i=1; i<=queue[0]; i++)
	{
		if(queue[i] == iTime)
			iAlready = 1;
	}
	
	if(iAlready==0)
	{
		queue[0]++;
		queue[queue[0]] = iTime;
	}
}

void dump()
{
#if 1
	int i, j;
	for(i=1; i<=CLIENT_MAX+1; i++)
	{
		if(arr[i][0] == 0)
			break;
		
		printf("[%d] ", i);
		for(j=0; j<=SLOT_MAX+1; j++)
		{
			if(arr[i][j] > 0)
			{
				printf("%d, ", arr[i][j]);
			}
			else
			{
				break;
			}
		}
		printf("\n");
	}
#else
	int i;
	for(i=1; i<=queue[0]; i++)
	{
		printf("%d ", queue[i]);
	}
	printf("\n");
#endif	
}

void addTempQueue(int iTime)
{
	int iAlready = 0;
	int i;
	for(i=1; i<=temp[0]; i++)
	{
		if(temp[i] == iTime)
			iAlready = 1;
	}
	
	if((iAlready==0) && (iTime<=TIME_MAX) )
	{
		temp[0]++;
		temp[temp[0]] = iTime;
	}
}

void dumpTempQueue()
{
#if DUMP	
	int i;
	for(i=1; i<=temp[0]; i++)
	{
		printf("%d ", temp[i]);
	}
	printf(" --> message count=%d\n", temp[0]);
#endif	
}

int getBaseTime(int now)
{
	int i;
	int iMin = 0x7FFFFFFF;
	
	//printf("%s: ", __func__);
	for(i=1; i<=CLIENT_MAX; i++)
	{
		if(arr[i][0] == 0)
			break;
		//printf("%d ", arr[i][0]);
		
		//if(arr[i][1] > 0)
		{
			if(iMin > arr[i][1])
			{
				iMin = arr[i][1];
			}
		}
	}
	//printf("\n");
	return iMin;
}

void updateSched(int now)
{
	int i;
	for(i=1; i<=CLIENT_MAX; i++)
	{
		if(arr[i][0] == 0)
			break;
		
		if(arr[i][1] == now)
		{
			//if(TIME_MAX >= (arr[i][1] + arr[i][0]) )
				arr[i][1] = arr[i][1] + arr[i][0];
		}
		//printf("arr[%d][0]=%d, arr[%d][1]=%d\n", i, arr[i][0], i, arr[i][1]);
	}
}

int getGCD(int a, int b)
{
	if(b==0) 
		return a;
	else
		return getGCD(b, a%b);
}

int getLCM(int a, int b)
{
	return (a*b)/getGCD(a, b);
}


int iAllowedRange = 0;
int main(int argc , char *argv[])
{
	int iNewInterval = 0;
	int iNewIntervalStartTime = 0;
	
	int iAlgorithm = 0;
	//int iAllowedRange = 0;
	int iMessageCount = 0;
	int iClientCount = 0;
	
	if(argc < 5)
	{
		printf("usage : %s {time_slice} {allowed_percent} {new_client_interval} {new_client_starttime} {old_client_count} {old_client1_interval} {old_client1_starttime} ... \n", argv[0]);
		printf("eg. ./a.out 50 20 800 500 2 1000 500 1200 800\n");
		//exit(0);
	}	//if(argc
	else
	{
		//iAlgorithm = atoi(argv[1]);
		giTimeSlice = atoi(argv[1]);
		if(giTimeSlice == 0) giTimeSlice = QUANTUM;
		iAllowedRange = atoi(argv[2]);
		iNewInterval = atoi(argv[3]);
		iNewIntervalStartTime = atoi(argv[4]);
		iClientCount = atoi(argv[5]);
		
		//arr[0][0] = iAlgorithm;
		
		int i;
		for(i=0; i<(iClientCount); i++)
		{
			arr[i+1][0] = atoi(argv[i*2+6]);
			arr[i+1][1] = atoi(argv[i*2+7]);
			//printf("arr[%d][0]=%d, arr[%d][1]=%d\n", i, arr[i][0], i, arr[i][1]);
		}
		
		arr[0][0] = iNewInterval;
		arr[0][1] = iNewIntervalStartTime;
	}
	//printf("iAllowedRange=%d, iNewInterval=%d, iNewIntervalStartTime=%d, iClientCount=%d\n", iAllowedRange, iNewInterval, iNewIntervalStartTime, iClientCount);
	//dump();
	
#if 1
{
	//9.client optimization
	int optMsgCnt = 0x7FFFFFFF;
	int optStartTime = 0;
	int abc = 0;
	int i;
	
	//printf("TIME_MAX+iNewIntervalStartTime=%d\n", iNewIntervalStartTime+TIME_MAX);
	int iDuration = getBaseTime(0);
	
	arr[iClientCount+1][0] = arr[0][0];
	arr[iClientCount+1][1] = 0;
	//dump();
		
	for(i=1; i<=(arr[0][0]/giTimeSlice); i++)
	//for(i=0; i<=(iDuration/giTimeSlice); i++)
	{
		//memset(temp, 0x0, sizeof(int)*((CLIENT_MAX+1)*(SLOT_MAX+1)));

		abc = abc + giTimeSlice;
		
		arr[iClientCount+1][1] = abc;
		
		process();
		
		//dumpTempQueue();
		
		if(optMsgCnt >= temp[0])
		{
			optMsgCnt = temp[0];
			optStartTime = giTimeSlice*i;
		}
		
		//iNewIntervalStartTime = iNewIntervalStartTime+giTimeSlice;
		//abc = abc + giTimeSlice;
	}
	printf("<<optimal message count=%d, optStartTime=%d>>\n", optMsgCnt, optStartTime);
}
#endif

	return 0;
}

void process()
{
	int iBaseTime = 0;
	int iNow = 0;
	int i, j;
	
	memset(temp, 0x0, sizeof(int)*((CLIENT_MAX+1)*(SLOT_MAX+1)));
	memcpy(bak, arr, sizeof(int)*((CLIENT_MAX+1)*(SLOT_MAX+1)));
	
	
	for(i=0; i<=(TIME_MAX/giTimeSlice); i++)
	{
		
		iNow = i*giTimeSlice;
		iBaseTime = getBaseTime(iNow);
		//printf("i=%d, iBaseTime=%d\n", i, iBaseTime);
		for(j=1; j<=CLIENT_MAX; j++)
		{
			if(arr[j][0] == 0)
				break;

			//printf("iNow=%d, i=%d, iBaseTime=%d, arr[%d][1]=%d\n", iNow, i, iBaseTime, j, arr[j][1]);
			
			if(iBaseTime == iNow)
			{
				//printf("iNow=%d,iBaseTime=%d\n", iNow, iBaseTime);
				
				//sched update
				updateSched(iNow);
			}
			else if( (iBaseTime<arr[j][1]) )
			{
				//time shift
				if((arr[j][1]-iBaseTime)<giTimeSlice)
					 arr[j][1] = iBaseTime;
				else if( (((arr[j][1]-iBaseTime)*100)/arr[j][0]) <= iAllowedRange )
					arr[j][1] = iBaseTime;
			}
		}
		
		//dump();
		
		addTempQueue(iBaseTime);
	}
	
	//dump();
	
	memcpy(arr, bak, sizeof(int)*((CLIENT_MAX+1)*(SLOT_MAX+1)));
	
	dumpTempQueue();
}
