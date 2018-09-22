#!/bin/bash
./imonitor add /var/log ;
./imonitor add `pwd`;
./imonitor add /var;

./imonitor remove /var/log ;
./imonitor add /;

./imonitor list;
