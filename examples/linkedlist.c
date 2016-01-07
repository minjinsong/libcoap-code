/*
	build : gcc -o resource_proxy resource_proxy.c -lpthread	
	execution : ./resource_proxy 2048 127.0.0.1 1024 
*/
 

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

struct __node {
	int number;
	struct __node *next;
};

struct __header {
	int total;
	struct __node *next;
};

struct __header header;

int init()
{
	;
}

int dump()
{
	struct __node *node;
	node = header.next;
	
	while(node)
	{
		printf("node->number=%d\n", node->number);
		node = node->next;
	}
		
	return 0;
}

int add(int num)
{
	struct __node *newNode;
	struct __node *prev;

	newNode = (struct __node *)malloc(sizeof(struct __node));
	newNode->number = num;
	
	newNode->next = header.next;
	header.next = newNode;
	
	return 0;
}

int main(void)
{
	
	init();
	
	add(1);
	add(2);
	add(3);
	add(4);
	add(5);
	
	dump();
	
	remove(1);
	
	return 0;
}