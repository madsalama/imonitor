#!/bin/bash
gcc serialize.c imonitord.c -o imonitord
./imonitord kill
./imonitord --daemon
