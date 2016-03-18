#!/bin/sh

sleep 0.8
./resource_client 127.0.0.1 2048 1 1900 1 20 &

sleep 1.2
./resource_client 127.0.0.1 2048 1 1800 1 23 &

sleep 1.0
./resource_client 127.0.0.1 2048 1 1700 1 30 &

sleep 0.7
./resource_client 127.0.0.1 2048 1 1100 1 33 &

sleep 1.4
./resource_client 127.0.0.1 2048 1 700 1 70 &

