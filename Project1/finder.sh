#!/bin/bash

#	echo $1

for var in "$@"
do
	if [ $1 == $var ]; then
		continue
	fi
	echo $var
	sum=0
	for FILE in $1'*';
	do
	#echo $FILE
		files=$(awk '/\.'$var' /' $FILE )
		if [[ -z "$files" ]]; then
    			continue
    		fi
		sum=$(($sum+`awk '/\.'$var' /' $FILE | awk -F. '{print $NF}' | awk '{ print $2 }' | awk '{s+=$1} END {print s}'`))
	done
	echo $sum
	printf "\n"

done
