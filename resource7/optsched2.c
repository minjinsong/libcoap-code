/*
	build : gcc -o opt.c resource_proxy2.c -lpthread	
	execution : ./opt {algorithm} {allowed_%} {client_count} {interval1} {interval2} ... 
	eg. ./a.out 1 20 2 800 500

*/
 
#include <stdio.h>
#include <string.h>

#define DUMP					1
#define TIME_MAX			10000
#define INTERVAL_MIN	100
#define SLOT_MAX			(TIME_MAX)/(INTERVAL_MIN)
#define CLIENT_MAX		20
#define QUANTUM				50

int arr[CLIENT_MAX+1][SLOT_MAX+1] = {0,};
int queue[CLIENT_MAX*SLOT_MAX+1] = {0,};
int temp[CLIENT_MAX*SLOT_MAX+1] = {0,};

void process();

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
		
		for(j=0; j<=SLOT_MAX+1; j++)
		{
			if(arr[i][j] > 0)
			{
				printf("%d ", arr[i][j]);
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
	
	if(iAlready==0)
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
	for(i=1; i<=CLIENT_MAX; i++)
	{
		if(arr[i][0] == 0)
			break;
		//printf("%d ", arr[i][0]);
		
		if(iMin > arr[i][1])
		{
			iMin = arr[i][1];
		}
	}
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
			arr[i][1] = arr[i][1] + arr[i][0];
		
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
	int iAlgorithm = 0;
	//int iAllowedRange = 0;
	int iMessageCount = 0;
	int iClientCount = 0;
	
	if(argc < 2)
	{
		//printf("usage : %s [proxy_port#] [server_ip#] [server_port#] [cache_mode#] [cache_algorithm#]\n", argv[0]);
		//exit(0);
	}	//if(argc
	else
	{
		iAlgorithm = atoi(argv[1]);
		iAllowedRange = atoi(argv[2]);
		iClientCount = atoi(argv[3]);
		
		//arr[0][0] = iAlgorithm;
		
		int i;
		for(i=1; i<=iClientCount; i++)
		{
			arr[i][0] = atoi(argv[i+3]);
		}
	}
	

if(iAlgorithm == 0)
{
	//1.normal
	memset(temp, 0x0, sizeof(int)*(CLIENT_MAX*SLOT_MAX+1));
	
	int i, j;
	for(i=1; i<=CLIENT_MAX+1; i++)
	{
		if(arr[i][0] == 0)
			break;
		arr[i][1] = arr[i][0];
	}
	
	int iBaseTime = 0;
	int iNow = 0;
	for(i=1; i<(TIME_MAX/QUANTUM); i++)
	{
		iNow = i*QUANTUM;
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
				if((arr[j][1]-iBaseTime)<QUANTUM)
					 arr[j][1] = iBaseTime;
				else if( (((arr[j][1]-iBaseTime)*100)/arr[j][0]) <= iAllowedRange )
					arr[j][1] = iBaseTime;
			}
		}
		
		//dump();
		
		addTempQueue(iBaseTime);
	}

	dumpTempQueue();
	printf("<<normal message count=%d>>\n", temp[0]);
}
else if (iAlgorithm==1)
{
	//3.lcm
	int iMinLCM = 0x7FFFFFFF;
	int iTempLCM = 0;
	int iMinLCDIntervalIdx = 0;
	int iTempGap = 0;
	//memcpy(temp, queue, sizeof(int)*(CLIENT_MAX*SLOT_MAX+1));
	
	memset(temp, 0x0, sizeof(int)*(CLIENT_MAX*SLOT_MAX+1));
	
	int i, j;
	for(i=1; i<=CLIENT_MAX+1; i++)
	{
		if(arr[i][0] == 0)
			break;
			
		iMinLCM = 0x7FFFFFFF;
		iMinLCDIntervalIdx = 0;
		
		for(j=1; j<=(i-j); j++)
		{
			if(arr[j][0] == 0)
				break;
			//printf("arr[0][0]=%d, arr[%d][0]=%d\n", arr[0][0], i, arr[i][0]);
			iTempLCM = getLCM(arr[i][0], arr[j][0]);
			if(iMinLCM > iTempLCM)
			{
				iMinLCDIntervalIdx = j;
				iMinLCM = iTempLCM;
			}
		}
		
		if(iMinLCDIntervalIdx == 0)
			arr[i][1] = arr[i][0];
		else
			arr[i][1] = arr[iMinLCDIntervalIdx][1];
	}
#if 1
	process();
#else
	int iBaseTime = 0;
	int iNow = 0;
	for(i=1; i<(TIME_MAX/QUANTUM); i++)
	{
		iNow = i*QUANTUM;
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
				if((arr[j][1]-iBaseTime)<QUANTUM)
					 arr[j][1] = iBaseTime;
				else if( (((arr[j][1]-iBaseTime)*100)/arr[j][0]) <= iAllowedRange )
					arr[j][1] = iBaseTime;
			}
		}
		//dump();
		
		addTempQueue(iBaseTime);
	}
#endif		
	dumpTempQueue();
	printf("<<LCM message count=%d>>\n", temp[0]);
}
else if (iAlgorithm==2)
{
	//10.optimal
	int iMinLCM = 0x7FFFFFFF;
	int iTempLCM = 0;
	int iMinLCDIntervalIdx = 0;
	int iTempGap = 0;
	
	memset(temp, 0x0, sizeof(int)*(CLIENT_MAX*SLOT_MAX+1));
	
	int i, j;
	for(i=1; i<=CLIENT_MAX+1; i++)
	{
		if(arr[i][0] == 0)
			break;
		arr[i][1] = arr[i][0];
	}
	
	int iBaseTime = 0;
	int iNow = 0;
	for(i=1; i<(TIME_MAX/QUANTUM); i++)
	{
		iNow = i*QUANTUM;
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
				if((arr[j][1]-iBaseTime)<QUANTUM)
					 arr[j][1] = iBaseTime;
				else if( (((arr[j][1]-iBaseTime)*100)/arr[j][0]) <= iAllowedRange )
					arr[j][1] = iBaseTime;
			}
		}
		
		//dump();
		
		addTempQueue(iBaseTime);
	}

	dumpTempQueue();
	printf("<<normal message count=%d>>\n", temp[0]);
}
	return 0;
}

void process()
{
	int iBaseTime = 0;
	int iNow = 0;
	int i, j;
	for(i=1; i<(TIME_MAX/QUANTUM); i++)
	{
		iNow = i*QUANTUM;
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
				if((arr[j][1]-iBaseTime)<QUANTUM)
					 arr[j][1] = iBaseTime;
				else if( (((arr[j][1]-iBaseTime)*100)/arr[j][0]) <= iAllowedRange )
					arr[j][1] = iBaseTime;
			}
		}
		
		//dump();
		
		addTempQueue(iBaseTime);
	}
}
