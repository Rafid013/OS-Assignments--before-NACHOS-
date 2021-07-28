#! /bin/bash

sum=0
num=-1
echo -n "Enter:"
read num
while [ $num -ne 0 ]
do
	if [ $num -gt 0 ];then
		sum=`expr $sum + $num`
	fi
	echo -n "Enter:"
	read num	
done
echo $sum
