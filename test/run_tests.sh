#!/bin/bash

# Simple test script adapted from Matt Dillon's 2.16  release.
# It's harder to do this in a Makefile, so let's use a script.

echo "=== build test cases (strict=off)"
echo

for i in *.bin.ref
do
  i=$(echo $i | sed 's/.bin.ref/.asm/g')
  NAME=`basename $i .asm`
  echo "  * $NAME"
  ../bin/dasm $i -f1 -o$NAME.bin -l$NAME.list.txt -DINEEPROM | \
    grep -vE 'error|Complete|^?'
  # echo "dasm returned $?"
  cmp -s $NAME.bin $NAME.bin.ref
  if [ $? == 0 ]
  then
    echo "    bin comparison: pass"
  else
    echo "    bin comparison: fail"
  fi
  ../bin/ftohex 1 $NAME.bin $NAME.hex
  # echo "ftohex returned $?"
  cmp -s $NAME.hex $NAME.hex.ref
  if [ $? == 0 ]
  then
    echo "    hex comparison: pass"
  else
    echo "    hex comparison: fail"
  fi
  echo
done

echo "=== error test cases (strict=on)"
echo
# test for assembly sniplets that should fail
for i in *.fail
do
  i=$(echo $i | sed 's/.fail/.asm/g')
  NAME=`basename $i .asm`
  echo "  * $NAME"
  ../bin/dasm $i -S -f1 -o$NAME.bin -l$NAME.list.txt -DINEEPROM 2>&1 | \
    grep -vE 'error|Complete|^$'
  grep error $NAME.list.txt 2>&1 >/dev/null
  if [ $? == 0 ] 
  then
    echo "    error triggered: pass"
  else
    echo "    error triggered: fail"
  fi
  echo
done
