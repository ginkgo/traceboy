
GRPC_CPP_PLUGIN_PATH ?= `which grpc_cpp_plugin`

SB_DIR=external/SameBoy
SB_BUILD=$(SB_DIR)/build

CC = gcc
CFLAGS = -Igen -I$(SB_BUILD)/include -O2 -ggdb
LFLAGS = -lprotobuf-c -lzmq -lssl -lcrypto -lm

obj/%.o: src/%.c gen/traceboy.pb-c.h $(SB_BUILD)/include
	gcc -c $(CFLAGS) -o $@ $<
obj/%.o: gen/%.c
	gcc -c $(CFLAGS) -o $@ $<

all: bin/server bin/test_client bin/verify bin/makevideo bin/packet_info

clean:
	rm -rf gen/* bin/* obj/*
	$(MAKE) -C $(SB_DIR) clean

$(SB_BUILD)/include:
	$(MAKE) -C $(SB_DIR) lib sdl

$(SB_BUILD)/lib/libsameboy.a:
	$(MAKE) -C $(SB_DIR) lib sdl

gen/traceboy.pb-c.c gen/traceboy.pb-c.h: traceboy.proto
	protoc-c --c_out gen/ traceboy.proto

bin/server: obj/server.o obj/verification.o obj/traceboy.pb-c.o $(SB_BUILD)/lib/libsameboy.a
	gcc $(LFLAGS) obj/server.o obj/verification.o obj/traceboy.pb-c.o  $(SB_BUILD)/lib/libsameboy.a -o bin/server

bin/test_client: obj/test_client.o obj/traceboy.pb-c.o $(SB_BUILD)/lib/libsameboy.a
	gcc $(LFLAGS) obj/test_client.o obj/traceboy.pb-c.o -o bin/test_client

bin/verify: obj/verify.o obj/verification.o obj/traceboy.pb-c.o $(SB_BUILD)/lib/libsameboy.a
	gcc $(LFLAGS) obj/verify.o obj/verification.o obj/traceboy.pb-c.o $(SB_BUILD)/lib/libsameboy.a -o bin/verify

bin/packet_info: obj/packet_info.o obj/verification.o obj/traceboy.pb-c.o $(SB_BUILD)/lib/libsameboy.a
	gcc $(LFLAGS) obj/packet_info.o obj/verification.o obj/traceboy.pb-c.o $(SB_BUILD)/lib/libsameboy.a -o bin/packet_info

bin/makevideo: obj/makevideo.o obj/traceboy.pb-c.o $(SB_BUILD)/lib/libsameboy.a
	gcc $(LFLAGS) -lwebp -lwebpmux obj/makevideo.o obj/traceboy.pb-c.o $(SB_BUILD)/lib/libsameboy.a -o bin/makevideo
