#!/bin/sh

sleep 2.3
./resource_client 127.0.0.1 2048 1 1700 1 20 &

sleep 3.6
./resource_client 127.0.0.1 2048 1 800 1 20 &

sleep 1.6
./resource_client 127.0.0.1 2048 1 1200 1 20 &

sleep 4.2
./resource_client 127.0.0.1 2048 1 500 1 20 &

sleep 2.5
./resource_client 127.0.0.1 2048 1 900 1 20 &

