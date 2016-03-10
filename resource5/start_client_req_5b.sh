#!/bin/sh

sleep 2.3
./resource_client 127.0.0.1 2048 0 1000 /tmp/log6.txt 20 &

sleep 3.6
./resource_client 127.0.0.1 2048 0 500 /tmp/log7.txt 20 &

sleep 1.6
./resource_client 127.0.0.1 2048 0 1000 /tmp/log8.txt 20 &

sleep 4.2
./resource_client 127.0.0.1 2048 0 400 /tmp/log9.txt 20 &

sleep 2.5
./resource_client 127.0.0.1 2048 0 500 /tmp/log10.txt 20 &

