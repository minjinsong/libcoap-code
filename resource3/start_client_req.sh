#!/bin/sh

MAXCOUNT=10
RANGE=10

rm -rf /tmp/client.log

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 0 4000 /tmp/client.log 25 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 0 3000 /tmp/client.log 25 &

number=$RANDOM
let "number %= $RANGE"
#let "number += 1"
sleep $number
./resource_client3 127.0.0.1 2048 0 5000 /tmp/client.log 25 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 0 2000 /tmp/client.log 25 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 0 6000 /tmp/client.log 25 &

