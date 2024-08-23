#!/usr/bin/env bash

mkdir -p webps

for path in `ls traces/*.traceboy`
do
	file=$(basename $path)
	webpfile="webps/${file%.traceboy}.webp"

	if [ ! -f $webpfile ]
	then
		echo "$webpfile"
		bin/makevideo "$path" "$webpfile"
	fi
done
