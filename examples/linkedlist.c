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
	//struct __node *tail;
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


printf("%s:header=0x%x\n", __func__, header);
printf("%s:header.total=%d\n", __func__,  header.total);	
printf("%s:header.next=0x%x\n", __func__, node);
	
	while(node)
	{
		printf("node->number=%d\n", node->number);
		node = node->next;
	}
		
	return 0;
}

int insert(int num)
{
	struct __node *node;
	struct __node *newNode;
	struct __node *prev;
	struct __node *temp;
	
	node = header.next;

	//newNode->next = header.next;
	//header.next = newNode;
	
header.total = 10;
	
	newNode = (struct __node *)malloc(sizeof(struct __node));
	newNode->number = num;
	newNode->next = NULL;
	
	if(!header.next)
		header.next = newNode;
	else
	{
		while(node)
		{
			//printf("node->number=%d\n", node->number);
			if(node->number < num)
			{
				prev = node;
				node = node->next;	
			}
			else
			{
				if(node == header.next)
				{
printf("%s:node=0x%x\n", __func__, node);
					temp = header.next;
					header.next = newNode;
					newNode->next = temp;
printf("9\n");	
				}
				else
				{
					temp = prev->next;
					prev->next = newNode;
					newNode->next = temp;
				}
				break;
			}
		}
		
		prev->next = newNode;
	}
	
	
	return 0;
}

int insertTail(int num)
{
	struct __node *node;
	struct __node *newNode;
	struct __node *prev;
	
	node = header.next;

	newNode = (struct __node *)malloc(sizeof(struct __node));
	newNode->number = num;
	newNode->next = NULL;
	
	if(header.next == NULL)
		header.next = newNode;
	else
	{
		while(node)
		{
				prev = node;
				node = node->next;	
		}
		prev->next = newNode;
	}
	
	return 0;
}

int insertHead(int num)
{
	struct __node *newNode;
	struct __node *prev;

	newNode = (struct __node *)malloc(sizeof(struct __node));
	newNode->number = num;
	
	newNode->next = header.next;
	header.next = newNode;
	
	return 0;
}

int removeAll()
{
	
	struct __node *node;
	struct __node *newNode;
	
	node = header.next;

	if(header.next == NULL)
		header.next = newNode;
	else
	{
		while(node)
		{
			struct __node *temp = node;
			node = node->next;
			free(temp);
			temp = NULL;
		}

	}
	
	return 0;
}

int main(void)
{
	
	init();
	
#if 1
	insertHead(1);
	insertHead(2);
	insertHead(3);
	insertHead(4);
	insertHead(5);
//removeAll();
	insertTail(1);
	insertTail(2);
	insertTail(3);
	insertTail(4);
	insertTail(5);
#else
	insert(1);
	insert(2);
	insert(3);
#endif	
	dump();
	
	//remove(1);
	
	return 0;
}