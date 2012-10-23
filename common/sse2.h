#pragma once

void memset_32(
	void* dest,		// 4 bytes aligned
	uint32_t value,	// 4 bytes value
	size_t count	// number of times to copy value
);

void BlendFill_SSE2(
	void* dest,		// 4 bytes aligned destination buffer
	uint32_t argb,	// 4 bytes argb color
	size_t count	// number of times to set pixels
);
