#!/bin/sh

echo "Stop Proxy..."
/usr/bin/killall resource_proxy3

echo "Stop Server..."
/usr/bin/killall resource_server3
