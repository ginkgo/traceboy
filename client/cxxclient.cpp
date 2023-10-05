#include <traceboy.pb.h>
#include <traceboy.grpc.pb.h>

#include <cstdio>
#include <format>

#include <grpcpp/grpcpp.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

template<class... Args>
void print(std::FILE* stream, const std::format_string<Args...> &fmt, Args&&... args)
{
	std::fprintf(stream, std::format(fmt, args...).c_str());
}

template<class... Args>
void print(const std::format_string<Args...> &fmt, Args&&... args)
{
	std::printf(std::format(fmt, args...).c_str());
}

int main(void)
{
	static constexpr uint32_t port = 1989;
	std::string ip = "localhost";
	std::string server_address = std::format("{}:{}", ip, port);

	TracePacket packet;
	packet.set_game_rom_crc32(1337);
	packet.set_start_state("Hello world!\n");
	packet.set_user_inputs("1234567");
	packet.set_end_state_crc32(92837402);


	std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
	std::unique_ptr<TraceDB::Stub> tracedb_stub = TraceDB::NewStub(channel);

	ClientContext context;
	Empty reply;
	Status status = tracedb_stub->Submit(&context, packet, &reply);

	if (status.ok())
	{
		printf("OK\n");
		return 0;
	}
	else
	{
		printf("Error\n");
		return 1;
	}
}
