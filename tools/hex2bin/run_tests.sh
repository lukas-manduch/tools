#!/bin/bash

set +e

BIN_NAME=hex2bin

echo "RUNNING TESTS FOR $BIN_NAME"

check_binary() {
	if [ $# -ne 2 ]; then
		echo "Bad number of arguments for inside function"
		exit 1
	fi

       f1="$(  sha256sum $1 | cut -f 1 -d ' ' )"
       f2="$(  sha256sum $2 | cut -f 1 -d ' ' )"
       if [ "$f1" != "$f2" ]; then
	       echo "File mismatch $1 $2"
       else
	       echo "Test OK"
       fi
}

echo "TEST 1"
OUT1="$(mktemp)"
./hex2bin ./tests/input1.txt "$OUT1"
check_binary "$OUT1" ./tests/out1.bin
rm $OUT1

echo "TEST 2"
OUT2="$(mktemp)"
./hex2bin ./tests/input2.txt "$OUT2" 2>/dev/null
check_binary "$OUT2" ./tests/out2.bin
rm $OUT2
