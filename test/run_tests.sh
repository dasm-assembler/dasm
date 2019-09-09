#!/bin/bash

# Simple test script adapted from Matt Dillon's 2.16  release.
# It's harder to do this in a Makefile, so let's use a script.

for i in *.bin.ref
do
  i=$(echo $i | sed 's/.bin.ref/.asm/g')
  NAME=`basename $i .asm`
  echo "----- $NAME -----"
  ../bin/dasm $i -f1 -o$NAME.bin -l$NAME.list.txt -DINEEPROM
  # echo "dasm returned $?"
  cmp -s $NAME.bin $NAME.bin.ref
  if [ $? == 0 ]
  then
    echo "------------------------good"
  else
    echo "------------------------error"
  fi
  ../bin/ftohex 1 $NAME.bin $NAME.hex
  # echo "ftohex returned $?"
  cmp -s $NAME.hex $NAME.hex.ref
  if [ $? == 0 ]
  then
    echo "------------------------good"
  else
    echo "------------------------error"
  fi
done
