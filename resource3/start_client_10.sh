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
#sleep $number
sleep 1.6
./resource_client3 127.0.0.1 2048 1 1000 log10.txt 20 &

number=$RANDOM
let "number %= $RANGE"
#sleep $number
sleep 4.2
./resource_client3 127.0.0.1 2048 1 1000 log10.txt 20 &

number=$RANDOM
let "number %= $RANGE"
#sleep $number
sleep 2.5
./resource_client3 127.0.0.1 2048 1 1000 log10.txt 20 &

number=$RANDOM
let "number %= $RANGE"
#sleep $number
sleep 0.8
./resource_client3 127.0.0.1 2048 1 1000 log10.txt 20 &

number=$RANDOM
let "number %= $RANGE"
#sleep $number
sleep 4.5
./resource_client3 127.0.0.1 2048 1 1000 log10.txt 20 &

number=$RANDOM
let "number %= $RANGE"
#sleep $number
sleep 2.3
./resource_client3 127.0.0.1 2048 1 1000 log10.txt 20 &

number=$RANDOM
let "number %= $RANGE"
#sleep $number
sleep 1.75
./resource_client3 127.0.0.1 2048 1 1000 log10.txt 20 &

number=$RANDOM
let "number %= $RANGE"
#sleep $number
sleep 5.4
./resource_client3 127.0.0.1 2048 1 1000 log10.txt 20 &

