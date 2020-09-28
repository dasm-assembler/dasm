#!/usr/bin/env bash

# Simple test script adapted from Matt Dillon's 2.16  release.
# It's harder to do this in a Makefile, so let's use a script.

should_fail="0"

REL_DIR="../bin"

DEFINES=""
format="1"

while [ "$1" != "" ]
do
    case "$1" in 
	-R)	REL_DIR="$2"
		shift
		;;
	-S)	should_fail="$2"
		shift
		;;
	-D)	DEFINES="${DEFINES} -D$2"
		shift
		;;
	-F)	format="$2"
		shift
		;;
	-I)	DEFINES="${DEFINES} -I$2"
		;;
    esac
    shift
done

echo "=== build test cases (strict=off)"

fail="0"
ok="0"
nTests="0"

leadingSpace() {
    echo "$@" | awk '{ printf(" * % 32s", $0)}'
}

refFiles=`find . -type f -name '*.bin.ref' | sort`

for item in $refFiles
do
    ARGS="-f${format}"

    NAME=$(echo $item | sed 's/\.bin\.ref$//g;s/^\.\///g')

    leadingSpace "$NAME"

    if test -r ${NAME}.args
    then
	. ${NAME}.args
    fi

  if ${REL_DIR}/dasm ${NAME}.asm ${ARGS} -o$NAME.bin -L$NAME.list.txt -DINEEPROM ${DEFINES} >$NAME.out 2>&1
  then
    if test -s $NAME.bin
    then
	${REL_DIR}/ftohex 1 $NAME.bin $NAME.hex
	if cmp -s $NAME.bin $NAME.bin.ref
	then
	    echo "	binary is same,	pass"
	    ok="$[$ok+1]"
	else
	    # second compare test is not needed, just help to find the diff
	    if ! test -r $NAME.hex.ref
	    then
		echo "		file [$NAME.hex.ref] missing	FAILED!" 
	    else
		diff -bBduw $NAME.hex $NAME.hex.ref >$NAME.hex.diff
		echo "		binaries differ	FAILED!" 
	    fi
	    fail="$[$fail+1]"
	fi 
    else
	echo "		file doesn't exist or has zero size	FAILED!" 
	fail="$[$fail+1]"
    fi
  else
    echo "		assembly	FAILED! [$?]" 
    fail="$[$fail+1]"
  fi
    nTests="$[$nTests+1]"
done

LIST=`find . -type f -name '*.fail' | sort`
if [ "$LIST" != "" ]
then

echo ""
echo "=== error test cases (strict=on)"

# test for assembly sniplets that should fail
for item in $LIST
do
    ARGS="-f${format}"

    NAME=$(echo $item | sed 's/\.fail$//g;s/^\.\///g')
    leadingSpace "$NAME"

    if test -r ${NAME}.args
    then
	. ${NAME}.args
    fi
    
    if ! test -r ${NAME}.asm
    then
	echo "source file [${NAME}.asm] is missing"
	fail="$[$fail+1]"
    else
        if ${REL_DIR}/dasm ${NAME}.asm -S ${ARGS} -o$NAME.bin -L$NAME.list.txt -DINEEPROM ${DEFINES} >${NAME}.out 2>&1
	then
	    echo "		no error	FAILED!"
    	    fail="$[$fail+1]"
	else 
	    case "$ARGS" in
		    *-R) if test -r $NAME.bin
			 then
			    echo "	file exists, FAILED!"
			    fail="$[$fail+1]"
			 else
			    echo "	file was deleted, pass"
			    ok="$[$ok+1]"
			 fi
			 ;;
		    *)	 got_error_level="$?"
			 expected_error_level=`cat $item | head -1`
			 if [ "${expected_error_level}" = "" ]
			 then
			    echo "		testcase definition incomplete, missing error_level (${got_error_level}), FAILED!"
			    fail="$[$fail+1]"
			 else
			    if [ "${got_error_level}" != "${expected_error_level}" ]
			    then
				echo "		got error_level (${got_error_level}) but expected (${expected_error_level}), double FAULT!"
				fail="$[$fail+1]"
			     else
				echo "	error_code [${got_error_level}] pass"
				ok="$[$ok+1]"
			     fi
			 fi
			 ;;
	    esac
	fi
    fi
    nTests="$[$nTests+1]"
done

fi

echo -n "executed ${nTests} tests, $ok OK, $fail failed, result: "

if [ "$[$ok+${should_fail}]" = "$nTests" ]
then
    echo "overall PASS"
    exit 0
else
    suspect="$[$fail-${should_fail}]"
    echo "need to investigate in $suspect test-cases"
    exit $suspect
fi



