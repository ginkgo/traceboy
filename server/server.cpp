#include <traceboy.pb.h>
#include <traceboy.grpc.pb.h>

#include <stdio.h>
#include <format>
#include <iostream>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class TraceDBImpl final : public TraceDB::Service
{
	Status Submit(ServerContext* context, const TracePacket* request, ::Empty* response)
	{
		printf("Received trace:\n");
		printf("  game_rom_crc32: %x\n", request->game_rom_crc32());
		printf("  start_state: %d bytes\n", request->start_state().size());
		printf("  user_inputs: %d bytes\n", request->user_inputs().size());
		printf("  end_state_crc32: %x\n", request->end_state_crc32());

		return Status::OK;
	}
};

int main(void)
{
	static constexpr uint32_t port = 1989;
	std::string server_address = std::format("0.0.0.0:{}", port);
	TraceDBImpl service;


	grpc::EnableDefaultHealthCheckService(true);
	grpc::EnableDefaultHealthCheckService(true);
	/* grpc::reflection::InitProtoReflectionServerBuilderPlugin(); */
	ServerBuilder builder;
	// Listen on the given address without any authentication mechanism.
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	// Register "service" as the instance through which we'll communicate with
	// clients. In this case it corresponds to an *synchronous* service.
	builder.RegisterService(&service);
	// Finally assemble the server.
	std::unique_ptr<Server> server(builder.BuildAndStart());
	std::cout << "Server listening on " << server_address << std::endl;

	// Wait for the server to shutdown. Note that some other thread must be
	// responsible for shutting down the server for this call to ever return.
	server->Wait();
}
