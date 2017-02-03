#ifndef __Mt_Edgemask_H__
#define __Mt_Edgemask_H__

#include "../../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Mask { namespace Edge {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc, ptrdiff_t nSrcPitch, const Short matrix[10], int nLowThreshold, int nHighThreshold, int nWidth, int nHeight);

extern Processor *convolution_c;
extern Processor *convolution_sse2;

extern Processor *sobel_c;
extern Processor *sobel_sse2;
extern Processor *sobel_ssse3;

extern Processor *roberts_c;
extern Processor *roberts_sse2;
extern Processor *roberts_ssse3;

extern Processor *laplace_c;
extern Processor *laplace_sse2;
extern Processor *laplace_ssse3;

extern Processor *cartoon_c;
extern Processor *cartoon_sse2;

extern Processor *half_prewitt_c;
extern Processor *half_prewitt_sse2;
extern Processor *half_prewitt_ssse3;

extern Processor *prewitt_c;
extern Processor *prewitt_sse2;
extern Processor *prewitt_ssse3;

extern Processor *morpho_c;
extern Processor *morpho_sse2;

class EdgeMask : public MaskTools::Filter
{
   int nLowThresholds[3];
   int nHighThresholds[3];
   Short matrix[10];

   ProcessorList<Processor> processors;

protected:

    virtual void process(int n, const Plane<Byte> &dst, int nPlane, const Filtering::Frame<const Byte> frames[3], const Constraint constraints[3]) override
    {
        UNUSED(n);
        processors.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
            frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
            matrix, nLowThresholds[nPlane], nHighThresholds[nPlane], dst.width(), dst.height());
    }

public:
   EdgeMask(const Parameters &parameters) : MaskTools::Filter( parameters, FilterProcessingType::CHILD )
   {
      nLowThresholds[0] = clip<int, int>( parameters["thY1"].toInt(), 0, 255 );
      nLowThresholds[1] = nLowThresholds[2] = clip<int, int>( parameters["thC1"].toInt(), 0, 255 );
      nHighThresholds[0] = clip<int, int>( parameters["thY2"].toInt(), 0, 255 );
      nHighThresholds[1] = nHighThresholds[2] = clip<int, int>( parameters["thC2"].toInt(), 0, 255 );

      /* add the processors */
      if ( parameters["mode"].toString() == "sobel" )
      {
         print(LOG_DEBUG, "Edge : using sobel detector");
         processors.push_back(Filtering::Processor<Processor>(sobel_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
         processors.push_back(Filtering::Processor<Processor>(sobel_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 1));
         processors.push_back(Filtering::Processor<Processor>(sobel_ssse3, Constraint(CPU_SSSE3, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 2));
      }
      else if ( parameters["mode"].toString() == "roberts" )
      {
         print(LOG_DEBUG, "Edge : using roberts detector");
         processors.push_back(Filtering::Processor<Processor>(roberts_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
         processors.push_back(Filtering::Processor<Processor>(roberts_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 1));
         processors.push_back(Filtering::Processor<Processor>(roberts_ssse3, Constraint(CPU_SSSE3, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 2));
      }
      else if ( parameters["mode"].toString() == "laplace" )
      {
         print(LOG_DEBUG, "Edge : using laplace detector");
         processors.push_back(Filtering::Processor<Processor>(laplace_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
         processors.push_back(Filtering::Processor<Processor>(laplace_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 1));
         processors.push_back(Filtering::Processor<Processor>(laplace_ssse3, Constraint(CPU_SSSE3, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 2));
      }
      else if ( parameters["mode"].toString() == "cartoon" )
      {
         print(LOG_DEBUG, "Edge : using cartoon detector");
         processors.push_back(Filtering::Processor<Processor>(cartoon_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
         processors.push_back(Filtering::Processor<Processor>(cartoon_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 2));
      }
      else if ( parameters["mode"].toString() == "min/max" )
      {
         print(LOG_DEBUG, "Edge : using morphologic detector");
         processors.push_back(Filtering::Processor<Processor>(morpho_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
         processors.push_back(Filtering::Processor<Processor>(morpho_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 2));
      }
      else if ( parameters["mode"].toString() == "prewitt" )
      {
         print(LOG_DEBUG, "Edge : using prewitt detector");
         processors.push_back(Filtering::Processor<Processor>(prewitt_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
         processors.push_back(Filtering::Processor<Processor>(prewitt_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 1));
         processors.push_back(Filtering::Processor<Processor>(prewitt_ssse3, Constraint(CPU_SSSE3, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 2));
      }
      else if ( parameters["mode"].toString() == "hprewitt" )
      {
         print(LOG_DEBUG, "Edge : using hprewitt detector");
         processors.push_back(Filtering::Processor<Processor>(half_prewitt_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
         processors.push_back(Filtering::Processor<Processor>(half_prewitt_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 1));
         processors.push_back(Filtering::Processor<Processor>(half_prewitt_ssse3, Constraint(CPU_SSSE3, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 16), 2));
      }
      else
      {
         print(LOG_DEBUG, "Edge : using custom detector");
         auto coeffs = Parser::getDefaultParser().parse(parameters["mode"].toString(), " ").getExpression();
         memset(matrix, 0, sizeof(matrix));
         int nNegative = 0, nPositive = 0;
         for ( int i = 0; i < 9; i++ )
         {
            Short coefficient;

            if ( !coeffs.size() )
            {
               error = "invalid mode";
               return;
            }

            print( LOG_DEBUG, "%i\n", coeffs.size());
            matrix[i] = coefficient = static_cast<Short>(coeffs.front().getValue(0,0,0));

            nNegative += coefficient < 0 ? -coefficient : 0;
            nPositive += coefficient > 0 ? +coefficient : 0;
            coeffs.pop_front();
         }
         int nSum = max<int>(nNegative, nPositive);

         bool isAsmOk = true;
         if (coeffs.size())
            nSum = static_cast<Short>(coeffs.front().getValue(0,0,0));

         /* disable asm if sum of coefficients indicates a risk of overflow */
         if (nNegative > 128 || nPositive > 128)
            isAsmOk = false;
         
         /* find the upper power of 2, if possible */
         int i;
         for (i = 0; i < 15; i++)
         {
            if ((1 << i) >= nSum)
            {
               if (!coeffs.size())
                  nSum = 1 << i;
               break;
            }
         }

         matrix[9] = static_cast<Short>(nSum);

         if (i >= 15)
            error = "Too high divisor, please specify a divisor between 1 and 32767";
         if (1 << i != matrix[9])
            isAsmOk = false;
         if (matrix[9] <= 0)
            error = "Divisor must be positive";

         print(LOG_DEBUG, "Edge : %3i %3i %3i\n"
                        "       %3i %3i %3i\n"
                        "       %3i %3i %3i\n"
                        "normalize: %3i", matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5], matrix[6], matrix[7], matrix[8], matrix[9]);
         processors.push_back(Filtering::Processor<Processor>(convolution_c, Constraint(CPU_NONE, 1, 1, 1, 1), 0));
         if (isAsmOk)
         {
            processors.push_back(Filtering::Processor<Processor>(convolution_sse2, Constraint(CPU_SSE2, 8, 1, 1, 1), 2));
         }
      }
   }

   InputConfiguration &input_configuration() const { return OneFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_edge";

      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(String("sobel"), "mode"));
      signature.add(Parameter(10, "thY1"));
      signature.add(Parameter(10, "thY2"));
      signature.add(Parameter(10, "thC1"));
      signature.add(Parameter(10, "thC2"));

      return add_defaults( signature );
   }

};

} } } } }

#endif