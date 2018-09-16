#!/bin/bash

if [ -z $1 ]
then
message='commit'
elif [ -z $2 ]
then
branch='master'
fi 

#message=$1
#branch=$2

git commit -a -m '$message'
git push origin $branch 
