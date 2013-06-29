#ifndef __Mt_MorphologicFunctions_H__
#define __Mt_MorphologicFunctions_H__

#include "../../../common/utils/utils.h"
#include "../../common/simd.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic {

typedef Byte (Operator)(Byte, Byte, Byte, Byte, Byte, Byte, Byte, Byte, Byte, int);
typedef __m128i (Limit)(__m128i source, __m128i sum, __m128i deviation);
typedef void (ProcessLineSse2)(Byte *pDst, const Byte *pSrcp, const Byte *pSrc, const Byte *pSrcn, const __m128i &maxDeviation, int width);

enum class Border {
    Left,
    Right,
    None
};

enum Directions {
    Vertical = 1,
    Horizontal = 2,
    Both = Vertical | Horizontal,
    Square = 7
};


template<Operator op>
void generic_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nMaxDeviation, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight)
{
   const Byte *pSrcp = pSrc - nSrcPitch;
   const Byte *pSrcn = pSrc + nSrcPitch;

   UNUSED(nCoordinates); UNUSED(pCoordinates);

   /* top-left */
   pDst[0] = op(pSrc[0], pSrc[0], pSrc[1], pSrc[0], pSrc[0], pSrc[1], pSrcn[0], pSrcn[0], pSrcn[1], nMaxDeviation);

   /* top */
   for ( int x = 1; x < nWidth - 1; x++ )
      pDst[x] = op(pSrc[x-1], pSrc[x], pSrc[x+1], pSrc[x-1], pSrc[x], pSrc[x+1], pSrcn[x-1], pSrcn[x], pSrcn[x+1], nMaxDeviation);

   /* top-right */
   pDst[nWidth-1] = op(pSrc[nWidth-2], pSrc[nWidth-1], pSrc[nWidth-1], pSrc[nWidth-2], pSrc[nWidth-1], pSrc[nWidth-1], pSrcn[nWidth-2], pSrcn[nWidth-1], pSrcn[nWidth-1], nMaxDeviation);

   pDst  += nDstPitch;
   pSrcp += nSrcPitch;
   pSrc  += nSrcPitch;
   pSrcn += nSrcPitch;

   for ( int y = 1; y < nHeight - 1; y++ )
   {
      /* left */
      pDst[0] = op(pSrcp[0], pSrcp[0], pSrcp[1], pSrc[0], pSrc[0], pSrc[1], pSrcn[0], pSrcn[0], pSrcn[1], nMaxDeviation);

      /* center */
      for ( int x = 1; x < nWidth - 1; x++ )
         pDst[x] = op(pSrcp[x-1], pSrcp[x], pSrcp[x+1], pSrc[x-1], pSrc[x], pSrc[x+1], pSrcn[x-1], pSrcn[x], pSrcn[x+1], nMaxDeviation);

      /* right */
      pDst[nWidth-1] = op(pSrcp[nWidth-2], pSrcp[nWidth-1], pSrcp[nWidth-1], pSrc[nWidth-2], pSrc[nWidth-1], pSrc[nWidth-1], pSrcn[nWidth-2], pSrcn[nWidth-1], pSrcn[nWidth-1], nMaxDeviation);

      pDst  += nDstPitch;
      pSrcp += nSrcPitch;
      pSrc  += nSrcPitch;
      pSrcn += nSrcPitch;
   }

   /* bottom-left */
   pDst[0] = op(pSrcp[0], pSrcp[0], pSrcp[1], pSrc[0], pSrc[0], pSrc[1], pSrc[0], pSrc[0], pSrc[1], nMaxDeviation);

   /* bottom */
   for ( int x = 1; x < nWidth - 1; x++ )
      pDst[x] = op(pSrcp[x-1], pSrcp[x], pSrcp[x+1], pSrc[x-1], pSrc[x], pSrc[x+1], pSrc[x-1], pSrc[x], pSrc[x+1], nMaxDeviation);

   /* bottom-right */
   pDst[nWidth-1] = op(pSrcp[nWidth-2], pSrcp[nWidth-1], pSrcp[nWidth-1], pSrc[nWidth-2], pSrc[nWidth-1], pSrc[nWidth-1], pSrc[nWidth-2], pSrc[nWidth-1], pSrc[nWidth-1], nMaxDeviation);
}

extern "C" static FORCEINLINE __m128i limit_up_sse2(__m128i source, __m128i sum, __m128i deviation) {
    auto limit = _mm_adds_epu8(source, deviation);
    return _mm_min_epu8(limit, _mm_max_epu8(source, sum));
}

extern "C" static FORCEINLINE __m128i limit_down_sse2(__m128i source, __m128i sum, __m128i deviation) {
    auto limit = _mm_subs_epu8(source, deviation);
    return _mm_max_epu8(limit, _mm_min_epu8(source, sum));
}


template<bool isBorder, decltype(simd_load_epi128) load>
static FORCEINLINE __m128i load_one_to_left(const Byte *ptr) {
    if (isBorder) {
        auto mask_left = _mm_setr_epi8(0xFF, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00);
        auto val = load(reinterpret_cast<const __m128i*>(ptr));
        return _mm_or_si128(_mm_slli_si128(val, 1), _mm_and_si128(val, mask_left));
    } else {
        return simd_loadu_epi128(reinterpret_cast<const __m128i*>(ptr - 1));
    }
}

template<bool isBorder, decltype(simd_load_epi128) load>
static FORCEINLINE __m128i load_one_to_right(const Byte *ptr) {
    if (isBorder) {
        auto mask_right = _mm_setr_epi8(00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 0xFF);
        auto val = load(reinterpret_cast<const __m128i*>(ptr));
        return _mm_or_si128(_mm_srli_si128(val, 1), _mm_and_si128(val, mask_right));
    } else {
        return simd_loadu_epi128(reinterpret_cast<const __m128i*>(ptr + 1));
    }
}

template<Border borderMode, Limit limit, decltype(simd_load_epi128) load, decltype(simd_store_epi128) store>
static FORCEINLINE void process_line_xxflate(Byte *pDst, const Byte *pSrcp, const Byte *pSrc, const Byte *pSrcn, const __m128i &maxDeviation, int width) {
    auto zero = _mm_setzero_si128();
    for ( int x = 0; x < width; x+=16 ) {
        auto up_left = load_one_to_left<borderMode == Border::Left, load>(pSrcp+x);
        auto up_center = load(reinterpret_cast<const __m128i*>(pSrcp + x));
        auto up_right = load_one_to_right<borderMode == Border::Right, load>(pSrcp+x);
        
        auto middle_left = load_one_to_left<borderMode == Border::Left, load>(pSrc+x);
        auto middle_right = load_one_to_right<borderMode == Border::Right, load>(pSrc+x);
        
        auto down_left = load_one_to_left<borderMode == Border::Left, load>(pSrcn+x);
        auto down_center = load(reinterpret_cast<const __m128i*>(pSrcn + x));
        auto down_right = load_one_to_right<borderMode == Border::Right, load>(pSrcn+x);

        auto up_left_lo = _mm_unpacklo_epi8(up_left, zero);
        auto up_left_hi = _mm_unpackhi_epi8(up_left, zero);

        auto up_center_lo = _mm_unpacklo_epi8(up_center, zero);
        auto up_center_hi = _mm_unpackhi_epi8(up_center, zero);

        auto up_right_lo = _mm_unpacklo_epi8(up_right, zero);
        auto up_right_hi = _mm_unpackhi_epi8(up_right, zero);

        auto middle_left_lo = _mm_unpacklo_epi8(middle_left, zero);
        auto middle_left_hi = _mm_unpackhi_epi8(middle_left, zero);

        auto middle_right_lo = _mm_unpacklo_epi8(middle_right, zero);
        auto middle_right_hi = _mm_unpackhi_epi8(middle_right, zero);

        auto down_left_lo = _mm_unpacklo_epi8(down_left, zero);
        auto down_left_hi = _mm_unpackhi_epi8(down_left, zero);

        auto down_center_lo = _mm_unpacklo_epi8(down_center, zero);
        auto down_center_hi = _mm_unpackhi_epi8(down_center, zero);

        auto down_right_lo = _mm_unpacklo_epi8(down_right, zero);
        auto down_right_hi = _mm_unpackhi_epi8(down_right, zero);

        auto sum_lo = _mm_add_epi16(up_left_lo, up_center_lo);
        sum_lo = _mm_add_epi16(sum_lo, up_right_lo);
        sum_lo = _mm_add_epi16(sum_lo, middle_left_lo);
        sum_lo = _mm_add_epi16(sum_lo, middle_right_lo);
        sum_lo = _mm_add_epi16(sum_lo, down_left_lo);
        sum_lo = _mm_add_epi16(sum_lo, down_center_lo);
        sum_lo = _mm_add_epi16(sum_lo, down_right_lo);

        auto sum_hi = _mm_add_epi16(up_left_hi, up_center_hi);
        sum_hi = _mm_add_epi16(sum_hi, up_right_hi);
        sum_hi = _mm_add_epi16(sum_hi, middle_left_hi);
        sum_hi = _mm_add_epi16(sum_hi, middle_right_hi);
        sum_hi = _mm_add_epi16(sum_hi, down_left_hi);
        sum_hi = _mm_add_epi16(sum_hi, down_center_hi);
        sum_hi = _mm_add_epi16(sum_hi, down_right_hi);

        sum_lo = _mm_srai_epi16(sum_lo, 3);
        sum_hi = _mm_srai_epi16(sum_hi, 3);

        auto result = _mm_packus_epi16(sum_lo, sum_hi);

        auto middle_center = load(reinterpret_cast<const __m128i*>(pSrc + x));
        
        result = limit(middle_center, result, maxDeviation);

        store(reinterpret_cast<__m128i*>(pDst+x), result);
    }
}

template<Directions directions, Border borderMode, decltype(_mm_max_epu8) op, Limit limit, decltype(simd_load_epi128) load, decltype(simd_store_epi128) store>
static FORCEINLINE void process_line_xxpand(Byte *pDst, const Byte *pSrcp, const Byte *pSrc, const Byte *pSrcn, const __m128i &maxDeviation, int width) {
    for ( int x = 0; x < width; x+=16 ) {
        __m128i up_left, up_center, up_right, middle_left, middle_right, down_left, down_center, down_right;

        if (directions == Directions::Square) {
            up_left = load_one_to_left<borderMode == Border::Left, load>(pSrcp+x);
            up_right = load_one_to_right<borderMode == Border::Right, load>(pSrcp+x);
            down_left = load_one_to_left<borderMode == Border::Left, load>(pSrcn+x);
            down_right = load_one_to_right<borderMode == Border::Right, load>(pSrcn+x);
        }
        
        if (directions & Directions::Vertical) {
            up_center = load(reinterpret_cast<const __m128i*>(pSrcp + x));
            down_center = load(reinterpret_cast<const __m128i*>(pSrcn + x));
        }

        if (directions & Directions::Horizontal) {
            middle_left = load_one_to_left<borderMode == Border::Left, load>(pSrc+x);
            middle_right = load_one_to_right<borderMode == Border::Right, load>(pSrc+x);
        }

        __m128i acc;
        if (directions == Directions::Square) {
            acc = op(up_left, up_center);
            acc = op(acc, up_right);
            acc = op(acc, middle_left);
            acc = op(acc, middle_right);
            acc = op(acc, down_left);
            acc = op(acc, down_center);
            acc = op(acc, down_right);
        } else if (directions == Directions::Horizontal) {
            acc = op(middle_left, middle_right);
        } else if (directions == Directions::Vertical) {
            acc = op(up_center, down_center); 
        } else  if (directions == Directions::Both) {
            acc = op(up_center, middle_left);
            acc = op(acc, middle_right); 
            acc = op(acc, down_center); 
        }

        auto middle_center = load(reinterpret_cast<const __m128i*>(pSrc + x));

        auto result = limit(middle_center, acc, maxDeviation);

        store(reinterpret_cast<__m128i*>(pDst+x), result);
    }
}

template<ProcessLineSse2 process_line_left, ProcessLineSse2 process_line, ProcessLineSse2 process_line_right>
static void generic_sse2(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nMaxDeviation, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight) {
    const Byte *pSrcp = pSrc - nSrcPitch;
    const Byte *pSrcn = pSrc + nSrcPitch;

    UNUSED(nCoordinates); UNUSED(pCoordinates);
    auto max_dev_v = simd_set8_epi32(nMaxDeviation);
    int sse2_width = (nWidth - 1 - 16) / 16 * 16 + 16;
    /* top-left */
    process_line_left(pDst, pSrc, pSrc, pSrcn, max_dev_v, 16);
    /* top */
    process_line(pDst + 16, pSrc+16, pSrc+16, pSrcn+16, max_dev_v, sse2_width - 16);

    /* top-right */
    process_line_right(pDst + nWidth - 16, pSrc + nWidth - 16, pSrc + nWidth - 16, pSrcn + nWidth - 16, max_dev_v, 16);

    pDst  += nDstPitch;
    pSrcp += nSrcPitch;
    pSrc  += nSrcPitch;
    pSrcn += nSrcPitch;

    for ( int y = 1; y < nHeight-1; y++ )
    {
        /* left */
        process_line_left(pDst, pSrcp, pSrc, pSrcn, max_dev_v, 16);
        /* center */
        process_line(pDst + 16, pSrcp+16, pSrc+16, pSrcn+16, max_dev_v, sse2_width - 16);
        /* right */
        process_line_right(pDst + nWidth - 16, pSrcp + nWidth - 16, pSrc + nWidth - 16, pSrcn + nWidth - 16, max_dev_v, 16);

        pDst  += nDstPitch;
        pSrcp += nSrcPitch;
        pSrc  += nSrcPitch;
        pSrcn += nSrcPitch;
    }

    /* bottom-left */
    process_line_left(pDst, pSrcp, pSrc, pSrc, max_dev_v, 16);
    /* bottom */
    process_line(pDst + 16, pSrcp+16, pSrc+16, pSrc+16, max_dev_v, sse2_width - 16);
    /* bottom-right */
    process_line_right(pDst + nWidth - 16, pSrcp + nWidth - 16, pSrc + nWidth - 16, pSrc + nWidth - 16, max_dev_v, 16);
}

template<class T>
void generic_custom_c(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nMaxDeviation, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight)
{
   for ( int j = 0; j < nHeight; j++ )
   {
      for ( int i = 0; i < nWidth; i++ )
      {
         T new_value(pSrc[i], nMaxDeviation);
         for ( int k = 0; k < nCoordinates; k+=2 )
         {
            if ( pCoordinates[k] + i >= 0 && pCoordinates[k] + i < nWidth &&
                 pCoordinates[k+1] + j >= 0 && pCoordinates[k+1] + j < nHeight )
               new_value.add(pSrc[i + pCoordinates[k] + pCoordinates[k+1] * nSrcPitch]);
         }
         pDst[i] = new_value.finalize();
      }
      pSrc += nSrcPitch;
      pDst += nDstPitch;
   }
}

} } } } // namespace Morphologic, Filters, MaskTools, Filtering

#endif