#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <traceboy.pb-c.h>

int main (void)
{
    printf ("Connecting to server…\n");
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, "tcp://localhost:1989");

	TracePacket trace_packet;
	trace_packet__init(&trace_packet);

	char *start_state = "Hello world\n";
	uint8_t user_inputs[] = {1,2,3,4,5,6};
	trace_packet.game_rom_crc32 = 0x1337;
	trace_packet.start_state.len = strlen(start_state);
	trace_packet.start_state.data = start_state;
	trace_packet.user_inputs.len = sizeof(user_inputs);
	trace_packet.user_inputs.data = user_inputs;
	trace_packet.end_state_crc32 = 0xdeadbeef;

	size_t packed_size = trace_packet__get_packed_size(&trace_packet);
	uint8_t *packed_msg = malloc(packed_size);
	trace_packet__pack(&trace_packet, packed_msg);

	struct timespec t1,t2;
	clock_gettime(CLOCK_MONOTONIC, &t1);
	zmq_send (requester, packed_msg, packed_size, 0);
	zmq_recv (requester, 0,0,0);
	clock_gettime(CLOCK_MONOTONIC, &t2);

	uint64_t d1 = (t2.tv_nsec - t1.tv_nsec) + 1000000000 * (t2.tv_sec - t1.tv_sec);
	printf("Sent %uB trace packet(%fms)\n", packed_size, d1/1000000);

	free(packed_msg);

    zmq_close (requester);
    zmq_ctx_destroy (context);
    return 0;
}
