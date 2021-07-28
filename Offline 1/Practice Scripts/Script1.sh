#!/bin/bash

echo -n "Enter 1st number:"
read num1
echo -n "Enter 2nd number:"
read num2
echo $num1
echo $num2

if [ `expr $num1 % 2` = 0 ] && [ `expr $num2 % 2` = 1 ]; then 
	echo yes
elif [ `expr $num1 % 2` = 1 ] && [ `expr $num2 % 2` = 0 ]; then
	echo yes
else echo no
fi
