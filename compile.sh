#!/bin/bash

cd src
res=`gcc daemon.c mmap_file.c -o ../bin/daemon`

if [ $? -eq 0 ]
then
../bin/daemon $@
fi

cd - > /dev/null
