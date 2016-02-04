#!/bin/sh
echo "Start Server..."
/home/minjin/resource/resource_server3 1024 &
echo "Start Proxy..."
/home/minjin/resource/resource_proxy3 2048 127.0.0.1 1024 1 1 &
