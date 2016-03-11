#!/bin/sh

sleep 0.8
./resource_client 127.0.0.1 2048 0 500 1 20 &

sleep 4.5
./resource_client 127.0.0.1 2048 0 1000 1 20 &

sleep 2.3
./resource_client 127.0.0.1 2048 0 400 1 20 &

sleep 1.75
./resource_client 127.0.0.1 2048 0 750 1 20 &

sleep 5.4
./resource_client 127.0.0.1 2048 0 1000 1 20 &

