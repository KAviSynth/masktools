#include "average.h"
#include "../../../common/simd.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Support  { namespace Average {

void average_c(Byte *pDst, ptrdiff_t dst_pitch, const Byte *pSrc, ptrdiff_t src_pitch, int width, int height)
{
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pDst[x] = (int(pDst[x]) + pSrc[x] + 1) >> 1;
        }
        pDst += dst_pitch;
        pSrc += src_pitch;
    }
}

template<MemoryMode mem_mode>
static void average_sse2_t(Byte *pDst, ptrdiff_t dst_pitch, const Byte *pSrc, ptrdiff_t src_pitch, int width, int height)
{
    int mod16_width = (width / 16) * 16;
    auto pDst2 = pDst;
    auto pSrc2 = pSrc;

    for ( int j = 0; j < height; ++j ) {
        for ( int i = 0; i < mod16_width; i+=16 ) {
            _mm_prefetch(reinterpret_cast<const char*>(pDst)+i+256, _MM_HINT_T0);
            _mm_prefetch(reinterpret_cast<const char*>(pSrc)+i+256, _MM_HINT_T0);

            auto dst = simd_load_si128<mem_mode>(pDst+i);
            auto src = simd_load_si128<mem_mode>(pSrc+i);

            auto result  = _mm_avg_epu8(dst, src);

            simd_store_si128<mem_mode>(pDst+i, result);
        }
        pDst += dst_pitch;
        pSrc += src_pitch;
    }

    if (width > mod16_width) {
        average_c(pDst2 + mod16_width, dst_pitch, pSrc2 + mod16_width, src_pitch, width - mod16_width, height);
    }
}

void average32_c(Byte *pDst, ptrdiff_t dst_pitch, const Byte *pSrc, ptrdiff_t src_pitch, int width, int height)
{
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      reinterpret_cast<Float *>(pDst)[x] = (reinterpret_cast<Float *>(pDst)[x] + reinterpret_cast<const Float *>(pSrc)[x]) * 0.5f;
    }
    pDst += dst_pitch;
    pSrc += src_pitch;
  }
}

template<MemoryMode mem_mode>
static void average32_sse2_t(Byte *pDst, ptrdiff_t dst_pitch, const Byte *pSrc, ptrdiff_t src_pitch, int width, int height)
{
  width *= sizeof(Float);

  int mod16_width = (width / 16) * 16;
  auto pDst2 = pDst;
  auto pSrc2 = pSrc;
  auto vDiv2 = _mm_set1_ps(0.5f);

  for (int j = 0; j < height; ++j) {
    for (int i = 0; i < mod16_width; i += 16) {
      _mm_prefetch(reinterpret_cast<const char*>(pDst) + i + 256, _MM_HINT_T0);
      _mm_prefetch(reinterpret_cast<const char*>(pSrc) + i + 256, _MM_HINT_T0);

      auto dst = simd_load_ps<mem_mode>(pDst + i);
      auto src = simd_load_ps<mem_mode>(pSrc + i);

      auto result = _mm_mul_ps(_mm_add_ps(dst, src), vDiv2);

      simd_store_ps<mem_mode>(pDst + i, result);
    }
    pDst += dst_pitch;
    pSrc += src_pitch;
  }

  if (width > mod16_width) {
    average32_c(pDst2 + mod16_width, dst_pitch, pSrc2 + mod16_width, src_pitch, (width - mod16_width) / sizeof(Float), height);
  }
}

Processor *average_sse2 = &average_sse2_t<MemoryMode::SSE2_UNALIGNED>;
Processor *average_asse2 = &average_sse2_t<MemoryMode::SSE2_ALIGNED>;

Processor32 *average32_sse2 = &average32_sse2_t<MemoryMode::SSE2_UNALIGNED>;
Processor32 *average32_asse2 = &average32_sse2_t<MemoryMode::SSE2_ALIGNED>;

} } } } }