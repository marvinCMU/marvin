#!/bin/bash

ps -ae | awk '/gui/ {print $1}' | xargs kill
jps -l | grep vd.jar | cut -d ' ' -f 1 | xargs -n1 kill
exec<matlab_pid
while read line
do
kill -9 $line;
done
rm matlab_pid

