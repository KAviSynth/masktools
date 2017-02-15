#ifndef __Mt_Inpand16_H__
#define __Mt_Inpand16_H__
#if 0
#include "../morphologic.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic { namespace Inpand {


extern StackedProcessor *inpand_square_stacked_c;
extern StackedProcessor *inpand_horizontal_stacked_c;
extern StackedProcessor *inpand_vertical_stacked_c;
extern StackedProcessor *inpand_both_stacked_c;
extern StackedProcessor *inpand_custom_stacked_c;

extern Processor16 *inpand_square_interleaved_c;
extern Processor16 *inpand_horizontal_interleaved_c;
extern Processor16 *inpand_vertical_interleaved_c;
extern Processor16 *inpand_both_interleaved_c;
extern Processor16 *inpand_custom_interleaved_c;

class Inpand16 : public Morphologic::MorphologicFilter
{
public:
    Inpand16(const Parameters& parameters) : Morphologic::MorphologicFilter( parameters )
    {
        if (parameters["stacked"].toBool()) {
            /* add the processors */
            if ( parameters["mode"].toString() == "square" )
            {
                stackedProcessors.push_back(Filtering::Processor<StackedProcessor>(inpand_square_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            }
            else if ( parameters["mode"].toString() == "horizontal" )
            {
                stackedProcessors.push_back(Filtering::Processor<StackedProcessor>(inpand_horizontal_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            }
            else if ( parameters["mode"].toString() == "vertical" )
            {
                stackedProcessors.push_back(Filtering::Processor<StackedProcessor>(inpand_vertical_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            }
            else if ( parameters["mode"].toString() == "both" )
            {
                stackedProcessors.push_back(Filtering::Processor<StackedProcessor>(inpand_both_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            }
            else
            {
                stackedProcessors.push_back(Filtering::Processor<StackedProcessor>(inpand_custom_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
                FillCoordinates( parameters["mode"].toString() );
            }
        } else {
            /* add the processors */
            if ( parameters["mode"].toString() == "square" )
            {
                processors16.push_back(Filtering::Processor<Processor16>(inpand_square_interleaved_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            }
            else if ( parameters["mode"].toString() == "horizontal" )
            {
                processors16.push_back(Filtering::Processor<Processor16>(inpand_horizontal_interleaved_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            }
            else if ( parameters["mode"].toString() == "vertical" )
            {
                processors16.push_back(Filtering::Processor<Processor16>(inpand_vertical_interleaved_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            }
            else if ( parameters["mode"].toString() == "both" )
            {
                processors16.push_back(Filtering::Processor<Processor16>(inpand_both_interleaved_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            }
            else
            {
                stackedProcessors.push_back(Filtering::Processor<StackedProcessor>(inpand_custom_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
                FillCoordinates( parameters["mode"].toString() );
            }
        }
    }

    static Signature Inpand16::filter_signature()
    {
        Signature signature = "mt_inpand16";

        signature.add( Parameter( TYPE_CLIP, "" ) );
        signature.add( Parameter( TYPE_FLOAT, "thY" ) );
        signature.add( Parameter(TYPE_FLOAT, "thC" ) );
        signature.add( Parameter( String( "square" ), "mode" ) );
        signature.add( Parameter( false, "stacked" ) );

        return add_defaults( signature );
    }
};

} } } } } // namespace Inpand, Morphologic, Filter, MaskTools, Filtering

#endif
#endif