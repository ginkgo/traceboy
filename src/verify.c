#include <assert.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "verification.h"

int main (int argc, char **argv)
{
	if (argc != 2)
	{
		printf("Usage: %s <traceboy-file>\n", argv[0]);
		return 1;
	}

	uint8_t buffer[1024*1024];
	size_t len;

	{
		FILE *f = fopen(argv[1], "rb");
		if (f)
		{
			len = fread(buffer, 1, sizeof(buffer), f);
		}
		else
		{
			printf("Error opening file\n");
			return 1;
		}
	}

	TracePacket *trace_packet = trace_packet__unpack(NULL, len, buffer);

	printf("Loaded packet(%uB):\n", len);
	printf("  game_rom_crc32: %08x\n", trace_packet->game_rom_crc32);
	printf("  start_state: %u bytes\n", trace_packet->start_state.len);
	printf("  user_inputs: %u bytes\n", trace_packet->user_inputs.len);
	printf("  end_state_crc32: %08x\n", trace_packet->end_state_crc32);

	int error = verify_trace_packet(trace_packet);

	trace_packet__free_unpacked(trace_packet, NULL);

	return error;
}
