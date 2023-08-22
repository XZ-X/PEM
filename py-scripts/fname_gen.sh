#/bin/bash

if [ -z "$1" ]
then
  echo "Specify binary file"
  exit
fi

$IDA_HOME/ida64 -A -S"$PWD/py-scripts/ida_get_function_name.py $PWD/$1" $1