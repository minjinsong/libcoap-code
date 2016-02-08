#!/bin/sh

echo $SHELL
echo "-----------------------------"
MAXCOUNT=5
RESULT=`ps -ef | grep resource_proxy3`
echo "max count ="$MAXCOUNT
echo $RESULT

if [ $RSULT ]; 
then
	echo "no"
else
	echo "yes"
fi

#while [$RESULT]
#do
#	RESULT = `ps -ef | grep resource_proxy3`
#done 
