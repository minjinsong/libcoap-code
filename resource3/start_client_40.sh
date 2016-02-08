#!/bin/sh

MAXCOUNT=10
RANGE=10

rm -rf log40.txt

./resource_client3 127.0.0.1 2048 1 4000 log40.txt 25 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 1 3000 log40.txt 25 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 1 5000 log40.txt 25 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 1 2000 log40.txt 25 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 1 6000 log40.txt 25 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 1 4000 log40.txt 25 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 1 3000 log40.txt 25 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 1 5000 log40.txt 25 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 1 2000 log40.txt 25 &

number=$RANDOM
let "number %= $RANGE"
sleep $number
./resource_client3 127.0.0.1 2048 1 6000 log40.txt 25 &
