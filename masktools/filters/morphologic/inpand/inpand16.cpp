#include "inpand.h"
#include "../functions16.h"

using namespace Filtering;

typedef Word (local_minimum_f)(Word a1, Word a2, Word a3, Word a4, Word a5, Word a6, Word a7, Word a8, Word a9);

static inline Word minimum_square(Word a1, Word a2, Word a3, Word a4, Word a5, Word a6, Word a7, Word a8, Word a9)
{
    Word nMin = a1;
    if ( a2 < nMin ) nMin = a2;
    if ( a3 < nMin ) nMin = a3;
    if ( a4 < nMin ) nMin = a4;
    if ( a5 < nMin ) nMin = a5;
    if ( a6 < nMin ) nMin = a6;
    if ( a7 < nMin ) nMin = a7;
    if ( a8 < nMin ) nMin = a8;
    if ( a9 < nMin ) nMin = a9;
    return nMin;
}

static inline Word minimum_horizontal(Word a1, Word a2, Word a3, Word a4, Word a5, Word a6, Word a7, Word a8, Word a9)
{
    Word nMin = a4;

    UNUSED(a1); UNUSED(a2); UNUSED(a3); UNUSED(a7); UNUSED(a8); UNUSED(a9); 

    if ( a5 < nMin ) nMin = a5;
    if ( a6 < nMin ) nMin = a6;
    return nMin;
}

static inline Word minimum_vertical(Word a1, Word a2, Word a3, Word a4, Word a5, Word a6, Word a7, Word a8, Word a9)
{
    Word nMin = a2;

    UNUSED(a1); UNUSED(a3); UNUSED(a4); UNUSED(a6); UNUSED(a7); UNUSED(a9); 

    if ( a5 < nMin ) nMin = a5;
    if ( a8 < nMin ) nMin = a8;
    return nMin;
}

static inline Word minimum_both(Word a1, Word a2, Word a3, Word a4, Word a5, Word a6, Word a7, Word a8, Word a9)
{
    Word nMin = a2;

    UNUSED(a1); UNUSED(a3); UNUSED(a7); UNUSED(a9); 

    if ( a4 < nMin ) nMin = a4;
    if ( a5 < nMin ) nMin = a5;
    if ( a6 < nMin ) nMin = a6;
    if ( a8 < nMin ) nMin = a8;
    return nMin;
}

template<local_minimum_f Minimum>
static inline Word minimumThresholded(Word a1, Word a2, Word a3, Word a4, Word a5, Word a6, Word a7, Word a8, Word a9, int nMaxDeviation)
{
    int nMinimum = Minimum(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    if ( a5 - nMinimum > nMaxDeviation ) nMinimum = a5 - nMaxDeviation;
    return static_cast<Word>(nMinimum);
}

extern "C" MT_FORCEINLINE __m128i inpand_operator_sse4_16(__m128i a, __m128i b) {
  return _mm_min_epu16(a, b);
}

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic { namespace Inpand {

    class NewValue16 {
        int nMin;
        int nMaxDeviation;
        Word nValue;
    public:
        NewValue16(Word nValue, int nMaxDeviation) : nMin(65536), nMaxDeviation(nMaxDeviation), nValue(nValue) { }
        void add(Word _nValue) { if ( _nValue < nMin ) nMin = _nValue; }
        Word finalize() const { return static_cast<Word>(nMin > 65535 ? nValue : (nValue - nMin > nMaxDeviation ? nValue - nMaxDeviation : nMin)); }
    };

StackedProcessor *inpand_square_stacked_c = &MorphologicProcessor<Byte>::generic_16_c<
    process_line_morpho_stacked_c<Border::Left, minimumThresholded<::minimum_square>>,
    process_line_morpho_stacked_c<Border::None, minimumThresholded<::minimum_square>>,
    process_line_morpho_stacked_c<Border::Right, minimumThresholded<::minimum_square>>
    >;

StackedProcessor *inpand_horizontal_stacked_c = &MorphologicProcessor<Byte>::generic_16_c<
    process_line_morpho_stacked_c<Border::Left, minimumThresholded<::minimum_horizontal>>,
    process_line_morpho_stacked_c<Border::None, minimumThresholded<::minimum_horizontal>>,
    process_line_morpho_stacked_c<Border::Right, minimumThresholded<::minimum_horizontal>>
    >;

StackedProcessor *inpand_vertical_stacked_c = &MorphologicProcessor<Byte>::generic_16_c<
    process_line_morpho_stacked_c<Border::Left, minimumThresholded<::minimum_vertical>>,
    process_line_morpho_stacked_c<Border::None, minimumThresholded<::minimum_vertical>>,
    process_line_morpho_stacked_c<Border::Right, minimumThresholded<::minimum_vertical>>
    >;

StackedProcessor *inpand_both_stacked_c = &MorphologicProcessor<Byte>::generic_16_c<
    process_line_morpho_stacked_c<Border::Left, minimumThresholded<::minimum_both>>,
    process_line_morpho_stacked_c<Border::None, minimumThresholded<::minimum_both>>,
    process_line_morpho_stacked_c<Border::Right, minimumThresholded<::minimum_both>>
    >;

Processor16 *inpand_square_16_c = &MorphologicProcessor<Word>::generic_16_c<
    process_line_morpho_16_c<Border::Left, minimumThresholded<::minimum_square>>,
    process_line_morpho_16_c<Border::None, minimumThresholded<::minimum_square>>,
    process_line_morpho_16_c<Border::Right, minimumThresholded<::minimum_square>>
    >;

Processor16 *inpand_horizontal_16_c = &MorphologicProcessor<Word>::generic_16_c<
    process_line_morpho_16_c<Border::Left, minimumThresholded<::minimum_horizontal>>,
    process_line_morpho_16_c<Border::None, minimumThresholded<::minimum_horizontal>>,
    process_line_morpho_16_c<Border::Right, minimumThresholded<::minimum_horizontal>>
    >;

Processor16 *inpand_vertical_16_c = &MorphologicProcessor<Word>::generic_16_c<
    process_line_morpho_16_c<Border::Left, minimumThresholded<::minimum_vertical>>,
    process_line_morpho_16_c<Border::None, minimumThresholded<::minimum_vertical>>,
    process_line_morpho_16_c<Border::Right, minimumThresholded<::minimum_vertical>>
    >;

Processor16 *inpand_both_16_c = &MorphologicProcessor<Word>::generic_16_c<
    process_line_morpho_16_c<Border::Left, minimumThresholded<::minimum_both>>,
    process_line_morpho_16_c<Border::None, minimumThresholded<::minimum_both>>,
    process_line_morpho_16_c<Border::Right, minimumThresholded<::minimum_both>>
    >;

#define DEFINE_SSE4_VERSIONS(name, mem_mode) \
    Processor16 *inpand_both_##name##_16 = &generic_sse4_16< \
    process_line_xxpand_both_16<Border::Left, inpand_operator_sse4_16, limit_down_sse4_16, mem_mode>, \
    process_line_xxpand_both_16<Border::None, inpand_operator_sse4_16, limit_down_sse4_16, mem_mode>, \
    process_line_xxpand_both_16<Border::Right, inpand_operator_sse4_16, limit_down_sse4_16, MemoryMode::SSE2_UNALIGNED> \
    >;

DEFINE_SSE4_VERSIONS(sse4, MemoryMode::SSE2_UNALIGNED)
DEFINE_SSE4_VERSIONS(asse4, MemoryMode::SSE2_ALIGNED)
#undef DEFINE_SSE4_VERSIONS

Processor16 *inpand_vertical_sse4_16 = &xxpand_sse4_vertical_16<inpand_operator_sse4_16, limit_down_sse4_16, MemoryMode::SSE2_UNALIGNED>;
Processor16 *inpand_vertical_asse4_16 = &xxpand_sse4_vertical_16<inpand_operator_sse4_16, limit_down_sse4_16, MemoryMode::SSE2_ALIGNED>;

Processor16 *inpand_horizontal_sse4_16 = &xxpand_sse4_horizontal_16<inpand_operator_sse4_16, limit_down_sse4_16, MemoryMode::SSE2_UNALIGNED, inpand_c_horizontal_core_16>;
Processor16 *inpand_horizontal_asse4_16 = &xxpand_sse4_horizontal_16<inpand_operator_sse4_16, limit_down_sse4_16, MemoryMode::SSE2_ALIGNED, inpand_c_horizontal_core_16>;

Processor16 *inpand_square_sse4_16 = &xxpand_sse4_square_16<inpand_operator_sse4_16, limit_down_sse4_16, MemoryMode::SSE2_UNALIGNED, inpand_c_horizontal_core_16>;
Processor16 *inpand_square_asse4_16 = &xxpand_sse4_square_16<inpand_operator_sse4_16, limit_down_sse4_16, MemoryMode::SSE2_ALIGNED, inpand_c_horizontal_core_16>;


StackedProcessor *inpand_custom_stacked_c = &generic_custom_stacked_c<NewValue16>;
Processor16 *inpand_custom_16_c = &generic_custom_16_c<NewValue16>;

} } } } }