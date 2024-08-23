#!/usr/bin/env bash

mkdir -p image_grid/webps

rm -rf image_grid/images.txt
touch image_grid/images.txt

for path in `ls traces/*.traceboy`
do
	file=$(basename $path)
	webpfile="${file%.traceboy}.webp"
	webppath="image_grid/webps/${webpfile}"
	
	if [ ! -f $webppath ]
	then
		echo "$webppath"
		bin/makevideo "$path" "$webppath"
	fi
	
	echo "webps/${webpfile}" >> image_grid/images.txt
	
done
