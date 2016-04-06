/*
	build : gcc -o opt.c resource_proxy2.c -lpthread	
	execution : ./opt {new_interval} {new_interval_start_time} {client_count} {interval1} {interval2} ... 

*/
 
#include <stdio.h>
#include <string.h>

#define TIME_MAX			100000
#define INTERVAL_MIN	100
#define SLOT_MAX			(TIME_MAX)/(INTERVAL_MIN)
#define CLIENT_MAX		20
#define QUANTUM				100

int arr[CLIENT_MAX+1][SLOT_MAX+1] = {0,};
int queue[CLIENT_MAX*SLOT_MAX+1] = {0,};
int temp[CLIENT_MAX*SLOT_MAX+1] = {0,};

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

void dump()
{
#if 0	
	int i, j;
	for(i=1; i<=CLIENT_MAX+1; i++)
	{
		if(arr[i][0] == 0)
			break;
		
		for(j=1; j<=SLOT_MAX+1; j++)
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


void dumpTempQueue()
{
	int i;
	for(i=1; i<=temp[0]; i++)
	{
		printf("%d ", temp[i]);
	}
	printf(" --> message count=%d\n", temp[0]);
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

int main(int argc , char *argv[])
{
	int iNewInterval = 0;
	int iNewIntervalStartTime = 0;
	int iMessageCount = 0;
	int iClientCount = 0;
	
	if(argc < 2)
	{
		//printf("usage : %s [proxy_port#] [server_ip#] [server_port#] [cache_mode#] [cache_algorithm#]\n", argv[0]);
		//exit(0);
	}	//if(argc
	else
	{
		iNewInterval = atoi(argv[1]);
		iNewIntervalStartTime = atoi(argv[2]);
		iClientCount = atoi(argv[3]);
		
		arr[0][0] = iNewInterval;
		
		int i;
		for(i=1; i<=iClientCount; i++)
		{
			arr[i][0] = atoi(argv[i+3]);
		}
	}
	
	int i, j;
	for(i=1; i<=CLIENT_MAX+1; i++)
	{
		if(arr[i][0] == 0)
			break;
		for(j=1; j<=SLOT_MAX+1; j++)
		{
				if(arr[i][0]*j <= TIME_MAX)
				{
					arr[i][j] = arr[i][0]*j;
					addQueue(arr[i][j]);
				}
				else
				{
					break;
				}
		}
	}
	
	//1.normal
	memcpy(temp, queue, sizeof(int)*(CLIENT_MAX*SLOT_MAX+1));
	//for(i=1; i<=CLIENT_MAX+1; i++)
	{
		//if(arr[0][0] == 0)
		//	break;
		for(j=0; j<=SLOT_MAX+1; j++)
		{
				if(arr[0][0]*j+iNewIntervalStartTime <= TIME_MAX)
				{
					arr[0][j+1] = arr[0][0]*j+iNewIntervalStartTime;
					addTempQueue(arr[0][j+1]);
				}
				else
				{
					break;
				}
		}
	}
	//dumpTempQueue();
	printf("normal message count=%d\n", temp[0]);
	
	
	//3.normal
	int iMinLCM = 0x7FFFFFFF;
	int iTempLCM = 0;
	int iMinLCDIntervalIdx = 0;
	int iTempGap = 0;
	memcpy(temp, queue, sizeof(int)*(CLIENT_MAX*SLOT_MAX+1));
	for(i=1; i<=CLIENT_MAX+1; i++)
	{
		if(arr[i][0] == 0)
			break;
		//printf("arr[0][0]=%d, arr[%d][0]=%d\n", arr[0][0], i, arr[i][0]);
		iTempLCM = getLCM(arr[0][0], arr[i][0]);
		if(iMinLCM > iTempLCM)
		{
			iMinLCDIntervalIdx = i;
			iMinLCM = iTempLCM;
		}
	}
	
	for(i=1; i<=CLIENT_MAX+1; i++)
	{
		if(arr[iMinLCDIntervalIdx][i] > iNewIntervalStartTime)
		{
			//iTempGap = arr[iMinLCDIntervalIdx][i] - iNewIntervalStartTime;
			iTempGap = arr[iMinLCDIntervalIdx][i];
			break;
		}
	}
	
	if(iTempGap > arr[0][0])
	{
		iTempGap = iTempGap - arr[0][0];
	}
	
		for(j=0; j<=SLOT_MAX+1; j++)
		{
				if(arr[0][0]*j+iTempGap <= TIME_MAX)
				{
					arr[0][j+1] = arr[0][0]*j+iTempGap;
					addTempQueue(arr[0][j+1]);
				}
				else
				{
					break;
				}
		}
	
	//printf("MinLCM=%d, iMinLCDIntervalIdx=%d\n", iMinLCM, iMinLCDIntervalIdx);
	
	dumpTempQueue();
	printf("<<lcm message count=%d>>\n", temp[0]);
	
#if 1
	//10. optimal result
	int optMsgCnt = 0x7FFFFFFF;
	int optTime = 0;
	
	iNewIntervalStartTime = 0;
	//memcpy(temp, queue, sizeof(int)*(CLIENT_MAX*SLOT_MAX+1));
	for(i=0; i<(arr[0][0]/QUANTUM); i++)
	{
		memcpy(temp, queue, sizeof(int)*(CLIENT_MAX*SLOT_MAX+1));

		//if(arr[0][0] == 0)
		//	break;
		for(j=0; j<=SLOT_MAX+1; j++)
		{
				if(arr[0][0]*j+iNewIntervalStartTime <= TIME_MAX)
				{
					arr[0][j+1] = arr[0][0]*j+iNewIntervalStartTime;
					addTempQueue(arr[0][j+1]);
				}
				else
				{
					break;
				}
		}
		
		dumpTempQueue();
		
		if(optMsgCnt > temp[0])
		{
			optMsgCnt = temp[0];
			optTime = QUANTUM*i;
		}
		
		iNewIntervalStartTime = iNewIntervalStartTime+QUANTUM;
	}
	printf("<<optimal message count=%d, optTime=%d>>\n", optMsgCnt, optTime);
#endif	
	
	
	
	return 0;
}
