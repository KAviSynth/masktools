#ifndef __Mt_Average16_H__
#define __Mt_Average16_H__

#include "../../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Support { namespace Average16 {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nWidth, int nHeight);

Processor average16_stacked_c;
Processor average16_stacked_sse2;

Processor average16_interleaved_c;
Processor average16_interleaved_sse2;

class Average16 : public MaskTools::Filter
{

   ProcessorList<Processor> processors;

protected:

   virtual void process(int n, const Plane<Byte> &dst, int nPlane, const Filtering::Frame<const Byte> frames[3], const Constraint constraints[3]) override
   {
      UNUSED(n);
      processors.best_processor( constraints[nPlane] )( dst.data(), dst.pitch(), frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(), dst.width(), dst.height() );
   }

public:
    Average16(const Parameters &parameters) : MaskTools::Filter( parameters, FilterProcessingType::INPLACE )
    {
        if (parameters["stacked"].toBool() == true) {
            /* add the processors */
            processors.push_back(Filtering::Processor<Processor>(average16_stacked_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            processors.push_back(Filtering::Processor<Processor>(average16_stacked_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
        } else {
            processors.push_back(Filtering::Processor<Processor>(average16_interleaved_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
            processors.push_back(Filtering::Processor<Processor>(average16_interleaved_sse2, Constraint(CPU_SSE2, 1, 1, 1, 1), 1));
        }
    }

   InputConfiguration &input_configuration() const { return InPlaceTwoFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_average16";

      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(false, "stacked"));

      return add_defaults( signature );
   }

};

} } } } }

#endif