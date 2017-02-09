#ifndef __Mt_Lut_H__
#define __Mt_Lut_H__

#include "../../../common/base/filter.h"
#include "../../../../common/parser/parser.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Lut { namespace Single {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, int nWidth, int nHeight, const Byte lut[256]);

Processor lut_c;

class Lut : public MaskTools::Filter
{
   Byte luts[3][256];

protected:
    virtual void process(int n, const Plane<Byte> &dst, int nPlane, const ::Filtering::Frame<const Byte> frames[3], const Constraint constraints[3]) override
    {
        UNUSED(n);
        UNUSED(constraints);
        UNUSED(frames);
        lut_c(dst.data(), dst.pitch(), dst.width(), dst.height(), luts[nPlane]);
    }

public:
   Lut(const Parameters &parameters) : MaskTools::Filter( parameters, FilterProcessingType::INPLACE )
   {
      static const char *expr_strs[] = { "yExpr", "uExpr", "vExpr" };

      Parser::Parser parser = Parser::getDefaultParser().addSymbol(Parser::Symbol::X);

      /* compute the luts */
      for ( int i = 0; i < 3; i++ )
      {
          if (operators[i] != PROCESS) {
              continue;
          }

          if (parameters[expr_strs[i]].undefinedOrEmptyString() && parameters["expr"].undefinedOrEmptyString()) {
              operators[i] = NONE; //inplace
              continue;
          }

          if ( parameters[expr_strs[i]].is_defined() ) 
              parser.parse(parameters[expr_strs[i]].toString(), " ");
          else
              parser.parse(parameters["expr"].toString(), " ");

          Parser::Context ctx(parser.getExpression());

          if ( !ctx.check() )
          {
              error = "invalid expression in the lut";
              return;
          }

          for ( int x = 0; x < 256; x++ )
              luts[i][x] = ctx.compute_byte(x, 0.0f);
      }
   }

   InputConfiguration &input_configuration() const { return InPlaceOneFrame(); }

   static Signature filter_signature()
   {
      Signature signature = "mt_lut";

      signature.add(Parameter(TYPE_CLIP, ""));
      signature.add(Parameter(String("x"), "expr"));
      signature.add(Parameter(String("x"), "yExpr"));
      signature.add(Parameter(String("x"), "uExpr"));
      signature.add(Parameter(String("x"), "vExpr"));

      return add_defaults( signature );
   }
};


} } } } }

#endif