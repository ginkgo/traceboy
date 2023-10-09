#include <assert.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>

#include <webp/encode.h>
#include <webp/mux.h>

#include <sameboy/gb.h>
#include <traceboy.pb-c.h>

#define NELEMS(a) (sizeof(a)/sizeof(a[0]))

#define W 160
#define H 144

static thread_local GB_vblank_type_t vblank_type;
static void vblank(GB_gameboy_t *gb, GB_vblank_type_t type)
{
	vblank_type = type;
}

static uint32_t rgb_encode_callback(GB_gameboy_t *gb, uint8_t r, uint8_t g, uint8_t b)
{
	uint8_t a = 255;
	return ((uint32_t)r << 0) | ((uint32_t)g << 8) | ((uint32_t)b << 16) | ((uint32_t)a << 24);
}

int convert_trace_packet(const TracePacket *trace_packet, const char *outfile_name)
{
	int error;

	GB_gameboy_t *gb = GB_init(GB_alloc(), GB_MODEL_DMG_B);
	if (!gb)
	{
		printf("Failed to allocate and init GB context\n");
		return 1;
	}

	GB_set_vblank_callback(gb, (GB_vblank_callback_t) vblank);
	GB_set_rgb_encode_callback(gb, (GB_rgb_encode_callback_t) rgb_encode_callback);

	{
		error = GB_load_boot_rom(gb, "external/sameboy/BootROMs/dmg_boot.bin");
		if (error)
		{
			printf("Failed to load boot ROM\n");
			goto end;
		}

		char rom_path[32];
		int len = snprintf(rom_path, sizeof(rom_path),
						   "rom_index/%08x", trace_packet->game_rom_crc32);
		assert(len == 18);

		error = GB_load_rom(gb, rom_path);
		if (error)
		{
			printf("Failed to load packet ROM %s\n", rom_path);
			goto end;
		}
		else
		{
			char rom_name[32];
			GB_get_rom_title(gb, rom_name);
			printf("Successfully loaded ROM '%s' for CRC %08x\n",
				   rom_name, trace_packet->game_rom_crc32);
		}

		error = GB_load_state_from_buffer(gb, trace_packet->start_state.data,
										  trace_packet->start_state.len);
	}

	uint32_t pixels[W * H];
	GB_set_palette(gb, &GB_PALETTE_DMG);
	GB_set_pixels_output(gb, pixels);

	struct timespec t1,t2;
	clock_gettime(CLOCK_MONOTONIC, &t1);

	WebPConfig config = {0};
	WebPConfigLosslessPreset(&config, 6);
	config.segments = 2;
	config.alpha_compression = 1;
	config.pass = 1;

	if (!WebPValidateConfig(&config))
	{
		printf("Something went wrong during WEBP config validation\n");
		error = 1;
		goto end;
	}

	struct WebPAnimEncoderOptions enc_options = {0};
	WebPAnimEncoderOptionsInit(&enc_options);

	struct WebPAnimEncoder* enc = WebPAnimEncoderNew(W, H, &enc_options);

	struct WebPPicture frame;
	WebPPictureInit(&frame);
	frame.use_argb = 1;
	frame.width = W;
	frame.height = H;
	frame.argb_stride = W;

	/* Simulate user inputs */
	size_t iterations = 0;
	for (size_t i = 0; i < trace_packet->user_inputs.len; ++i)
	{
		GB_set_key_mask(gb, trace_packet->user_inputs.data[i]);

		do {
			GB_run_frame(gb);

			if (iterations++ > trace_packet->user_inputs.len * 3)
			{
				printf("Hang during evaluation!\n");
				error = 1;
				goto end;
			}
		} while (vblank_type != GB_VBLANK_TYPE_NORMAL_FRAME);

		WebPPictureImportRGBX(&frame, (uint8_t*)pixels, W * sizeof(uint32_t));
		WebPAnimEncoderAdd(enc, &frame, (int)(i * 16.666 + 0.5), &config);
	}

	WebPPictureFree(&frame);

	WebPData webp_data;
	WebPAnimEncoderAssemble(enc, &webp_data);

	FILE *file = fopen(outfile_name, "wb");
	if (file == NULL)
	{
		printf("Error opening file '%s' for writing.\n", outfile_name);
		error = 1;
		goto end2;
	}

	size_t bytes_written = fwrite(webp_data.bytes, sizeof(uint8_t), webp_data.size, file);
	if (bytes_written != webp_data.size)
	{
		printf("Failed to write all bytes: (%d/%d) written\n", bytes_written, webp_data.size);
		error = 1;
	}

end2:

	WebPAnimEncoderDelete(enc);

	clock_gettime(CLOCK_MONOTONIC, &t2);

	{
		double secs = (t2.tv_sec - t1.tv_sec) + (t2.tv_nsec - t1.tv_nsec) / 1000000000.0;
		printf("Simulation+encoding time %.2fs (%.2fx real-time)\n", secs,
			   (trace_packet->user_inputs.len / 60.)/secs);
	}

end:

	GB_free(gb);
	return error;
}


int main (int argc, char **argv)
{
	if (argc != 3)
	{
		printf("Usage: %s <traceboy-file> <webp-file>\n", argv[0]);
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

	int error = convert_trace_packet(trace_packet, argv[2]);

	trace_packet__free_unpacked(trace_packet, NULL);

	return error;
}
