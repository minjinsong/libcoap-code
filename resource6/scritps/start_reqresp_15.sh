#!/bin/sh

sleep 0.8
./resource_client 127.0.0.1 2048 0 2700 1 20 &

sleep 1.2
./resource_client 127.0.0.1 2048 0 2200 1 25 &

sleep 1.0
./resource_client 127.0.0.1 2048 0 1700 1 30 &

sleep 0.7
./resource_client 127.0.0.1 2048 0 1200 1 35 &

sleep 1.4
./resource_client 127.0.0.1 2048 0 700 1 60 &

