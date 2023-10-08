#pragma once

#include <traceboy.pb-c.h>

uint32_t calc_crc32(size_t size, const uint8_t *byte);
int verify_trace_packet(const TracePacket *trace_packet);
