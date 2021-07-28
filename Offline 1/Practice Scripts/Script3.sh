#!/bin/bash

if [ $# -eq 0 ]; then
	echo "No args"
else 
	for file in $*
	do
		if [ -d $file ]; then echo "File $file is not regular"
		elif [ ! -e $file ]; then echo "File $file does not exist"
		elif [ ! -x $file ]; then echo "File $file is not executable"
		else echo "$file is currently executable"
		     ls -l $file
		     echo "$file permission changing"
		     chmod ugo-x $file
		     ls -l $file
		     echo "File $file is not executable"
		fi
	done
fi
