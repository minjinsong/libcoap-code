#!/bin/sh

sleep 2.3
./resource_client3 127.0.0.1 2048 1 1000 log10.txt 5 &

sleep 3.6
./resource_client3 127.0.0.1 2048 1 1000 log10.txt 5 &

sleep 1.6
./resource_client3 127.0.0.1 2048 1 1000 log10.txt 5 &

sleep 4.2
./resource_client3 127.0.0.1 2048 1 1000 log10.txt 5 &

sleep 2.5
./resource_client3 127.0.0.1 2048 1 1000 log10.txt 5 &

