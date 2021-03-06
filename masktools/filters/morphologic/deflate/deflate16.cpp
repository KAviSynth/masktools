#include "deflate.h"
#include "../functions16.h"
#include "../../../common/simd.h"

using namespace Filtering;

static MT_FORCEINLINE Word meanMin(Word a1, Word a2, Word a3, Word a4, Word a5, Word a6, Word a7, Word a8, Word a9)
{
    int nSum = 0;
    nSum += a1 + a2 + a3 + a4 + a6 + a7 + a8 + a9;
    nSum >>= 3;
    return static_cast<Word>(nSum < a5 ? nSum : a5);
}

static MT_FORCEINLINE Word meanMinThresholded(Word a1, Word a2, Word a3, Word a4, Word a5, Word a6, Word a7, Word a8, Word a9, int nMaxDeviation)
{
    int nMeanMin = meanMin(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    if ( a5 - nMeanMin > nMaxDeviation ) nMeanMin = a5 - nMaxDeviation;
    return static_cast<Word>(nMeanMin);
}

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic { namespace Deflate {

StackedProcessor *deflate_stacked_c = &MorphologicProcessor<Byte>::generic_16_c<
    process_line_morpho_stacked_c<Border::Left, meanMinThresholded>,
    process_line_morpho_stacked_c<Border::None, meanMinThresholded>,
    process_line_morpho_stacked_c<Border::Right, meanMinThresholded>
>;

Processor16 *deflate_16_c = &MorphologicProcessor<Word>::generic_16_c<
    process_line_morpho_16_c<Border::Left, meanMinThresholded>,
    process_line_morpho_16_c<Border::None, meanMinThresholded>,
    process_line_morpho_16_c<Border::Right, meanMinThresholded>
>;

Processor16 *deflate_sse4_16 = &generic_sse4_16<
  process_line_xxflate_16<Border::Left, limit_down_sse4_16, MemoryMode::SSE2_UNALIGNED>,
  process_line_xxflate_16<Border::None, limit_down_sse4_16, MemoryMode::SSE2_UNALIGNED>,
  process_line_xxflate_16<Border::Right, limit_down_sse4_16, MemoryMode::SSE2_UNALIGNED>
>;
Processor16 *deflate_asse4_16 = &generic_sse4_16<
  process_line_xxflate_16<Border::Left, limit_down_sse4_16, MemoryMode::SSE2_ALIGNED>,
  process_line_xxflate_16<Border::None, limit_down_sse4_16, MemoryMode::SSE2_ALIGNED>,
  process_line_xxflate_16<Border::Right, limit_down_sse4_16, MemoryMode::SSE2_UNALIGNED>
>;


} } } } }