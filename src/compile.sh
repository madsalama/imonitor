#!/bin/bash
gcc serialize.c imonitor.c -o imonitor
echo "./imonitor add /var/log"
./imonitor add /var/log
