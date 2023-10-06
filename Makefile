
GRPC_CPP_PLUGIN_PATH ?= `which grpc_cpp_plugin`

SAMEBOYDIR=external/sameboy

CFLAGS = -Igen -I$(SAMEBOYDIR)/include -O2 -ggdb
LFLAGS = -lprotobuf-c -lzmq -lssl -lcrypto -lm

all: bin/server bin/client bin/verification

gen/traceboy.pb-c.c gen/traceboy.pb-c.h: traceboy.proto
	protoc-c --c_out gen/ traceboy.proto

bin/server: gen/traceboy.pb-c.c gen/traceboy.pb-c.h server/server.c
	gcc $(CFLAGS) $(LFLAGS) server/server.c gen/traceboy.pb-c.c -o bin/server

bin/client: gen/traceboy.pb-c.c gen/traceboy.pb-c.h client/client.c
	gcc $(CFLAGS) $(LFLAGS) client/client.c gen/traceboy.pb-c.c -o bin/client

bin/verification: gen/traceboy.pb-c.c gen/traceboy.pb-c.h verification/verification.c
	gcc $(CFLAGS) $(LFLAGS) verification/verification.c gen/traceboy.pb-c.c $(SAMEBOYDIR)/lib/libsameboy.a -o bin/verification
