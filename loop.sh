#!/bin/sh

HALT_SEC=1
LOOP_MAX=100

cd /home/supachan/offsetsync
COUNT=0
while [ $COUNT -lt $LOOP_MAX ]
do
	/usr/sbin/ntpdate -p 1 -b supachan.sfc.wide.ad.jp
	if [ $? -ne 0 ]; then
		COUNT=`expr ${COUNT}`
	else
		./gpioout.elf >> gpio_ntp_out.csv
		sleep ${HALT_SEC}
		COUNT=`expr ${COUNT} + 1`
	fi
done

