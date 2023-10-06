#!/usr/bin/env bash

mkdir -p rom_index

for file in "$@"
do
	output=`rhash --simple -C "$file"`
	hash=${output%% *}

	rm -f rom_index/$hash
	ln -s "$file" rom_index/$hash
done
