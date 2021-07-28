#!/bin/bash

line=$1
patt=$2
for i in *.txt
do
	if [ -f "$i" ]
	then	
		a=`wc -l "$i" | cut -f1 -d' '`
		if [ $a -gt 2 ]
		then
			head -$line "$i" | tail -1 | grep -q $patt
			if [ $? -ne 0 ];then echo "Fine for $i"
			else rm "$i"
			fi
		else echo  "Fine for $i"
		fi
	fi
done
