#!/usr/bin/env bash

# Script to find traces that don't verify cleanly.

for file in $@
do
	if ! bin/verify $file > /dev/null; then
		echo $file
	fi
done
