#!/bin/sh
# First script to find files
# Author: Hector Redal 
# - first argument: directory (filedir)
# - second argument: string (searchstr) to search in the files inside filedir

# set -e
set -u
# set -x

NUMFILES=0
NUMLINES=0
LINE=0
STRINGSTR=STRING_TO_SEARCH
DIRTOSEARCH=/tmp
username=$(cat conf/username.txt)

if [ $# -lt 2 ]
then
	echo "Low number of arguments. The usage of the script is: finder.sh <dirToSearch> <stringToSearch>"
        exit 1	
else
	DIRTOSEARCH=$1
	STRINGSTR=$2
fi

if [ -d "$DIRTOSEARCH" ]
then
	echo "Directory $DIRTOSEARCH exists."
else
	echo "Directory $DIRTOSEARCH does not exist."
	exit 1
fi

dirIternation() {
  for dir in *; do
    if [ -d "$dir" ]; then
      (cd -- "$dir" && dirIternation)
	else
	  NUMFILES=$((NUMFILES+1))
	  LINE=$(grep -c "$STRINGSTR" "$dir")
	  NUMLINES=$((NUMLINES+$LINE))
    fi
  done
}

cd $DIRTOSEARCH; dirIternation

echo "The number of files are" $NUMFILES "and the number of matching lines are" $NUMLINES

exit 0