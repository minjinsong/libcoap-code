#!/bin/sh

a=$1
date
while [ $a -gt 0 ]
do
	a=$[$a-1]
	examples/coap-client -m get -T cafe coap://[::1]/time &
#	echo $a
done
date