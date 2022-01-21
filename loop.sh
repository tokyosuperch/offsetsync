#!/bin/sh

SLEEP_SEC=3500
LOOP_MAX=168

COUNT=0
while [ $COUNT -lt $LOOP_MAX ]
do
  
  ./client.elf >> result0119.csv
  
  sleep ${SLEEP_SEC}
  COUNT=`expr ${COUNT} + 1`
done
