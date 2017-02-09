#ifndef __Mt_Params_H__
#define __Mt_Params_H__

#include "../../../common/utils/utils.h"

namespace Filtering { namespace MaskTools {

typedef enum {
   NONE,
   MEMSET,
   COPY,
   PROCESS,
   COPY_SECOND,
   COPY_THIRD,

} Mode;

class Operator {

   Mode mode;
   int nValue;      /* only for mode == memset */

public:

   Operator() : mode(NONE), nValue(-1) {}
   Operator(Mode _mode, int _nValue = -1) : mode(_mode), nValue(_mode == MEMSET ? _nValue : -1) {}
   Operator(int nValue)
   {
      this->nValue = -1;
      switch ( nValue )
      {
      case 5 : mode = COPY_THIRD; break;
      case 4 : mode = COPY_SECOND; break;
      case 3 : mode = PROCESS; break;
      case 2 : mode = COPY; break;
      case 1 : mode = NONE; break;
      default: mode = MEMSET; this->nValue = -nValue; break;
      }
   }

   bool operator==(const Operator &operation) const { return mode == operation.mode; }
   bool operator!=(const Operator &operation) const { return mode != operation.mode; }
   bool operator==(Mode _mode) const { return _mode == this->mode; }
   bool operator!=(Mode _mode) const { return _mode != this->mode; }
   Mode getMode() const { return mode; }
   int value() const { return nValue; }
};

} } // namespace MaskTools, Filtering

#endif
