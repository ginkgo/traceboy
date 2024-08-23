#!/usr/bin/env bash

# Script to generate the CRC32-based ROM index used by the verification script
# to find the ROM corresponding to a trace file.
#
# Usage: gen_rom_index.sh <list-of-roms>
#
# Pass a list of ROMs to this script, for instance using `find`:
#
# $ gen_rom_index.sh `find <your-rom-dir> -iname '*.gb'`
#
# This will then create a symbolic link to that rom in the rom_index/ directory.

mkdir -p rom_index

for file in "$@"
do
	output=`rhash --simple -C "$file"`
	hash=${output%% *}

	rm -f rom_index/$hash
	ln -s "$file" rom_index/$hash
done
