#ifndef __Mt_Expand16_H__
#define __Mt_Expand16_H__
#if 0
#include "../morphologic16.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic16 { namespace Expand16 {


extern StackedProcessor *expand_square_stacked_c;
extern StackedProcessor *expand_horizontal_stacked_c;
extern StackedProcessor *expand_vertical_stacked_c;
extern StackedProcessor *expand_both_stacked_c;
extern StackedProcessor *expand_custom_stacked_c;

extern Processor16 *expand_square_interleaved_c;
extern Processor16 *expand_horizontal_interleaved_c;
extern Processor16 *expand_vertical_interleaved_c;
extern Processor16 *expand_both_interleaved_c;
extern Processor16 *expand_custom_interleaved_c;

class Expand16 : public Morphologic16::MorphologicFilter16
{
public:
    Expand16(const Parameters&parameters) : Morphologic16::MorphologicFilter16( parameters )
    {
        if (parameters["stacked"].toBool()) {
            /* add the processors */
            if ( parameters["mode"].toString() == "square" )
            {
                stackedProcessors.push_back(Filtering::Processor<StackedProcessor>(expand_square_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            }
            else if ( parameters["mode"].toString() == "horizontal" )
            {
                stackedProcessors.push_back(Filtering::Processor<StackedProcessor>(expand_horizontal_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            }
            else if ( parameters["mode"].toString() == "vertical" )
            {
                stackedProcessors.push_back(Filtering::Processor<StackedProcessor>(expand_vertical_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            }
            else if ( parameters["mode"].toString() == "both" )
            {
                stackedProcessors.push_back(Filtering::Processor<StackedProcessor>(expand_both_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            }
            else
            {
                stackedProcessors.push_back(Filtering::Processor<StackedProcessor>(expand_custom_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
                FillCoordinates( parameters["mode"].toString() );
            }
        } else {
            if ( parameters["mode"].toString() == "square" )
            {
                processors16.push_back(Filtering::Processor<Processor16>(expand_square_interleaved_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            }
            else if ( parameters["mode"].toString() == "horizontal" )
            {
                processors16.push_back(Filtering::Processor<Processor16>(expand_horizontal_interleaved_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            }
            else if ( parameters["mode"].toString() == "vertical" )
            {
                processors16.push_back(Filtering::Processor<Processor16>(expand_vertical_interleaved_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            }
            else if ( parameters["mode"].toString() == "both" )
            {
                processors16.push_back(Filtering::Processor<Processor16>(expand_both_interleaved_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            }
            else
            {
                processors16.push_back(Filtering::Processor<Processor16>(expand_custom_interleaved_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
                FillCoordinates( parameters["mode"].toString() );
            }
        }
    }

    static Signature Expand16::filter_signature()
    {
        Signature signature = "mt_expand16";

        signature.add( Parameter(TYPE_CLIP, "") );
        signature.add( Parameter(65535, "thY") );
        signature.add( Parameter(65535, "thC") );
        signature.add( Parameter(String("square"), "mode") );
        signature.add( Parameter( false, "stacked" ) );

        return add_defaults( signature );
    }
};
#endif
} } } } } // namespace Expand, Morphologic, Filter, MaskTools, Filtering

#endif