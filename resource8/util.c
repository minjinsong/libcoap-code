#include <stdio.h>


int gcd(int a, int b)
{
	//printf("a=%d, b=%d\n", a, b);
	if(b==0) 
		return a;
	else
		return gcd(b, a%b);
}

int lcm(int a, int b)
{
	return (a*b)/gcd(a, b);
}


int main(int argc, char *argv[])
{
	int m, n;
	//scanf("%d %d", &m, &n);
	//printf("gcd=%d\n", gcd(m, n));
	//printf("lcm=%d\n", lcm(m, n));

	FILE *pFileId = fopen("/tmp/testid", "w");
	int iGet = 10;
	int iSet = 16;
	fscanf(pFileId, "%d", &iGet);
	printf("getid=%d\n", iGet++);
	fprintf(pFileId, "%d", iSet);
	printf("getid=%d\n", iSet);
	//fclose(pFileId);
		/*
		fprintf(pfileLog, "Hello Minjin!MaxAge=%d\n", msg.uiMaxAge);
		fprintf(pfileLog, "Hello Minjin!owner=%d\n", msg.owner);
		*/
}