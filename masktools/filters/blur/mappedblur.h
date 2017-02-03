#ifndef __Mt_Blur_H__
#define __Mt_Blur_H__

#include "../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Blur { 

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch,
                        const Byte *pMask, ptrdiff_t nMaskPitch, const Short *matrix, int nWidth, int nHeight);

extern Processor *mapped_below_c;
extern Processor *mapped_all_c;

class MappedBlur : public MaskTools::Filter
{
   Short matrix[10];

   ProcessorList<Processor> processors;
protected:
    virtual void process(int n, const Plane<Byte> &dst, int nPlane, const Frame<const Byte> frames[3], const Constraint constraints[3]) override
    {
        UNUSED(n);
        processors.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
            frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
            frames[1].plane(nPlane).data(), frames[1].plane(nPlane).pitch(),
            matrix, dst.width(), dst.height());
    }

public:
   MappedBlur(const Parameters &parameters) : MaskTools::Filter( parameters, FilterProcessingType::CHILD )
   {
      auto coeffs = Parser::getDefaultParser().parse(parameters["kernel"].toString(), " ").getExpression();
      memset(matrix, 0, sizeof(matrix));
      for ( int i = 0; i < 9; i++ )
      {
         if ( !coeffs.size() )
         {
            error = "invalid kernel";
            return;
         }

         matrix[9] += abs(matrix[i] = Short(coeffs.front().getValue(0,0,0)));
         coeffs.pop_front();
      }

      if ( coeffs.size() )
         matrix[9] = Short(coeffs.front().getValue(0,0,0));

      if ( !matrix[9] )
         matrix[9] = 1;

      /* add the processors */
      if ( parameters["mode"].toString() == "replace" )
         processors.push_back(Filtering::Processor<Processor>(mapped_all_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
      else if ( parameters["mode"].toString() == "dump" )
         processors.push_back(Filtering::Processor<Processor>(mapped_below_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
      else
      {
         error = "unknown mode";
         return;
      }
   }

   InputConfiguration &input_configuration() const { return TwoFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_mappedblur";

      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(String("1 1 1 1 1 1 1 1 1"), "kernel"));
      signature.add(Parameter(String("replace"), "mode"));

      return add_defaults( signature );
   }
};

} } } }

#endif