#include "mappedblur.h"
#include "../../filters/mask/functions16.h"

using namespace Filtering;

namespace Filtering { namespace MaskTools { namespace Filters { namespace Blur {

template<int bits_per_pixel>
inline Word convolution_all_16_t_c(Word a11, Word a21, Word a31, Word a12, Word a22, Word a32, Word a13, Word a23, Word a33, const Short matrix[10], int nLowThreshold, int nHighThreshold)
{
   int nSum = a22 * matrix[4];

   UNUSED(nHighThreshold);

   if ( abs(a11 - a22) < nLowThreshold ) nSum += a11 * matrix[0]; else nSum += a22 * matrix[0];
   if ( abs(a21 - a22) < nLowThreshold ) nSum += a21 * matrix[1]; else nSum += a22 * matrix[1];
   if ( abs(a31 - a22) < nLowThreshold ) nSum += a31 * matrix[2]; else nSum += a22 * matrix[2];
   if ( abs(a12 - a22) < nLowThreshold ) nSum += a12 * matrix[3]; else nSum += a22 * matrix[3];
   if ( abs(a32 - a22) < nLowThreshold ) nSum += a32 * matrix[5]; else nSum += a22 * matrix[5];
   if ( abs(a13 - a22) < nLowThreshold ) nSum += a13 * matrix[6]; else nSum += a22 * matrix[6];
   if ( abs(a23 - a22) < nLowThreshold ) nSum += a23 * matrix[7]; else nSum += a22 * matrix[7];
   if ( abs(a33 - a22) < nLowThreshold ) nSum += a33 * matrix[8]; else nSum += a22 * matrix[8];

   return clip<Word, int>((nSum + matrix[9] / 2)/ matrix[9], 0, (1 << bits_per_pixel) - 1);
}

template<int bits_per_pixel>
inline Word convolution_below_16_t_c(Word a11, Word a21, Word a31, Word a12, Word a22, Word a32, Word a13, Word a23, Word a33, const Short matrix[10], int nLowThreshold, int nHighThreshold)
{
   int nSum = a22 * matrix[4];
   int nSumCoeff = matrix[4];

   UNUSED(nHighThreshold);

   if ( abs(a11 - a22) < nLowThreshold ) { nSum += a11 * matrix[0]; nSumCoeff += matrix[0]; }
   if ( abs(a21 - a22) < nLowThreshold ) { nSum += a21 * matrix[1]; nSumCoeff += matrix[1]; }
   if ( abs(a31 - a22) < nLowThreshold ) { nSum += a31 * matrix[2]; nSumCoeff += matrix[2]; }
   if ( abs(a12 - a22) < nLowThreshold ) { nSum += a12 * matrix[3]; nSumCoeff += matrix[3]; }
   if ( abs(a32 - a22) < nLowThreshold ) { nSum += a32 * matrix[5]; nSumCoeff += matrix[5]; }
   if ( abs(a13 - a22) < nLowThreshold ) { nSum += a13 * matrix[6]; nSumCoeff += matrix[6]; }
   if ( abs(a23 - a22) < nLowThreshold ) { nSum += a23 * matrix[7]; nSumCoeff += matrix[7]; }
   if ( abs(a33 - a22) < nLowThreshold ) { nSum += a33 * matrix[8]; nSumCoeff += matrix[8]; }

   if ( nSumCoeff ) return clip<Word, int>((nSum + nSumCoeff / 2) / nSumCoeff, 0, (1 << bits_per_pixel) - 1);
   else return a22;
}

class Thresholds {
   const Word *pThreshold;
   ptrdiff_t nPitch;
public:
   Thresholds(const Word *pThreshold, ptrdiff_t nPitch) :
   pThreshold(pThreshold), nPitch(nPitch / sizeof(uint16_t))
   {
   }

   ptrdiff_t minpitch() const { return nPitch; }
   ptrdiff_t maxpitch() const { return nPitch; }
   void nextRow() { pThreshold += nPitch; }
   Word min(int x) const { return pThreshold[x]; }
   Word max(int x) const { return pThreshold[x]; }
};

template<Filters::Mask::Operator op>
void mask16_t(Word *pDst, ptrdiff_t nDstPitch, const Word *pSrc, ptrdiff_t nSrcPitch, const Word *pMask, ptrdiff_t nMaskPitch, const Short matrix[10], int nWidth, int nHeight)
{
   Thresholds thresholds(pMask, nMaskPitch);
   Filters::Mask::generic16_c<op, Thresholds>(pDst, nDstPitch, pSrc, nSrcPitch, thresholds, matrix, nWidth, nHeight);
}

Processor16 *mapped_below_10_c = &mask16_t<convolution_below_16_t_c<10>>;
Processor16 *mapped_all_10_c = &mask16_t<convolution_all_16_t_c<10>>;
Processor16 *mapped_below_12_c = &mask16_t<convolution_below_16_t_c<12>>;
Processor16 *mapped_all_12_c = &mask16_t<convolution_all_16_t_c<12>>;
Processor16 *mapped_below_14_c = &mask16_t<convolution_below_16_t_c<14>>;
Processor16 *mapped_all_14_c = &mask16_t<convolution_all_16_t_c<14>>;
Processor16 *mapped_below_16_c = &mask16_t<convolution_below_16_t_c<16>>;
Processor16 *mapped_all_16_c = &mask16_t<convolution_all_16_t_c<16>>;

} } } }