#!/bin/bash
gcc serialization.c monitoring.c imonitord.c -pthread -o imonitord
./imonitord kill
valgrind --leak-check=full --log-file=./debug.log --trace-children=yes --track-origins=yes ./imonitord --daemon
