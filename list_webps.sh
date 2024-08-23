#!/usr/bin/env bash
cd webps

for webp in `ls *.webp`
do
	echo "    '$webp',"
done
