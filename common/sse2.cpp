#include "sse2.h"

#include <assert.h>
#include <emmintrin.h>
#include <algorithm>

#include "../graphics/graphics.h"

void memset_32(
	void* dest,		// 4 bytes aligned
	uint32_t value,	// 4 bytes value
	size_t count	// number of times to copy value
	)
{
	assert(!((uint32_t)dest & 3));
#if 0
	// �ςɍœK�������肱�����̕��������B
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

void BlendFill_SSE2(
	void* dest,		// 4 bytes aligned destination buffer
	uint32_t argb,	// 4 bytes argb color
	size_t count	// number of times to set pixels
	)
{
	assert(!((uint32_t)dest & 3));
	uint32_t* pInts = (uint32_t*) dest;
	uint32_t preEdgeBytes = ((uint32_t)pInts) & 15;
	if (preEdgeBytes) {
		size_t preCnt = (16 - preEdgeBytes) / 4;
		preCnt = std::min(preCnt, count);
		for (size_t i=0; i<preCnt; ++i) {
			*pInts = Graphics::BlendColor(argb, *pInts);
			pInts++;
		}
		count -= preCnt;
	}
	size_t vecCnt = count / 4;
	if (vecCnt) {
		__m128i srcVec = _mm_set1_epi32(argb);
		__m128i alphaVec = _mm_set1_epi16(((argb>>24)+1));
		__m128i srcVec0 = _mm_unpacklo_epi8(srcVec, _mm_setzero_si128());
		__m128i srcVec1 = _mm_unpackhi_epi8(srcVec, _mm_setzero_si128());
		__m128i* pDestVec = (__m128i*)pInts;
		for (size_t i=0; i<vecCnt; ++i) {
			__m128i destVec = _mm_load_si128(pDestVec);
			__m128i destVec0 = _mm_unpacklo_epi8(destVec, _mm_setzero_si128());
			__m128i destVec1 = _mm_unpackhi_epi8(destVec, _mm_setzero_si128());
			__m128i diffVec0 = _mm_sub_epi16(srcVec0, destVec0);
			__m128i diffVec1 = _mm_sub_epi16(srcVec1, destVec1);
			diffVec0 = _mm_mullo_epi16(alphaVec, diffVec0);
			diffVec1 = _mm_mullo_epi16(alphaVec, diffVec1);
			destVec0 = _mm_slli_si128(destVec0, 1);
			destVec1 = _mm_slli_si128(destVec1, 1);
			destVec0 = _mm_add_epi16(destVec0, diffVec0);
			destVec1 = _mm_add_epi16(destVec1, diffVec1);
			destVec0 = _mm_srli_epi16(destVec0, 8);
			destVec1 = _mm_srli_epi16(destVec1, 8);
			destVec0 = _mm_packus_epi16(destVec0, destVec1);
			_mm_store_si128(pDestVec, destVec0);
			++pDestVec;
		}
		pInts = (uint32_t*) pDestVec;
	}
	count = count % 4;
	for (size_t i=0; i<count; ++i) {
		*pInts = Graphics::BlendColor(argb, *pInts);
		++pInts;
	}
}