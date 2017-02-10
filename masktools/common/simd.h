#ifndef __Mt_SIMD_H__
#define __Mt_SIMD_H__

#include <emmintrin.h>
#include "common.h"

namespace Filtering {

//because ICC is smart enough on its own and force inlining actually makes it slower
#ifdef __INTEL_COMPILER
#define MT_FORCEINLINE inline
#else
#define MT_FORCEINLINE __forceinline
#endif

#define USE_MOVPS

enum class MemoryMode {
    SSE2_UNALIGNED,
    SSE2_ALIGNED
};


template<MemoryMode mem_mode, typename T>
static MT_FORCEINLINE __m128i simd_load_si128(const T* ptr) {
#ifdef USE_MOVPS
    if (mem_mode == MemoryMode::SSE2_ALIGNED) {
        return _mm_castps_si128(_mm_load_ps(reinterpret_cast<const float*>(ptr)));
    } else {
        return _mm_castps_si128(_mm_loadu_ps(reinterpret_cast<const float*>(ptr)));
    }
#else
    if (mem_mode == MemoryMode::SSE2_ALIGNED) {
        return _mm_load_si128(reinterpret_cast<const __m128i*>(ptr));
    } else {
        return _mm_loadu_si128(reinterpret_cast<const __m128i*>(ptr));
    }
#endif
}


template<MemoryMode mem_mode, typename T>
static MT_FORCEINLINE void simd_store_si128(T *ptr, __m128i value) {
#ifdef USE_MOVPS
    if (mem_mode == MemoryMode::SSE2_ALIGNED) {
        _mm_store_ps(reinterpret_cast<float*>(ptr), _mm_castsi128_ps(value));
    } else {
        _mm_storeu_ps(reinterpret_cast<float*>(ptr), _mm_castsi128_ps(value));
    }
#else
    if (mem_mode == MemoryMode::SSE2_ALIGNED) {
        _mm_store_si128(reinterpret_cast<__m128i*>(ptr), value);
    } else {   
        _mm_storeu_si128(reinterpret_cast<__m128i*>(ptr), value);
    }
#endif
}


static MT_FORCEINLINE int simd_bit_scan_forward(int value) {
#ifdef __INTEL_COMPILER
    return _bit_scan_forward(value);
#else
    unsigned long index;
    _BitScanForward(&index, value);
    return index;
#endif
}



enum class Border {
    Left,
    Right,
    None
};

#pragma warning(disable: 4309)

template<Border border_mode, MemoryMode mem_mode>
static MT_FORCEINLINE __m128i load_one_to_left(const Byte *ptr) {
    if (border_mode == Border::Left) {
        auto mask_left = _mm_setr_epi8(0xFF, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00);
        auto val = simd_load_si128<mem_mode>(ptr);
        return _mm_or_si128(_mm_slli_si128(val, 1), _mm_and_si128(val, mask_left));
    } else {
        return simd_load_si128<MemoryMode::SSE2_UNALIGNED>(ptr - 1);
    }
}

template<Border border_mode, MemoryMode mem_mode>
static MT_FORCEINLINE __m128i load_one_to_right(const Byte *ptr) {
    if (border_mode == Border::Right) {
        auto mask_right = _mm_setr_epi8(00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 0xFF);
        auto val = simd_load_si128<mem_mode>(ptr);
        return _mm_or_si128(_mm_srli_si128(val, 1), _mm_and_si128(val, mask_right));
    } else {
        return simd_load_si128<MemoryMode::SSE2_UNALIGNED>(ptr + 1);
    }
}

#pragma warning(default: 4309)

static MT_FORCEINLINE __m128i simd_movehl_si128(const __m128i &a, const __m128i &b) {
    return _mm_castps_si128(_mm_movehl_ps(_mm_castsi128_ps(a), _mm_castsi128_ps(b)));
}

static MT_FORCEINLINE __m128i threshold_sse2(const __m128i &value, const __m128i &lowThresh, const __m128i &highThresh, const __m128i &v128) {
    auto sat = _mm_sub_epi8(value, v128);
    auto low = _mm_cmpgt_epi8(sat, lowThresh);
    auto high = _mm_cmpgt_epi8(sat, highThresh);
    auto result = _mm_and_si128(value, low);
    return _mm_or_si128(result, high);
}

template<CpuFlags flags>
static MT_FORCEINLINE __m128i simd_blend_epi8(__m128i const &selector, __m128i const &a, __m128i const &b) {
    if (flags >= CPU_SSE4_1) {
        return _mm_blendv_epi8 (b, a, selector);
    } else {
        return _mm_or_si128(_mm_and_si128(selector, a), _mm_andnot_si128(selector, b));
    }
}

template<CpuFlags flags>
static MT_FORCEINLINE __m128i simd_mullo_epi32(__m128i &a, __m128i &b) {
    if (flags >= CPU_SSE4_1) {
        return _mm_mullo_epi32(a, b);
    } else {
        auto a13    = _mm_shuffle_epi32(a, 0xF5);          // (-,a3,-,a1)
        auto b13    = _mm_shuffle_epi32(b, 0xF5);          // (-,b3,-,b1)
        auto prod02 = _mm_mul_epu32(a, b);                 // (-,a2*b2,-,a0*b0)
        auto prod13 = _mm_mul_epu32(a13, b13);             // (-,a3*b3,-,a1*b1)
        auto prod01 = _mm_unpacklo_epi32(prod02,prod13);   // (-,-,a1*b1,a0*b0) 
        auto prod23 = _mm_unpackhi_epi32(prod02,prod13);   // (-,-,a3*b3,a2*b2) 
        return _mm_unpacklo_epi64(prod01,prod23);   // (ab3,ab2,ab1,ab0)
    }
}

// sse2 replacement of _mm_mullo_epi32 in SSE4.1
// another way for do mullo for SSE2, actually not used, there is simd_mullo_epi32
// use it after speed test, may have too much overhead and C is faster
static MT_FORCEINLINE __m128i _MM_MULLO_EPI32(const __m128i &a, const __m128i &b)
{
  // for SSE 4.1: return _mm_mullo_epi32(a, b);
  __m128i tmp1 = _mm_mul_epu32(a, b); // mul 2,0
  __m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(a, 4), _mm_srli_si128(b, 4)); // mul 3,1
                                                                            // shuffle results to [63..0] and pack. a2->a1, a0->a0
  return _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE(0, 0, 2, 0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE(0, 0, 2, 0)));
}

// fake _mm_packus_epi32 (orig is SSE4.1 only)
static MT_FORCEINLINE __m128i _MM_PACKUS_EPI32(__m128i a, __m128i b)
{
  a = _mm_slli_epi32(a, 16);
  a = _mm_srai_epi32(a, 16);
  b = _mm_slli_epi32(b, 16);
  b = _mm_srai_epi32(b, 16);
  a = _mm_packs_epi32(a, b);
  return a;
}

// non-existant in simd
static MT_FORCEINLINE __m128i _MM_CMPLE_EPU16(__m128i x, __m128i y)
{
  // Returns 0xFFFF where x <= y:
  return _mm_cmpeq_epi16(_mm_subs_epu16(x, y), _mm_setzero_si128());
}


template<CpuFlags flags>
static MT_FORCEINLINE __m128i simd_packus_epi32(__m128i &a, __m128i &b) {
  if (flags >= CPU_SSE4_1) {
    return _mm_packus_epi32(a, b);
  }
  else {
    return _MM_PACKUS_EPI32(a, b);
  }
}


static MT_FORCEINLINE __m128i read_word_stacked_simd(const Byte *pMsb, const Byte *pLsb, int x) {
    auto msb = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(pMsb+x));
    auto lsb = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(pLsb+x));
    return _mm_unpacklo_epi8(lsb, msb);
}

static MT_FORCEINLINE void write_word_stacked_simd(Byte *pMsb, Byte *pLsb, int x, const __m128i &value, const __m128i &ff, const __m128i &zero) {
    auto result_lsb = _mm_and_si128(value, ff);
    auto result_msb = _mm_srli_epi16(value, 8);

    result_lsb = _mm_packus_epi16(result_lsb, zero);
    result_msb = _mm_packus_epi16(result_msb, zero);

    _mm_storel_epi64(reinterpret_cast<__m128i*>(pMsb+x), result_msb);
    _mm_storel_epi64(reinterpret_cast<__m128i*>(pLsb+x), result_lsb);
}

}

#endif __Mt_SIMD_H__
