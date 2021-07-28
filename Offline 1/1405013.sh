#!/bin/bash

a=1

file_struct_traverse(){
	cd "$1"
	for file in *
	do
		if [ -d "$file" ] 
		then 
			b=`expr $a - 1`
			for((i=0;i<$b;i++))
			do
				echo -n '|  '
			done
			echo "|--$file"
			a=`expr $a + 1`
			file_struct_traverse "$file"
		elif [ -f "$file" ]; then
			b=`expr $a - 1`
			for((i=0;i<$b;i++))
			do
				echo -n '|  '
			done
			echo "|--$file"
		fi
	done
	a=`expr $a - 1`
	cd ../
}


if [ $# -eq 0 ] 
then 
	echo .
	file_struct_traverse .
else
	for i in $*
	do
		if [ -e "$i" ]
		then
			echo "$i"
			file_struct_traverse "$i"
		else
			echo "$i does not exist"
		fi
	done
fi














