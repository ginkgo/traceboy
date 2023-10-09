
GRPC_CPP_PLUGIN_PATH ?= `which grpc_cpp_plugin`

SAMEBOYDIR=external/sameboy

CC = gcc
CFLAGS = -Igen -I$(SAMEBOYDIR)/include -O2 -ggdb
LFLAGS = -lprotobuf-c -lzmq -lssl -lcrypto -lm

obj/%.o: src/%.c gen/traceboy.pb-c.h
	gcc -c $(CFLAGS) -o $@ $<
obj/%.o: gen/%.c
	gcc -c $(CFLAGS) -o $@ $<

all: bin/server bin/client bin/verify bin/makevideo

clean:
	rm -rf gen/* bin/* obj/*

gen/traceboy.pb-c.c gen/traceboy.pb-c.h: traceboy.proto
	protoc-c --c_out gen/ traceboy.proto

bin/server: obj/server.o obj/verification.o obj/traceboy.pb-c.o
	gcc $(LFLAGS) obj/server.o obj/verification.o obj/traceboy.pb-c.o  $(SAMEBOYDIR)/lib/libsameboy.a -o bin/server

bin/client: obj/client.o obj/traceboy.pb-c.o
	gcc $(LFLAGS) obj/client.o obj/traceboy.pb-c.o -o bin/client

bin/verify: obj/verify.o obj/verification.o obj/traceboy.pb-c.o
	gcc $(LFLAGS) obj/verify.o obj/verification.o obj/traceboy.pb-c.o $(SAMEBOYDIR)/lib/libsameboy.a -o bin/verify

bin/makevideo: obj/makevideo.o obj/traceboy.pb-c.o
	gcc $(LFLAGS) -lwebp -lwebpmux obj/makevideo.o obj/traceboy.pb-c.o $(SAMEBOYDIR)/lib/libsameboy.a -o bin/makevideo
