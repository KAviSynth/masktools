#ifndef __Mt_MorphologicFunctions_H__
#define __Mt_MorphologicFunctions_H__

#include "../../../common/utils/utils.h"
#include "../../common/simd.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic {

typedef Byte (Operator)(Byte, Byte, Byte, Byte, Byte, Byte, Byte, Byte, Byte, int);
typedef __m128i (Limit)(__m128i source, __m128i sum, __m128i deviation);
typedef void (ProcessLineSse2)(Byte *pDst, const Byte *pSrcp, const Byte *pSrc, const Byte *pSrcn, const __m128i &maxDeviation, int width);

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

static MT_FORCEINLINE __m128i limit_up_sse2(__m128i source, __m128i sum, __m128i deviation) {
    auto limit = _mm_adds_epu8(source, deviation);
    return _mm_min_epu8(limit, _mm_max_epu8(source, sum));
}

static MT_FORCEINLINE __m128i limit_down_sse2(__m128i source, __m128i sum, __m128i deviation) {
    auto limit = _mm_subs_epu8(source, deviation);
    return _mm_max_epu8(limit, _mm_min_epu8(source, sum));
}


template<Border borderMode, Limit limit, MemoryMode mem_mode>
static MT_FORCEINLINE void process_line_xxflate(Byte *pDst, const Byte *pSrcp, const Byte *pSrc, const Byte *pSrcn, const __m128i &maxDeviation, int width) {
    auto zero = _mm_setzero_si128();
    for ( int x = 0; x < width; x+=16 ) {
        auto up_left = load_one_to_left<borderMode, mem_mode>(pSrcp+x);
        auto up_center = simd_load_si128<mem_mode>(pSrcp + x);
        auto up_right = load_one_to_right<borderMode, mem_mode>(pSrcp+x);
        
        auto middle_left = load_one_to_left<borderMode, mem_mode>(pSrc+x);
        auto middle_right = load_one_to_right<borderMode, mem_mode>(pSrc+x);
        
        auto down_left = load_one_to_left<borderMode, mem_mode>(pSrcn+x);
        auto down_center = simd_load_si128<mem_mode>(pSrcn + x);
        auto down_right = load_one_to_right<borderMode, mem_mode>(pSrcn+x);

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

        auto middle_center = simd_load_si128<mem_mode>(pSrc + x);
        
        result = limit(middle_center, result, maxDeviation);

        simd_store_si128<mem_mode>(pDst+x, result);
    }
}

template<Border borderMode, decltype(_mm_max_epu8) op, Limit limit, MemoryMode mem_mode>
static MT_FORCEINLINE void process_line_xxpand_both(Byte *pDst, const Byte *pSrcp, const Byte *pSrc, const Byte *pSrcn, const __m128i &maxDeviation, int width) {
    for (int x = 0; x < width; x += 16) {
        __m128i up_center = simd_load_si128<mem_mode>(pSrcp+x);
        __m128i middle_center = simd_load_si128<mem_mode>(pSrc+x);
        __m128i down_center = simd_load_si128<mem_mode>(pSrcn+x);
        __m128i middle_left = load_one_to_left<borderMode, mem_mode>(pSrc+x);
        __m128i middle_right = load_one_to_right<borderMode, mem_mode>(pSrc+x);

        __m128i acc = op(up_center, middle_left);
        acc = op(acc, middle_right);
        acc = op(acc, down_center);

        __m128i result = limit(middle_center, acc, maxDeviation);
        simd_store_si128<mem_mode>(pDst+x, result);
    }
}

/* Vertical mt_xxpand */

template<decltype(_mm_max_epu8) op, Limit limit, MemoryMode mem_mode>
static MT_FORCEINLINE void process_line_xxpand_vertical(Byte *pDst, const Byte *pSrcp, const Byte *pSrc, const Byte *pSrcn, const __m128i &maxDeviation, int width) {
    for (int x = 0; x < width; x += 16) {
        auto up_center = simd_load_si128<mem_mode>(pSrcp+x);
        auto down_center = simd_load_si128<mem_mode>(pSrcn+x);
        auto middle_center = simd_load_si128<mem_mode>(pSrc+x);
        auto acc = op(up_center, down_center);
        auto result = limit(middle_center, acc, maxDeviation);
        simd_store_si128<mem_mode>(pDst+x, result);
    }
}

template<decltype(_mm_max_epu8) op, Limit limit, MemoryMode mem_mode>
static void xxpand_sse2_vertical(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nMaxDeviation, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight) {
    const Byte *pSrcp = pSrc - nSrcPitch;
    const Byte *pSrcn = pSrc + nSrcPitch;

    UNUSED(nCoordinates); UNUSED(pCoordinates);
    auto max_dev_v = _mm_set1_epi8(Byte(nMaxDeviation));
    int mod16_width = nWidth / 16 * 16;
    bool not_mod16 = nWidth != mod16_width;
   
    process_line_xxpand_vertical<op, limit, mem_mode>(pDst, pSrc, pSrc, pSrcn, max_dev_v, mod16_width);

    if (not_mod16) {
        process_line_xxpand_vertical<op, limit, MemoryMode::SSE2_UNALIGNED>(pDst + nWidth - 16, pSrc+nWidth-16, pSrc+nWidth-16, pSrcn+nWidth-16, max_dev_v, 16);
    }

    pDst += nDstPitch;
    pSrcp += nSrcPitch;
    pSrc += nSrcPitch;
    pSrcn += nSrcPitch;

    for (int y = 1; y < nHeight-1; y++)
    {
        process_line_xxpand_vertical<op, limit, mem_mode>(pDst, pSrcp, pSrc, pSrcn, max_dev_v, mod16_width);

        if (not_mod16) {
            process_line_xxpand_vertical<op, limit, MemoryMode::SSE2_UNALIGNED>(pDst + nWidth - 16, pSrcp+nWidth - 16, pSrc+nWidth - 16, pSrcn+nWidth - 16, max_dev_v, 16);
        }

        pDst += nDstPitch;
        pSrcp += nSrcPitch;
        pSrc += nSrcPitch;
        pSrcn += nSrcPitch;
    }
    
    process_line_xxpand_vertical<op, limit, mem_mode>(pDst, pSrcp, pSrc, pSrc, max_dev_v, mod16_width);

    if (not_mod16) {
        process_line_xxpand_vertical<op, limit, MemoryMode::SSE2_UNALIGNED>(pDst + nWidth - 16, pSrcp+nWidth - 16, pSrc+nWidth - 16, pSrc+nWidth - 16, max_dev_v, 16);
    }
}


/* Horizontal mt_xxpand */

static MT_FORCEINLINE Byte expand_c_horizontal_core(Byte left, Byte center, Byte right, Byte max_dev) {
    Byte ma = left;

    if (center > ma) ma = center;
    if (right > ma) ma = right;
    
    if (ma - center > max_dev) ma = center + max_dev;
    return static_cast<Byte>(ma);
}

static MT_FORCEINLINE Byte inpand_c_horizontal_core(Byte left, Byte center, Byte right, Byte max_dev) {
    Byte mi = left;

    if (center < mi) mi = center;
    if (right < mi) mi = right;

    if (center - mi > max_dev) mi = center - max_dev;
    return static_cast<Byte>(mi);
}

//this implemented in a somewhat retarded way to allow pDst and pSrc be the same pointer (used in 3x3 expand)
template<decltype(_mm_max_epu8) op, Limit limit, MemoryMode mem_mode, decltype(expand_c_horizontal_core) c_core>
static void xxpand_sse2_horizontal(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nMaxDeviation, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight) {
    UNUSED(nCoordinates); UNUSED(pCoordinates);

    int mod16_width = (nWidth / 16) * 16;
    int sse_loop_limit = nWidth == mod16_width ? mod16_width - 16 : mod16_width;
    
    Byte max_dev = Byte(nMaxDeviation);
    __m128i max_dev_v = _mm_set1_epi8(max_dev);
    __m128i left_mask = _mm_set_epi32(0, 0, 0, 0xFF);
#pragma warning(disable: 4309)
    __m128i right_mask = _mm_set_epi8(0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#pragma warning(default: 4309)

    __m128i left;

    for (int y = 0; y < nHeight; ++y) {
        //left border
        __m128i center = simd_load_si128<mem_mode>(pSrc);
        __m128i right = simd_load_si128<MemoryMode::SSE2_UNALIGNED>(pSrc+1);
        left = _mm_or_si128(_mm_and_si128(center, left_mask), _mm_slli_si128(center, 1));

        __m128i result = op(left, right);
        result = limit(center, result, max_dev_v);

        left = simd_load_si128<MemoryMode::SSE2_UNALIGNED>(pSrc+15);
        simd_store_si128<mem_mode>(pDst, result);

        //main processing loop
        for (int x = 16; x < sse_loop_limit; x += 16) {
            center = simd_load_si128<mem_mode>(pSrc+x);
            right = simd_load_si128<MemoryMode::SSE2_UNALIGNED>(pSrc+x+1);

            result = op(left, right);
            result = limit(center, result, max_dev_v);

            left = simd_load_si128<MemoryMode::SSE2_UNALIGNED>(pSrc+x+15);

            simd_store_si128<mem_mode>(pDst+x, result);
        }

        //right border
        if (mod16_width == nWidth) {
            center = simd_load_si128<mem_mode>(pSrc+mod16_width-16);
            right = _mm_or_si128(_mm_and_si128(center, right_mask), _mm_srli_si128(center, 1));

            result = op(left, right);
            result = limit(center, result, max_dev_v);

            simd_store_si128<mem_mode>(pDst+mod16_width-16, result);
        } else { //some stuff left
             Byte l = _mm_cvtsi128_si32(left) & 0xFF;

             int x;
             for (x = mod16_width; x < nWidth-1; ++x) {
                 Byte temp = c_core(l, pSrc[x], pSrc[x+1], max_dev);
                 l = pSrc[x];
                 pDst[x] = temp;
             }
             pDst[x] = c_core(l, pSrc[x], pSrc[x], max_dev);
        }
        pSrc += nSrcPitch;
        pDst += nDstPitch;
    }
}

/* Square mt_xxpand */

template<decltype(_mm_max_epu8) op, Limit limit, MemoryMode mem_mode, decltype(expand_c_horizontal_core) c_core>
static void xxpand_sse2_square(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nMaxDeviation, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight) {
    xxpand_sse2_vertical<op, limit, mem_mode>(pDst, nDstPitch, pSrc, nSrcPitch, nMaxDeviation, pCoordinates, nCoordinates, nWidth, nHeight);
    xxpand_sse2_horizontal<op, limit, mem_mode, c_core>(pDst, nDstPitch, pDst, nDstPitch, nMaxDeviation, pCoordinates, nCoordinates, nWidth, nHeight);
}


template<ProcessLineSse2 process_line_left, ProcessLineSse2 process_line, ProcessLineSse2 process_line_right>
static void generic_sse2(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nMaxDeviation, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight) {
    const Byte *pSrcp = pSrc - nSrcPitch;
    const Byte *pSrcn = pSrc + nSrcPitch;

    UNUSED(nCoordinates); UNUSED(pCoordinates);
    auto max_dev_v = _mm_set1_epi8(Byte(nMaxDeviation));
    int mod16_width = (nWidth - 1 - 16) / 16 * 16 + 16;
    /* top-left */
    process_line_left(pDst, pSrc, pSrc, pSrcn, max_dev_v, 16);
    /* top */
    process_line(pDst + 16, pSrc+16, pSrc+16, pSrcn+16, max_dev_v, mod16_width - 16);

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
        process_line(pDst + 16, pSrcp+16, pSrc+16, pSrcn+16, max_dev_v, mod16_width - 16);
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
    process_line(pDst + 16, pSrcp+16, pSrc+16, pSrc+16, max_dev_v, mod16_width - 16);
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