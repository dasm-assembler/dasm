#!/usr/bin/env bash

# Simple test script adapted from Matt Dillon's 2.16 release.
# It's harder to do this in a Makefile, so let's use a script.
#
# .xfail support: if a test directory contains $NAME.xfail, the test is
# expected to fail. A failure counts as pass; an unexpected success fails.
# This replaces the old -S <n> "expected failure count" bodge.

REL_DIR="../bin"
DEFINES=""
format="1"

while [ "$1" != "" ]
do
    case "$1" in
        -R) REL_DIR="$2"; shift ;;
        -D) DEFINES="${DEFINES} -D$2"; shift ;;
        -F) format="$2"; shift ;;
        -I) DEFINES="${DEFINES} -I$2" ;;
    esac
    shift
done

fail="0"
ok="0"
nTests="0"

leadingSpace() {
    echo "$@" | awk '{ printf(" * % 32s", $0)}'
}

# pass LABEL MSG  -- record a passing result
pass() {
    echo "	$2,	pass"
    ok="$[$ok+1]"
}

# fail LABEL MSG  -- record a failing result
fail_result() {
    echo "		$2	FAILED!"
    fail="$[$fail+1]"
}

# xpass LABEL MSG -- unexpected pass for an xfail test
xpass() {
    echo "		unexpected pass (xfail)	FAILED!"
    fail="$[$fail+1]"
}

# xfail LABEL MSG -- expected failure for an xfail test
xfail_result() {
    echo "	$2 (xfail),	pass"
    ok="$[$ok+1]"
}

# record NAME PASSED MSG
# Checks for $NAME.xfail and routes to the appropriate handler.
record() {
    local name="$1" passed="$2" msg="$3"
    if test -r "${name}.xfail"
    then
        if [ "$passed" = "1" ]
        then xpass  "$name" "$msg"
        else xfail_result "$name" "$msg"
        fi
    else
        if [ "$passed" = "1" ]
        then pass      "$name" "$msg"
        else fail_result "$name" "$msg"
        fi
    fi
}

# ------------------------------------------------------------------
echo "=== build test cases (strict=off)"

refFiles=$(find . -type f -name '*.bin.ref' | sort)

for item in $refFiles
do
    ARGS="-f${format}"
    NAME=$(echo "$item" | sed 's/\.bin\.ref$//g;s/^\.\///g')
    leadingSpace "$NAME"

    if test -r "${NAME}.args"
    then
        . "${NAME}.args"
    fi

    if ${REL_DIR}/dasm "${NAME}.asm" ${ARGS} -o"${NAME}.bin" -L"${NAME}.list.txt" \
            -DINEEPROM ${DEFINES} >"${NAME}.out" 2>&1
    then
        if test -s "${NAME}.bin"
        then
            ${REL_DIR}/ftohex 1 "${NAME}.bin" "${NAME}.hex"
            if cmp -s "${NAME}.bin" "${NAME}.bin.ref"
            then
                record "$NAME" 1 "binary is same"
            else
                if ! test -r "${NAME}.hex.ref"
                then
                    record "$NAME" 0 "file [${NAME}.hex.ref] missing"
                else
                    diff -bBduw "${NAME}.hex" "${NAME}.hex.ref" >"${NAME}.hex.diff"
                    record "$NAME" 0 "binaries differ"
                fi
            fi
        else
            record "$NAME" 0 "file doesn't exist or has zero size"
        fi
    else
        record "$NAME" 0 "assembly failed [$?]"
    fi

    nTests="$[$nTests+1]"
done

# ------------------------------------------------------------------
LIST=$(find . -type f -name '*.fail' | sort)
if [ "$LIST" != "" ]
then

echo ""
echo "=== error test cases (strict=on)"

for item in $LIST
do
    ARGS="-f${format}"
    NAME=$(echo "$item" | sed 's/\.fail$//g;s/^\.\///g')
    leadingSpace "$NAME"

    if test -r "${NAME}.args"
    then
        . "${NAME}.args"
    fi

    if ! test -r "${NAME}.asm"
    then
        record "$NAME" 0 "source file [${NAME}.asm] is missing"
    else
        if ${REL_DIR}/dasm "${NAME}.asm" -S ${ARGS} -o"${NAME}.bin" \
                -L"${NAME}.list.txt" -DINEEPROM ${DEFINES} >"${NAME}.out" 2>&1
        then
            # dasm exited 0 — assembly succeeded when it should have failed
            record "$NAME" 0 "no error"
        else
            case "$ARGS" in
                *-R)
                    if test -r "${NAME}.bin"
                    then record "$NAME" 0 "file exists"
                    else record "$NAME" 1 "file was deleted"
                    fi
                    ;;
                *)
                    got_error_level="$?"
                    expected_error_level=$(head -1 "$item")
                    if [ "${expected_error_level}" = "" ]
                    then
                        record "$NAME" 0 \
                            "testcase definition incomplete, missing error_level (${got_error_level})"
                    elif [ "${got_error_level}" != "${expected_error_level}" ]
                    then
                        record "$NAME" 0 \
                            "got error_level (${got_error_level}) but expected (${expected_error_level})"
                    else
                        record "$NAME" 1 "error_code [${got_error_level}]"
                    fi
                    ;;
            esac
        fi
    fi

    nTests="$[$nTests+1]"
done

fi

# ------------------------------------------------------------------
echo ""
echo -n "executed ${nTests} tests, $ok OK, $fail failed, result: "

if [ "$fail" = "0" ]
then
    echo "overall PASS"
    exit 0
else
    echo "need to investigate in $fail test-cases"
    exit "$fail"
fi
