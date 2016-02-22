#!/bin/sh

MAXCOUNT=10
RANGE=10

rm -rf log10.log

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 0 4000 /tmp/log10.txt 25 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 0 3000 /tmp/log10.txt 25 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 0 5000 /tmp/log10.txt 25 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 0 2000 /tmp/log10.txt 25 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 0 6000 /tmp/log10.txt 25 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 0 4000 /tmp/log10.txt 25 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 0 3000 /tmp/log10.txt 25 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 0 5000 /tmp/log10.txt 25 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 0 2000 /tmp/log10.txt 25 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 0 6000 /tmp/log10.txt 25 &
