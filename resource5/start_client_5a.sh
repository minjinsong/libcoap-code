#!/bin/sh

sleep 0.8
./resource_client 127.0.0.1 2048 1 1300 /tmp/log1.txt 20 &

sleep 4.5
./resource_client 127.0.0.1 2048 1 700 /tmp/log2.txt 20 &

sleep 2.3
./resource_client 127.0.0.1 2048 1 400 /tmp/log3.txt 20 &

sleep 1.75
./resource_client 127.0.0.1 2048 1 1100 /tmp/log4.txt 20 &

sleep 5.4
./resource_client 127.0.0.1 2048 1 600 /tmp/log5.txt 20 &

