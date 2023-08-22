#/bin/bash

if [ -z "$1" ]
then
  echo "Specify binary file"
  exit
fi

$IDA_HOME/ida64 -A -S"$PWD/py-scripts/ida_find_string.py $PWD/$1" $1
# $IDA_HOME/ida64 -S"$PWD/py-scripts/ida_find_string.py $PWD/$1" $1