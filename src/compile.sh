#!/bin/bash
gcc serialize.c imonitor.c -o imonitor
echo "Check valgrind logs at: debug.log"
./imonitor add /var/log
./imonitor list
