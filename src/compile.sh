#!/bin/bash
gcc serialize.c imonitor.c -o imonitor
echo "Check daemon logs at: /var/tmp/imonitord.log"
./imonitor add /var/log
./imonitor list
