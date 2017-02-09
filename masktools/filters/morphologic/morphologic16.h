#ifndef __Mt_MorphologicFilter16_H__
#define __Mt_MorphologicFilter16_H__

#include "../../common/base/filter.h"
#include "../../../common/parser/parser.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Morphologic16 {

typedef void (StackedProcessor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, int nMaxDeviation, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight);
typedef void (InterleavedProcessor)(Word *pDst, ptrdiff_t nDstPitch, const Word *pSrc, ptrdiff_t nSrcPitch, int nMaxDeviation, const int *pCoordinates, int nCoordinates, int nWidth, int nHeight);

class MorphologicFilter16 : public MaskTools::Filter
{
   int nMaxDeviations[3];
   int *pCoordinates, nCoordinates;

   MorphologicFilter16(const MorphologicFilter16 &filter);

protected:

   ProcessorList<StackedProcessor> stackedProcessors;
   ProcessorList<InterleavedProcessor> interleavedProcessors;

   virtual void process(int n, const Plane<Byte> &dst, int nPlane, const ::Filtering::Frame<const Byte> frames[3], const Constraint constraints[3]) override
    {
        UNUSED(n);
        if (parameters["stacked"].toBool()) {
            stackedProcessors.best_processor(constraints[nPlane])(dst.data(), dst.pitch(), 
                frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(), 
                nMaxDeviations[nPlane], pCoordinates, nCoordinates, dst.width(), dst.height() / 2);
        }
        else {
            interleavedProcessors.best_processor(constraints[nPlane])(reinterpret_cast<Word*>((Byte*)dst.data()), dst.pitch() / 2, 
                reinterpret_cast<const Word*>((const Byte*)(frames[0].plane(nPlane).data())), frames[0].plane(nPlane).pitch() / 2, 
                nMaxDeviations[nPlane], pCoordinates, nCoordinates, dst.width() / 2, dst.height());
        }
    }

   void FillCoordinates(const String &coordinates)
   {
      auto coeffs = Parser::getDefaultParser().parse( coordinates, " (),;." ).getExpression();
      nCoordinates = coeffs.size();
      pCoordinates = new int[nCoordinates];
      int i = 0;

      while ( !coeffs.empty() )
      {
         pCoordinates[i++] = int( coeffs.front().getValue(0, 0, 0) );
         coeffs.pop_front();
      }
   }

public:
   MorphologicFilter16(const Parameters &parameters) : MaskTools::Filter( parameters, FilterProcessingType::CHILD ), pCoordinates( NULL ), nCoordinates( 0 )
   {
      nMaxDeviations[0] = clip<int, int>( parameters["thY"].toInt(), 0, 65535 );
      nMaxDeviations[1] = clip<int, int>( parameters["thC"].toInt(), 0, 65535 );
      nMaxDeviations[2] = clip<int, int>( parameters["thC"].toInt(), 0, 65535 );
   }

   ~MorphologicFilter16()
   {
      if ( pCoordinates )
         delete[] pCoordinates;
      pCoordinates = NULL;
   }

   InputConfiguration &input_configuration() const { return OneFrame(); }
};

} } } } // namespace Morphologic, Filters, MaskTools, Filtering

#endif