#!/bin/bash
gcc serialize.c imonitord.c -o imonitord
./imonitord kill
valgrind --leak-check=full --log-file=./debug.log --trace-children=yes --track-origins=yes ./imonitord --daemon
