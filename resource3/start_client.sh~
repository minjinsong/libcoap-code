#!/bin/sh

MAXCOUNT=10
RANGE=10

rm -rf /tmp/client.log

./resource_client3 127.0.0.1 2048 1 4000 /tmp/client.log 25 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 1 3000 /tmp/client.log 25 &

number=$RANDOM
let "number %= $RANGE"
sleep $number 
./resource_client3 127.0.0.1 2048 1 5000 /tmp/client.log 25 &

number=$RANDOM
let "number %= $RANGE"
sleep $number 
./resource_client3 127.0.0.1 2048 1 2000 /tmp/client.log 25 &

number=$RANDOM
let "number %= $RANGE"
sleep $number 
./resource_client3 127.0.0.1 2048 1 6000 /tmp/client.log 25 &

