#!/bin/bash
if [ -z $1 ]
then
echo "ERROR: \$1 should contain commit message!"
exit 1
elif [ -z $2 ]
then
echo "ERROR: \$2 should contain branch name!"
exit 1
fi 

message=$1
branch=$2

git commit -a -m '$message'
git push origin $branch 
