all:
	gcc -o resource_server resource_server.c -lpthread
	gcc -o resource_proxy resource_proxy.c -lpthread
	gcc -o resource_client resource_client.c

clean:
	rm -f resouce_server resource_proxy resource_client *.bak *.o *.log
