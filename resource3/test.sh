#!/bin/sh

MAXCOUNT=10
RANGE=10
count=1
i=1

echo $MAXCOUNT
echo "-----------------------------"
while [ "$count" -le $MAXCOUNT ]
do
  let "i = 1"
  while [ "$i" -le $count ]
  do
		number=$RANDOM
  	let "number %= $RANGE"
  	echo "random="$number
  	sleep $number
  	./resource_client3 127.0.0.1 2048 1 4000 log2.txt 50 &
  	
  	let "i += 1" 
  done
 
 	let "count += 1" 
done
echo "-----------------"
