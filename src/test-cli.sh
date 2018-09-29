#!/bin/bash
gcc imonitor.c serialization.c -o imonitor
./imonitor add /var/log ;
./imonitor add `pwd`;
./imonitor add /var;

./imonitor remove /var/log ;
./imonitor add /;

./imonitor list;

