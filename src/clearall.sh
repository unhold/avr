#!/bin/sh

find -mindepth 1 -maxdepth 2 -type d \
| while read d
do
	cd "$d"
	pwd
	make clean 2> /dev/null
	cd - > /dev/null
done

