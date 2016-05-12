#!/bin/sh

sleep 0.8
./resource_client 127.0.0.1 2048 1 2800 1 20 &

sleep 1.2
./resource_client 127.0.0.1 2048 1 2300 1 25 &

sleep 1.0
./resource_client 127.0.0.1 2048 1 1800 1 30 &

sleep 0.7
./resource_client 127.0.0.1 2048 1 1300 1 40 &

sleep 1.4
./resource_client 127.0.0.1 2048 1 800 1 60 &

