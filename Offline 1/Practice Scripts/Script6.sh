#! /bin/bash

for i in *; do
if [ -f "$i" ];then
	echo "$i" | grep -q [0-9]
	if [ $? = 0 ];then  #Retrieving the return value of the last statement by $?
		rm "$i"
	fi
fi
done
