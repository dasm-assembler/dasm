#!/bin/bash

# $Id: run_valgrind.sh 273 2008-11-02 05:07:08Z phf $
#
# Simple test script adapted from Matt Dillon's 2.16  release.
# It's harder to do this in a Makefile, so let's use a script.

for i in *.asm
do
  NAME=`basename $i .asm`
  echo "----- $NAME -----"
  valgrind -v --tool=memcheck --leak-check=full --error-exitcode=10 --show-reachable=yes --log-file=$NAME.valgrind ../bin/dasm $i -f1 -o$NAME.bin -DINEEPROM
  echo "valgrind returned $?"
done
