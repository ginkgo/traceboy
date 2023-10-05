
GRPC_CPP_PLUGIN_PATH ?= `which grpc_cpp_plugin`

CPPFLAGS += --std=c++23 -Igen `pkg-config --cflags protobuf grpc` -Os
LFLAGS += `pkg-config --libs --static protobuf grpc++`

all: bin/server bin/cxxclient

gen/traceboy.pb-c.c gen/traceboy.pb-c.h: traceboy.proto
	protoc-c --c_out gen/ traceboy.proto

gen/traceboy.pb.cc gen/traceboy.pb.h: traceboy.proto
	protoc --cpp_out gen/ traceboy.proto

gen/traceboy.grpc.pb.cc gen/traceboy.grpc.pb.h: traceboy.proto
	protoc --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) --grpc_out gen/ traceboy.proto

bin/cxxclient: gen/traceboy.pb.cc gen/traceboy.pb.h gen/traceboy.grpc.pb.cc gen/traceboy.grpc.pb.h client/cxxclient.cpp
	g++ $(CPPFLAGS) $(LFLAGS) client/cxxclient.cpp gen/traceboy.pb.cc gen/traceboy.grpc.pb.cc -o bin/cxxclient

bin/server: gen/traceboy.pb.cc gen/traceboy.pb.h gen/traceboy.grpc.pb.cc gen/traceboy.grpc.pb.h server/server.cpp
	g++ $(CPPFLAGS) $(LFLAGS) server/server.cpp gen/traceboy.pb.cc gen/traceboy.grpc.pb.cc -o bin/server
