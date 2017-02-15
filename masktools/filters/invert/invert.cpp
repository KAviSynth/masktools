#include "invert.h"
#include "../../common/simd.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Invert {

template<int bits_per_pixel>
void invert16_t_c(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight)
{
  uint16_t max_pixel_value_mask = (1 << bits_per_pixel) - 1;
  for (int j = 0; j < nHeight; j++)
  {
    for (int i = 0; i < nWidth; i++)
      reinterpret_cast<uint16_t *>(pDst)[i] = max_pixel_value_mask - reinterpret_cast<uint16_t *>(pDst)[i];
    pDst += nDstPitch;
  }
}

void invert_c(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight)
{
   for ( int j = 0; j < nHeight; j++ )
   {
      for ( int i = 0; i < nWidth; i++ )
         pDst[i] = 255 - pDst[i];
      pDst += nDstPitch;
   }
}

void invert_sse2(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight)
{
    auto fff = _mm_set1_epi32(0xFFFFFFFF);
    for ( int j = 0; j < nHeight; j++ ) {
        for ( int i = 0; i < nWidth; i+=16 ) {
            auto src = simd_load_si128<MemoryMode::SSE2_ALIGNED>(pDst+i);
            auto result = _mm_xor_si128(src, fff);
            simd_store_si128<MemoryMode::SSE2_ALIGNED>(pDst+i, result);
        }
        pDst += nDstPitch;
    }
}

template<int bits_per_pixel>
void invert16_t_sse2(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight)
{
  nWidth *= 2; // really rowsize: width * sizeof(uint16)

  int max_pixel_value_mask = (1 << bits_per_pixel) - 1;
  auto fff = _mm_set1_epi16(max_pixel_value_mask);
  for (int j = 0; j < nHeight; j++) {
    for (int i = 0; i < nWidth; i += 16) {
      auto src = simd_load_si128<MemoryMode::SSE2_ALIGNED>(pDst + i);
      auto result = _mm_xor_si128(src, fff);
      simd_store_si128<MemoryMode::SSE2_ALIGNED>(pDst + i, result);
    }
    pDst += nDstPitch;
  }
}

template void invert16_t_c<10>(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight);
template void invert16_t_c<12>(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight);
template void invert16_t_c<14>(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight);
template void invert16_t_c<16>(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight);
template void invert16_t_sse2<10>(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight);
template void invert16_t_sse2<12>(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight);
template void invert16_t_sse2<14>(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight);
template void invert16_t_sse2<16>(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight);


Processor *invert10_c = &invert16_t_c<10>;
Processor *invert12_c = &invert16_t_c<12>;
Processor *invert14_c = &invert16_t_c<14>;
Processor *invert16_c = &invert16_t_c<16>;

Processor *invert10_sse2 = &invert16_t_sse2<10>;
Processor *invert12_sse2 = &invert16_t_sse2<12>;
Processor *invert14_sse2 = &invert16_t_sse2<14>;
Processor *invert16_sse2 = &invert16_t_sse2<16>;

} } } }
