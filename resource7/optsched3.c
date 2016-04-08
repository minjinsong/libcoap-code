/*
	build : gcc -o opt.c opt
	execution : ./opt {time_slice_ms} {allowed_timeshift_%} {new_client_interval_ms} {new_client_starttime_ms} {old_client_count} {old_client1_interval} {old_client1_starttime} ... 
	eg. ./a.out 100 0 800 500 2 1000 500 1200 800
	eg. ./optsched 100 0 1300 100 20 4900 4900 4400 4400 3900 3900 3400 3400 2900 2900 2400 2400 1900 1900 1400 1400 900 900 400 400 4800 4800 4300 4300 3800 3800 3300 3300 2800 2800 2300 2300 1800 1800 1300 1300 800 800 300 300
	./optsched 100 0 800 100 20 400 400 900 900 1400 1400 1800 1800 2300 2300 2800 2800 3300 3300 3800 3800 4300 4300 4800 4800 5300 5300 5800 5800 6300 6300 6800 6800 7300 7300 7800 7800 8300 8300 8800 8800 9300 9300 9800 9800
	./optsched3 100 0 800 100 10 4900 4900 4400 4400 3900 3900 3400 3400 2900 2900 2400 2400 1900 1900 1400 1400 900 900 400 400
  ./optsched3 100 0 800 100 20 4900 4900 4400 4400 3900 3900 3400 3400 2900 2900 2400 2400 1900 1900 1400 1400 900 900 400 400 4800 4800 4300 4300 3800 3800 3300 3300 2800 2800 2300 2300 1800 1800 1300 1300 800 800 300 300
  ./optsched3 100 0 800 100 30 4900 4900 4400 4400 3900 3900 3400 3400 2900 2900 2400 2400 1900 1900 1400 1400 900 900 400 400 4800 4800 4300 4300 3800 3800 3300 3300 2800 2800 2300 2300 1800 1800 1300 1300 800 800 300 300 4700 4700 4200 4200 3700 3700 3200 3200 2700 2700 2200 2200 1700 1700 1200 1200 700 700 5200 5200
  ./optsched3 100 0 800 100 40 4900 4900 4400 4400 3900 3900 3400 3400 2900 2900 2400 2400 1900 1900 1400 1400 900 900 400 400 4800 4800 4300 4300 3800 3800 3300 3300 2800 2800 2300 2300 1800 1800 1300 1300 800 800 300 300 4700 4700 4200 4200 3700 3700 3200 3200 2700 2700 2200 2200 1700 1700 1200 1200 700 700 5200 5200 4600 4600 4100 4100 3600 3600 3100 3100 2600 2600 2100 2100 1600 1600 1100 1100 600 600 5100 5100
  ./optsched3 100 0 800 100 50 4900 4900 4400 4400 3900 3900 3400 3400 2900 2900 2400 2400 1900 1900 1400 1400 900 900 400 400 4800 4800 4300 4300 3800 3800 3300 3300 2800 2800 2300 2300 1800 1800 1300 1300 800 800 300 300 4700 4700 4200 4200 3700 3700 3200 3200 2700 2700 2200 2200 1700 1700 1200 1200 700 700 5200 5200 4600 4600 4100 4100 3600 3600 3100 3100 2600 2600 2100 2100 1600 1600 1100 1100 600 600 5100 5100 5000 5000 4500 4500 4000 4000 3500 3500 3000 3000 2500 2500 2000 2000 1500 1500 1000 1000 500 500
  ./optsched3 100 0 1300 1300 30 700 700 1100 1100 1500 1500 1900 1900 2300 2300 2700 2700 3100 3100 3400 3400 3800 3800 4200 4200 4600 4600 5100 5100 5500 5500 5900 5900 6300 6300 900 900 1300 1300 1700 1700 2100 2100 2600 2600 2900 2900 3300 3300 3700 3700 4100 4100 4700 4700 4900 4900 5300 5300 5700 5700 6100 6100 6700 6700 
	new client만 time shift할 수 있다.
*/
 
#include <stdio.h>
#include <string.h>

#define DUMP					1
#define TIME_MAX			500000 //(0xFFFFFFF)	//1000000
#define INTERVAL_MIN	100
#define SLOT_MAX			(TIME_MAX)/(INTERVAL_MIN)
#define CLIENT_MAX		101
#define QUANTUM				50

int arr[CLIENT_MAX+1][SLOT_MAX+1] = {0,};
int bak[CLIENT_MAX+1][SLOT_MAX+1] = {0,};
int queue[(CLIENT_MAX+1)*(SLOT_MAX+1)] = {0,};
int temp[(CLIENT_MAX+1)*(SLOT_MAX+1)] = {0,};
static int giTimeSlice = 0;
//static int giTimeDuration = 0;
static int giEndTime = 0;
static int iAllowedRange = 0;

void process();
void optimalresult(int index);
static int giMin = 0x7FFFFFFF;

void dumpClientsInfo()
{
#if DUMP		
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
	//printf("giEndTime=%d\n", giEndTime);
	if((iAlready==0) && (iTime<giEndTime) )
	{
		temp[0]++;
		temp[temp[0]] = iTime;
	}
}

void dumpResultQueue()
{
#if DUMP	
	int i;
	for(i=1; i<=queue[0]; i++)
	{
		printf("%d ", queue[i]);
	}
	printf(" --> message count=%d\n", queue[0]);
#endif	
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

unsigned int getGCD(int a, int b)
{
	if(b==0) 
		return a;
	else
		return getGCD(b, a%b);
}

unsigned int getLCM(int a, int b)
{
	return (a*b)/getGCD(a, b);
}

int getTimeDuration()
{
	int i;
	int iResult = arr[0][0];
	
	for(i=1; i<=CLIENT_MAX; i++)
	{
		if(arr[i][0] == 0)
			break;
			
		iResult = getLCM(iResult, arr[i][0]);
		//printf("iRsult=%d\n", iResult);
	}

#if 0
	iResult = TIME_MAX;
#else	
	if(iResult > TIME_MAX)
	{
		printf("downsize from %d to %d\n", iResult, TIME_MAX);
		iResult = TIME_MAX;
	}
#endif
		
	return iResult;
}

int main(int argc , char *argv[])
{
	int iNewInterval = 0;
	int iNewIntervalStartTime = 0;
	
	int iAlgorithm = 0;
	//int iAllowedRange = 0;
	int iMessageCount = 0;
	int iClientCount = 0;
	//int iDurationTime = 0;
	
	if(argc < 5)
	{
		printf("usage : %s {time_slice} {allowed_percent} {new_client_interval} {new_client_starttime} {old_client_count} {old_client1_interval} {old_client1_starttime} ... \n", argv[0]);
		printf("eg. ./a.out 50 20 800 500 2 1000 500 1200 800\n");
		//exit(0);
	}	//if(argc
	else
	{
		giTimeSlice = atoi(argv[1]);
		if(giTimeSlice == 0) giTimeSlice = QUANTUM;
		iAllowedRange = atoi(argv[2]);
		iNewInterval = atoi(argv[3]);
		iNewIntervalStartTime = atoi(argv[4]);
		iClientCount = atoi(argv[5]);
		
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
	//dumpClientsInfo();
	
	//giEndTime = getTimeDuration();
	//giEndTime = getBaseTime(0) + giEndTime;
	//printf("getTimeDuation=%d\n", giEndTime);
	
	
	printf("===== Started ===== \n");
	
	
	int iStartTime = 0;
	//int iEndTime = 0;
#if 1
{
	//1.normal
	arr[iClientCount+1][0] = arr[0][0];
	arr[iClientCount+1][1] = arr[0][1];
	//dumpClientsInfo();
	
	//giEndTime = getBaseTime(0) + getTimeDuration();
	//giEndTime = getBaseTime(0) + getTimeDuration();
	iStartTime = arr[0][1];
	giEndTime = iStartTime + getTimeDuration();
	//giEndTime = getTimeDuration();
	printf("1.Normal : ");
	
	process();
	memcpy(queue, temp, sizeof(int)*(CLIENT_MAX+1)*(SLOT_MAX+1));
	dumpResultQueue();
	printf("Messages=%d(%d~%d)\n", queue[0], iStartTime, giEndTime);
}
#endif	


#if 1
{
	unsigned int iMinLCM = 0xFFFFFFFF;
	unsigned int iTempLCM = 0;
	int iMinLCDIntervalIdx = 0;
	int iTempGap = 0;
	int iTmpLcm = 0;
	
	arr[iClientCount+1][0] = 0;
	arr[iClientCount+1][1] = 0;
	
	int i, j;
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
	
	arr[i][0] = arr[0][0];
	if(iMinLCDIntervalIdx == 0)
		arr[i][1] = arr[i][0];
	else
	{
		arr[i][1] = arr[iMinLCDIntervalIdx][1];
		iTmpLcm = arr[iMinLCDIntervalIdx][1];
	
		//printf("arr[%d][1]=%d, arr[%d][0]=%d\n", i, arr[i][1], i, arr[i][0]);	
		while(arr[i][1] > arr[i][0])
		{
			arr[i][1] = arr[i][1] - arr[i][0];
		}
	}
	//printf("arr[%d][1]=%d, iMinLCDIntervalIdx=%d\n", i, arr[i][1], iMinLCDIntervalIdx);
	
	iStartTime = arr[i][1] ;
	giEndTime = iStartTime + getTimeDuration();
	//giEndTime = getTimeDuration();
	printf("2.LCM : ");
		
	process();
	memcpy(queue, temp, sizeof(int)*(CLIENT_MAX+1)*(SLOT_MAX+1));
	dumpResultQueue();
	printf("Messages=%d(%d~%d, lcm=%d)\n", queue[0], iStartTime, giEndTime, iTmpLcm);
}
#elif 0
{
	//5.lcm
	int iMinLCM = 0x7FFFFFFF;
	unsigned int iTempLCM = 0;
	int iMinLCDIntervalIdx = 0;
	int iTempGap = 0;
	
	//TODO: get min LCM index
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
	
	while()
	{
		arr[i][1]
	}
	
	process();
	memcpy(queue, temp, sizeof(int)*(CLIENT_MAX+1)*(SLOT_MAX+1));
	dumpResultQueue();
	printf("<<LCM message count=%d>>\n", temp[0]);
}
#endif
	
#if 1
{
	//10.client optimization
	int optMsgCnt = 0x7FFFFFFF;
	int optStartTime = 0;
	int abc = 0;
	int i;
	
	//printf("TIME_MAX+iNewIntervalStartTime=%d\n", iNewIntervalStartTime+TIME_MAX);
	//int iDuration = getBaseTime(0);
	
	arr[iClientCount+1][0] = arr[0][0];
	arr[iClientCount+1][1] = 0;
	//dumpClientsInfo();
	
	printf("3.Optimized : ");
	for(i=1; i<=(arr[0][0]/giTimeSlice); i++)
	{
		//memset(temp, 0x0, sizeof(int)*((CLIENT_MAX+1)*(SLOT_MAX+1)));

		abc = abc + giTimeSlice;
		
		arr[iClientCount+1][1] = abc;
		
		iStartTime = abc;
		giEndTime = iStartTime + getTimeDuration();
		//giEndTime = getTimeDuration();
		//printf("3.Optimized (%d~%d)\n", getBaseTime(0), giEndTime);
		
		process();
		
		//dumpTempQueue();
		
		if(optMsgCnt >= temp[0])
		{
			optMsgCnt = temp[0];
			optStartTime = giTimeSlice*i;
			
			memcpy(queue, temp, sizeof(int)*(CLIENT_MAX+1)*(SLOT_MAX+1));
		}
		
		//iNewIntervalStartTime = iNewIntervalStartTime+giTimeSlice;
		//abc = abc + giTimeSlice;
	}
	dumpResultQueue();
	printf("Messages=%d(%d~%d, optStartTime=%d)\n", queue[0], iStartTime, giEndTime, optStartTime);
}
#endif
	
	printf("===== Fnished ===== \n");
	
	return 0;
}

void process()
{
	int iBaseTime = 0;
	int iNow = 0;
	int i, j;
	
	memset(temp, 0x0, sizeof(int)*((CLIENT_MAX+1)*(SLOT_MAX+1)));
	memcpy(bak, arr, sizeof(int)*((CLIENT_MAX+1)*(SLOT_MAX+1)));
	
	//int iDuration = getBaseTime(0) + giEndTime - 1;
	
	//for(i=0; i<=(TIME_MAX/giTimeSlice); i++)
	for(i=0; i<=(giEndTime/giTimeSlice); i++)
	//for(i=0; i<=(iDuration/giTimeSlice); i++)
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
				//printf("iNow=%d, iBaseTime=%d, iAllowedRange=%d\n", iNow, iBaseTime, iAllowedRange);
				if((arr[j][1]-iBaseTime)<giTimeSlice)
					 arr[j][1] = iBaseTime;
				else if( (((arr[j][1]-iBaseTime)*100)/arr[j][0]) <= iAllowedRange )
					arr[j][1] = iBaseTime;
			}
		}
		
		//dumpClientsInfo();
		
		addTempQueue(iBaseTime);
	}
	
	//dumpClientsInfo();
	
	memcpy(arr, bak, sizeof(int)*((CLIENT_MAX+1)*(SLOT_MAX+1)));
	
	//dumpTempQueue();
}
