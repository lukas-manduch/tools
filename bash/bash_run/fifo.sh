#!/bin/bash

# Simple proof of concept script
# Launch process and feed to stdin some data
# after some time save output from process and
# feed another


echo "Script takes one optional argument - executable name"

##########
EXECUTABLE=./prg.sh

if [ $# -gt 0 ]; then
    EXECUTABLE="$1"
fi

if [ ! -f $EXECUTABLE ]; then
    echo "File doesn't exist $EXECUTABLE"
    exit 2
fi
##########

fifo="/tmp/fifo-"$( date +"%s" )

mkfifo $fifo
if [ $? -ne 0 ]; then
    echo "Unable to create pipe"
    exit 1
fi
##########

function onExit()
{
    echo "Exitting"

    rm -f $fifo
    kill -SIGKILL $EXECUTABLE_PID 2>/dev/null
    echo "Output is in file: $OUTPUT_NAME"
}

trap onExit EXIT

##########

OUTPUT_NAME="$( mktemp )"

exec 3<> $fifo
exec 4<> $OUTPUT_NAME

# Resolve path to executable
EXECUTABLE="$( readlink -f "$EXECUTABLE" )"

# Launch
chmod +rx "$EXECUTABLE"
$EXECUTABLE  <&3 >&4 &

EXECUTABLE_PID=$!


# Send command
echo "Command 1" >&3

sleep 1

kill -SIGSTOP $EXECUTABLE_PID 2>/dev/null

echo "Iteration finished"
echo "" >&4
echo "After 1 second" >&4
echo "" >&4

echo "Command 1" >&3

kill -SIGCONT $EXECUTABLE_PID 2>/dev/null

sleep 2

echo "Iteration finished"
echo "" >&4
echo "After 2 seconds" >&4
echo "" >&4

kill -SIGSTOP $EXECUTABLE_PID 2>/dev/null
