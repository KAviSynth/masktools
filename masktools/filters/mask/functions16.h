#ifndef __Mt_MaskFunctions16_H__
#define __Mt_MaskFunctions16_H__

#include "../../../common/utils/utils.h"
#include "../../common/simd.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Mask {

typedef Word (Operator)(Word, Word, Word, Word, Word, Word, Word, Word, Word, const Short matrix[10], int nLowThreshold, int nHighThreshold);
typedef void (ProcessLineSse2)(Byte *pDst, const Byte *pSrcp, const Byte *pSrc, const Byte *pSrcn, const Short matrix[10], const __m128i &lowThresh, const __m128i &highThresh, int width);

template<Operator op, class T>
void generic16_c(Word *pDst, ptrdiff_t nDstPitch, const Word *pSrc, ptrdiff_t nSrcPitch, T &thresholds, const Short matrix[10], int nWidth, int nHeight)
{
  nDstPitch /= sizeof(uint16_t);
  nSrcPitch /= sizeof(uint16_t);
  const Word *pSrcp = pSrc - nSrcPitch;
  const Word *pSrcn = pSrc + nSrcPitch;

   /* top-left */
   pDst[0] = op(pSrc[0], pSrc[0], pSrc[1], pSrc[0], pSrc[0], pSrc[1], pSrcn[0], pSrcn[0], pSrcn[1], matrix, thresholds.min(0), thresholds.max(0));

   /* top */
   for ( int x = 1; x < nWidth - 1; x++ )
      pDst[x] = op(pSrc[x-1], pSrc[x], pSrc[x+1], pSrc[x-1], pSrc[x], pSrc[x+1], pSrcn[x-1], pSrcn[x], pSrcn[x+1], matrix, thresholds.min(x), thresholds.max(x));

   /* top-right */
   pDst[nWidth-1] = op(pSrc[nWidth-2], pSrc[nWidth-1], pSrc[nWidth-1], pSrc[nWidth-2], pSrc[nWidth-1], pSrc[nWidth-1], pSrcn[nWidth-2], pSrcn[nWidth-1], pSrcn[nWidth-1], matrix, thresholds.min(nWidth-1), thresholds.max(nWidth-1));

   pDst  += nDstPitch;
   pSrcp += nSrcPitch;
   pSrc  += nSrcPitch;
   pSrcn += nSrcPitch;

   thresholds.nextRow();

   for ( int y = 1; y < nHeight - 1; y++ )
   {
      /* left */
      pDst[0] = op(pSrcp[0], pSrcp[0], pSrcp[1], pSrc[0], pSrc[0], pSrc[1], pSrcn[0], pSrcn[0], pSrcn[1], matrix, thresholds.min(0), thresholds.max(0));

      /* center */
      for ( int x = 1; x < nWidth - 1; x++ )
         pDst[x] = op(pSrcp[x-1], pSrcp[x], pSrcp[x+1], pSrc[x-1], pSrc[x], pSrc[x+1], pSrcn[x-1], pSrcn[x], pSrcn[x+1], matrix, thresholds.min(x), thresholds.max(x));

      /* right */
      pDst[nWidth-1] = op(pSrcp[nWidth-2], pSrcp[nWidth-1], pSrcp[nWidth-1], pSrc[nWidth-2], pSrc[nWidth-1], pSrc[nWidth-1], pSrcn[nWidth-2], pSrcn[nWidth-1], pSrcn[nWidth-1], matrix, thresholds.min(nWidth-1), thresholds.max(nWidth-1));

      pDst  += nDstPitch;
      pSrcp += nSrcPitch;
      pSrc  += nSrcPitch;
      pSrcn += nSrcPitch;

      thresholds.nextRow();
   }

   /* bottom-left */
   pDst[0] = op(pSrcp[0], pSrcp[0], pSrcp[1], pSrc[0], pSrc[0], pSrc[1], pSrc[0], pSrc[0], pSrc[1], matrix, thresholds.min(0), thresholds.max(0));

   /* bottom */
   for ( int x = 1; x < nWidth - 1; x++ )
      pDst[x] = op(pSrcp[x-1], pSrcp[x], pSrcp[x+1], pSrc[x-1], pSrc[x], pSrc[x+1], pSrc[x-1], pSrc[x], pSrc[x+1], matrix, thresholds.min(x), thresholds.max(x));

   /* bottom-right */
   pDst[nWidth-1] = op(pSrcp[nWidth-2], pSrcp[nWidth-1], pSrcp[nWidth-1], pSrc[nWidth-2], pSrc[nWidth-1], pSrc[nWidth-1], pSrc[nWidth-2], pSrc[nWidth-1], pSrc[nWidth-1], matrix, thresholds.min(nWidth-1), thresholds.max(nWidth-1));
}

// todo bitdepth template
template<int bits_per_pixel, ProcessLineSse2 process_line_left, ProcessLineSse2 process_line, ProcessLineSse2 process_line_right>
void generic16_sse2(Word *pDst0, ptrdiff_t nDstPitch, const Word *pSrc0, ptrdiff_t nSrcPitch, const Short matrix[10], int nLowThreshold, int nHighThreshold, int nWidth, int nHeight) {
    Byte *pDst = reinterpret_cast<Byte *>(pDst0);
    const Byte *pSrc = reinterpret_cast<const Byte *>(pSrc0);

    nWidth *= sizeof(uint16_t); // byte size for sse

    const Byte *pSrcp = pSrc - nSrcPitch;
    const Byte *pSrcn = pSrc + nSrcPitch;

#pragma warning(disable: 4309)
    auto vHalf = _mm_set1_epi16(1 << (bits_per_pixel - 1));
#pragma warning(default: 4309)
    auto low_thr_v = _mm_set1_epi16(Word(nLowThreshold)); // for faster sse2 signed comparison
    low_thr_v = _mm_sub_epi16(low_thr_v, vHalf);
    auto high_thr_v = _mm_set1_epi16(Word(nHighThreshold));
    high_thr_v = _mm_sub_epi16(high_thr_v, vHalf);

    int sse2_width = (nWidth - sizeof(uint16_t) - 16) / 16 * 16 + 16;
    /* top-left */
    process_line_left(pDst, pSrc, pSrc, pSrcn, matrix, low_thr_v, high_thr_v, 16);
    /* top */
    process_line(pDst + 16, pSrc+16, pSrc+16, pSrcn+16, matrix, low_thr_v, high_thr_v, sse2_width - 16);

    /* top-right */
    process_line_right(pDst + nWidth - 16, pSrc + nWidth - 16, pSrc + nWidth - 16, pSrcn + nWidth - 16, matrix, low_thr_v, high_thr_v, 16);

    pDst  += nDstPitch;
    pSrcp += nSrcPitch;
    pSrc  += nSrcPitch;
    pSrcn += nSrcPitch;

    for ( int y = 1; y < nHeight-1; y++ )
    {
        /* left */
        process_line_left(pDst, pSrcp, pSrc, pSrcn, matrix, low_thr_v, high_thr_v, 16);
        /* center */
        process_line(pDst + 16, pSrcp+16, pSrc+16, pSrcn+16, matrix, low_thr_v, high_thr_v, sse2_width - 16);
        /* right */
        process_line_right(pDst + nWidth - 16, pSrcp + nWidth - 16, pSrc + nWidth - 16, pSrcn + nWidth - 16, matrix, low_thr_v, high_thr_v, 16);

        pDst  += nDstPitch;
        pSrcp += nSrcPitch;
        pSrc  += nSrcPitch;
        pSrcn += nSrcPitch;
    }

    /* bottom-left */
    process_line_left(pDst, pSrcp, pSrc, pSrc, matrix, low_thr_v, high_thr_v, 16);
    /* bottom */
    process_line(pDst + 16, pSrcp+16, pSrc+16, pSrc+16, matrix, low_thr_v, high_thr_v, sse2_width - 16);
    /* bottom-right */
    process_line_right(pDst + nWidth - 16, pSrcp + nWidth - 16, pSrc + nWidth - 16, pSrc + nWidth - 16, matrix, low_thr_v, high_thr_v, 16);
}

} } } }

#endif