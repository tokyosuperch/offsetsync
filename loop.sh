#!/bin/sh

SLEEP_SEC=3500
HALT_SEC=1
LOOP_MAX=100

while :
do
	COUNT=0
	while [ $COUNT -lt $LOOP_MAX ]
	do
		ntpdate -p 1 -b supachan.sfc.wide.ad.jp
		if [ $? -ne 0 ]; then
			COUNT=`expr ${COUNT}`
		else
			./gpioout.elf >> gpio_ntp_out.csv
			sleep ${HALT_SEC}
			COUNT=`expr ${COUNT} + 1`
		fi
	done
	sleep ${SLEEP_SEC}
done
