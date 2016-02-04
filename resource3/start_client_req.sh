#!/bin/sh

MAXCOUNT=10
RANGE=10

./resource_client3 127.0.0.1 2048 0 4000 log_req.txt 25 &

number=$RANDOM
let "number %= $RANGE"
let "number += 2"
sleep $number
./resource_client3 127.0.0.1 2048 0 3000 log_req.txt 25 &

number=$RANDOM
let "number %= $RANGE"
let "number += 2"
sleep $number
./resource_client3 127.0.0.1 2048 0 5000 log_req.txt 25 &