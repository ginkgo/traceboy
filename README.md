# TraceBoy

This is a proof-of-concept for a packet-based trace system for recording and collecting Game Boy playback information.

The general idea is to store a snippet of a gameplay session as a packet of data containing the initial state (as a BESS
save state) along with the user button inputs for the next couple of frames and the final game state checksum.

```
message TracePacket {
	uint32 game_rom_crc32 =1;

	bytes start_state = 2;
	bytes user_inputs = 3;

	uint32 end_state_crc32 = 4;
}
```
Instead of sending the game ROM along we simply add the ROM's CRC32. This can then be used by the receiver of the
packet to find the correct ROM in a database of available ROMs.
This is enough information to rerun and verify the trace and should only take a couple KB for many seconds of gameplay.
Files are usually around 50KB and generally compress really well to much much less.

The idea would then be to have emulators build end send these packets to a database server during gameplay so a large
database of sample gameplay data can be created.
Using the start and end state CRCs it should then be possible to connect individual trace packets into longer sessions
of gameplay data.

The main usecase for something like this would be for collecting large amounts of easily reproducable trace data for
machine learning, but I can also imagine this being useful as a data format for tool-assisted speedruns or as part of
a rewind system.

It can also be used to generate [cute video walls of various gameplay snippets](https://ginkgo.github.io/traceboy/). :)

In a perfect world I'd like to see tracing functionality added to popular emulators so a large number of users could (optionally)
send their gameplay data to a central collection server.
The collected data could be made publically available so people could use it for training.

## Open issues

The current system sort of works, but there's still cases where trace packets fail go give deterministic results.
I'm not completely sure why that is, but it's more common in some games than others.
It seems like audio memory is most likely to diverge somehow.

The way packet creation was added to SameBoy was very haphazard. A proper implementation should clean this up significantly.

## Build requirements

This require everything the SDL frontend and library needs for building:
 * clang (Recommended; required for macOS) or GCC
 * make
 * SDL frontend: libsdl2
 * [rgbds](https://github.com/gbdev/rgbds/releases/), for boot ROM compilation
 * [cppp](https://github.com/BR903/cppp), for cleaning up headers when compiling SameBoy as a library

On top of that we also need the following extra dependencies:
 * [ZeroMQ](https://zeromq.org/)'s libzmq
 * [Protobuf](https://protobuf.dev/)

## Compilation

Make sure to check the `external/SameBoy` submodule:

```
$ git submodules init
$ git submodules update
```

This will check out a patched version of SameBoy.

Build everything with `make`

```
$ make -j8
```

This should build both SameBoy as well as the tracing tools.

## Usage

Before you can start collecting traces you need to first generate an index of Game Boy ROMs using `gen_rom_index.sh`.

If you have a directory with GB ROMs you can generate an index with:

```
$ gen_rom_index.sh $ROMDIR/*.gb
```

This will insert a symbolic link for each given ROM into the `rom_index` directory. The link's filename is the ROM's
CRC32. This will later be used to find the correct ROM for a given trace packet.

To collect traces first start the server in its own shell:

```
$ bin/server
```

The server will start listening for trace packets on port `1989`.

Then launch the patched SameBoy SDL app and play some game.

```
$ external/SameBoy/build/bin/SDL/sameboy 
```

You should now see a trace packet sent every couple seconds or so. The server will replay the trace to verify if the
final CRC32 reported in the packet matches its own replay. If so it will store the trace in the `traces/` directory for
later use.

If the replay doesn't match it will print an error message with further checksum information and store the trace in the
`unstable/` directory for further inspection.

## Copyright

The copyright to SameBoy belongs to Lior Halphon and is available under the Expat License.

Everything else is copyright of Thomas Weber available under the MIT license.
