#!/bin/sh

sleep 0.8
./resource_client 127.0.0.1 2048 0 1300 1 20 &

sleep 1.2
./resource_client 127.0.0.1 2048 0 1100 1 25 &

sleep 1.0
./resource_client 127.0.0.1 2048 0 700 1 35 &

sleep 0.7
./resource_client 127.0.0.1 2048 0 600 1 40 &

sleep 1.4
./resource_client 127.0.0.1 2048 0 400 1 70 &

