#include <assert.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>
#include <zmq.h>

#include <traceboy.pb-c.h>
#include "verification.h"

#define NELEMS(arr) (sizeof(arr)/sizeof(arr[0]))

// Function to compute the SHA-256 hash of a buffer
void compute_sha256(uint8_t *buffer, size_t len, uint8_t hash[SHA256_DIGEST_LENGTH]) {
    EVP_MD_CTX *mdctx;
    const EVP_MD *md = EVP_sha256(); // Use SHA-256
    mdctx = EVP_MD_CTX_new();

    EVP_DigestInit(mdctx, md);
    EVP_DigestUpdate(mdctx, buffer, len);
    EVP_DigestFinal(mdctx, hash, NULL);

    EVP_MD_CTX_free(mdctx);
}

// Function to dump a buffer to a file with a hash-based filename
int dump_buffer(uint8_t *buffer, size_t len, int unstable) {
    uint8_t hash[SHA256_DIGEST_LENGTH];
    compute_sha256(buffer, len, hash);

    // Use the hash string as the filename
    char filename[64];
	if (!unstable)
	{
		snprintf(filename, sizeof(filename), "traces/%016llx%016llx.traceboy",
				 ((uint64_t*)hash)[0], ((uint64_t*)hash)[1]);
	}
	else
	{
		snprintf(filename, sizeof(filename), "unstable/%016llx%016llx.traceboy",
				 ((uint64_t*)hash)[0], ((uint64_t*)hash)[1]);
	}

    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }

    size_t elements_written = fwrite(buffer, sizeof(uint8_t), len, file);
    if (elements_written != len) {
        perror("Error writing to file");
        fclose(file);
        return 1;
    }

    fclose(file);

    printf("Data written to file with hash-based filename: %s\n", filename);
    return 0;
}

struct message_buffer
{
	size_t size;
	uint8_t data[];
};

int process_packet(void* buffer)
{
	struct message_buffer *msg_buffer = buffer;
	TracePacket *trace_packet = trace_packet__unpack(NULL, msg_buffer->size, msg_buffer->data);

	if (!trace_packet)
	{
		printf("Received packet(%uB), but failed to decode.\n", msg_buffer->size);
		return 1;
	}

	printf("Received packet(%uB):\n", msg_buffer->size);
	printf("  game_rom_crc32: %08x\n", trace_packet->game_rom_crc32);
	printf("  start_state: %u bytes\n", trace_packet->start_state.len);
	printf("  start_state_crc32: %08x\n",
		   calc_crc32(trace_packet->start_state.len,
					  trace_packet->start_state.data));
	printf("  user_inputs: %u bytes\n", trace_packet->user_inputs.len);
	printf("  end_state_crc32: %08x\n", trace_packet->end_state_crc32);

	int error = verify_trace_packet(trace_packet);
	dump_buffer(msg_buffer->data, msg_buffer->size, error);

	printf("\n");

	trace_packet__free_unpacked(trace_packet, NULL);
	free(buffer);
	return error;
}

int main (void)
{
    //  Socket to talk to clients
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, "tcp://*:1989");

	if (rc != 0)
	{
		printf("Failed to bind to port 1989. (Other server running?)\n");
		return 0;
	}

	thrd_t threads[8] = {0};
	size_t current_thread = 0;

    while (1)
	{
		const size_t MSG_BUFFER_SIZE = 1024*1024 - sizeof(size_t);
		struct message_buffer *buffer = malloc(sizeof(size_t) + MSG_BUFFER_SIZE);

		if (buffer == 0)
		{
			printf("Failed to allocate message buffer\n");
			return 1;
		}

        buffer->size = zmq_recv (responder, buffer->data, MSG_BUFFER_SIZE, 0);
		zmq_send(responder, NULL, 0, 0);

		if (buffer->size < 0)
		{
			printf("Failed to receive packet %d, %d\n", buffer->size, (int)errno);
			continue;
		}

		if (threads[current_thread])
		{
			int res;
			thrd_join(threads[current_thread], &res);
		}

		thrd_create(&threads[current_thread], process_packet, buffer);
		current_thread = (current_thread + 1) % NELEMS(threads);
    }

	for (size_t i = 0; i < NELEMS(threads); ++i)
	{
		if (threads[i])
		{
			int res;
			thrd_join(threads[current_thread], &res);
		}
	}

    return 0;
}
