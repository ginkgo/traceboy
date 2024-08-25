# Scripts

This directory contains various utility scripts.

## `find_successor.py`

Given a `.traceboy` packet file try to find the packet that would directly follow it.
This is useful when investigating why a packet failed to verify since it allows a direct comparison of the various
data sections.

## `gen_rom_index.sh`

Use this to generate the `rom_index` directory needed by the verification tool to find the ROM used in a packet.

## `make_webps.sh`

Use this to generate WebP video clips for the image_grid demo page.
More on this in `image_grid/README.md`

## `filter_traces.sh`

Use this to find trace packet files that don't verify cleanly.
Calling this might be necessary after something has changed in the verification setup.
