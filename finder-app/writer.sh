#!/bin/sh
# Second script to write a word in a file
# Author: Hector Redal 
# - first argument: Full path to a file (including file name)-> writefile
# - second argument: text string which will be written within this file -> writestr
# Example of usage: writer.sh /tmp/aesd/assignment1/sample.txt ios

set -u

if [ $# -lt 2 ]
then
  echo "Low number of arguments. The usage of the script is: writer.sh <writefile> <writestr>"
  exit 1
else
  FILENAME=$1
  WORDTOWRITE=$2
fi

if [ -f "$FILENAME" ]
then
  echo "File $FILENAME already exists. Overwriting it."
  echo "$WORDTOWRITE" > $FILENAME
else
  echo "File $FILENAME does not exist. Creating it."
  mkdir -p $FILENAME
  rm -d $FILENAME
  touch $FILENAME
  echo "$WORDTOWRITE" > $FILENAME
fi




 