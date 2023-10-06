#include <assert.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zmq.h>

#include <traceboy.pb-c.h>

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
int dump_buffer(uint8_t *buffer, size_t len) {
    uint8_t hash[SHA256_DIGEST_LENGTH];
    compute_sha256(buffer, len, hash);

    // Convert the hash to a string representation
    char hash_str[32 + 1];  // +1 for the null terminator
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&hash_str[i * 2], "%02x", hash[i]);
    }

    // Use the hash string as the filename
    char filename[64];
    snprintf(filename, sizeof(filename), "%s.traceboy", hash_str);

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

int main (void)
{
    //  Socket to talk to clients
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, "tcp://*:1989");
    assert (rc == 0);

	const size_t buf_size = 1024*1024;
	uint8_t *buffer = malloc(buf_size);

    while (1) {
        int len = zmq_recv (responder, buffer, buf_size, 0);

		if (len < 0)
		{
			printf("Failed to receive packet %d, %d\n", len, (int)errno);
			continue;
		}

		TracePacket *trace_packet = trace_packet__unpack(NULL, len, buffer);

		if (trace_packet)
		{
			printf("Received packet(%uB):\n", len);
			printf("  game_rom_crc32: %x\n", trace_packet->game_rom_crc32);
			printf("  start_state: %u bytes\n", trace_packet->start_state.len);
			printf("  user_inputs: %u bytes\n", trace_packet->user_inputs.len);
			printf("  end_state_crc32: %x\n", trace_packet->end_state_crc32);

			dump_buffer(buffer, len);
		}
		else
		{
			printf("Received packet(%uB), but failed to decode.\n", len);
		}



		zmq_send(responder, NULL, 0, 0);

		trace_packet__free_unpacked(trace_packet, NULL);
    }

	free(buffer);

    return 0;
}
