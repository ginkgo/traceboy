#!/usr/bin/env bash

for file in $@
do
	if ! bin/verify $file > /dev/null; then
		echo $file
	fi
done
