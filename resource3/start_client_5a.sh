#!/bin/sh

sleep 0.8
./resource_client3 127.0.0.1 2048 1 1000 log5.txt 20 &

sleep 4.5
./resource_client3 127.0.0.1 2048 1 1000 log5.txt 20 &

sleep 2.3
./resource_client3 127.0.0.1 2048 1 1000 log5.txt 20 &

sleep 1.75
./resource_client3 127.0.0.1 2048 1 1000 log5.txt 20 &

sleep 5.4
./resource_client3 127.0.0.1 2048 1 1000 log5.txt 20 &

