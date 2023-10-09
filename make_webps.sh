#!/usr/bin/env bash

mkdir -p webps

for path in `ls traces/*.traceboy`
do
	file=$(basename $path)
	bin/makevideo "$path" "webps/${file%.traceboy}.webp"
done
