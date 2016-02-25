#!/bin/sh

MAXCOUNT=10
RANGE=10

rm -rf log10.txt
rm -rf /tmp/proxy.log

./resource_client3 127.0.0.1 2048 1 1000 log10.txt 20 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 1 1000 log10.txt 20 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 1 1000 log10.txt 20 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 1 1000 log10.txt 20 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 1 1000 log10.txt 20 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 1 1000 log10.txt 20 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 1 1000 log10.txt 20 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 1 1000 log10.txt 20 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 1 1000 log10.txt 20 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 1 1000 log10.txt 20 &

