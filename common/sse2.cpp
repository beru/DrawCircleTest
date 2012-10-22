#include "sse2.h"

#include <assert.h>
#include <emmintrin.h>
#include <algorithm>

void memset_32(
	void* dest,		// 4 bytes aligned
	uint32_t value,	// 4 bytes value
	size_t count	// number of times to copy value
	)
{
	assert(!((uint32_t)dest & 3));
#if 0
	// •Ï‚ÉÅ“K‰»‚·‚é‚æ‚è‚±‚Á‚¿‚Ì•û‚ª‘¬‚¢B
	uint32_t* pInts = (uint32_t*) dest;
	for (size_t i=0; i<count; ++i) {
		*pInts++ = value;
	}
#else
	uint32_t* pInts = (uint32_t*) dest;
	uint32_t preEdgeBytes = ((uint32_t)pInts) & 15;
	if (preEdgeBytes) {
		size_t preCnt = (16 - preEdgeBytes) / 4;
		preCnt = std::min(preCnt, count);
		for (size_t i=0; i<preCnt; ++i) {
			*pInts++ = value;
		}
		count -= preCnt;
	}
	size_t n16bytes = count / 4;
	__m128i vec = _mm_set1_epi32(value);
	__m128i* pVec = (__m128i*)pInts;
	for (size_t i=0; i<n16bytes; ++i) {
		_mm_store_si128(pVec++, vec);
	}
	pInts = (uint32_t*)pVec;
	size_t nRemain = count % 4;
	for (size_t i=0; i<nRemain; ++i) {
		*pInts++ = value;
	}
#endif
}

