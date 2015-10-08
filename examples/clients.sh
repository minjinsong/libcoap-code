#!/bin/sh

a=$1
date
while [ $a -gt 0 ]
do
	a=$[$a-1]
#	examples/coap-client -m get -T cafe coap://192.168.0.7:5683/test &	
#	examples/coap-client -v 1 -m get coap://[::1]/test &
#	examples/coap-client -v 1 -m get -T cafe coap://192.168.0.7:5683/time &
#	examples/coap-client -m get -T cafe coap://[::1]/time &
#	echo $a
#	examples/coap-client -l 1 -v 1 -m get coap://[::1]/test &
	examples/coap-client -i 1 -l 1000 -v 1 -m get coap://[::1]/test
#	sleep 0.001
done
date