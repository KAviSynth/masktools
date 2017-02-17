#ifndef __Mt_Lut_Functions_H__
#define __Mt_Lut_Functions_H__

namespace Filtering { namespace MaskTools { namespace Filters { namespace Lut {

typedef enum {

   NONIZER = 0,
   AVERAGER = 1,
   MINIMIZER = 2,
   MAXIMIZER = 3,
   DEVIATER = 4,
   RANGIZER = 5,
   MEDIANIZER = 6,
   MEDIANIZER4 = 7,
   MEDIANIZER6 = 8,
   MEDIANIZER2 = 9,

   NUM_MODES,

} MPROCESSOR_MODE;

#define EXPRESSION_SINGLE( realtime, base, mode1, mode2 ) &base< realtime, mode2 >
#define EXPRESSION_DUAL( realtime, base, mode1, mode2 ) &base< mode1, mode2 >

#define MPROCESSOR(realtime, expr, param, mode) \
{ \
   expr( realtime, param, mode, Nonizer ), \
   expr( realtime, param, mode, Averager<int> ), \
   expr( realtime, param, mode, Minimizer ), \
   expr( realtime, param, mode, Maximizer ), \
   expr( realtime, param, mode, Deviater<int> ), \
   expr( realtime, param, mode, Rangizer ), \
   expr( realtime, param, mode, Medianizer ), \
   expr( realtime, param, mode, MedianizerBetter<4> ), \
   expr( realtime, param, mode, MedianizerBetter<6> ), \
   expr( realtime, param, mode, MedianizerBetter<2> ), \
}

#define MPROCESSOR_SINGLE( base, realtime )     MPROCESSOR( realtime, EXPRESSION_SINGLE, base, )
#define MPROCESSOR_DUAL( base, mode ) MPROCESSOR( false, EXPRESSION_DUAL, base, mode )
// for dual: realtime n/a

static inline int ModeToInt(const String &mode)
{
   if ( mode == "average" || mode == "avg" )
      return AVERAGER;
   else if ( mode == "standard deviation" || mode == "std" )
      return DEVIATER;
   else if ( mode == "minimum" || mode == "min" )
      return MINIMIZER;
   else if ( mode == "maximum" || mode == "max" )
      return MAXIMIZER;
   else if ( mode == "range" )
      return RANGIZER;
   else if ( mode == "median" || mode == "med" )
      return MEDIANIZER4;
   else
      return NONIZER;
}

class Nonizer {

   Double *pdCoefficients;
   int nCoefficients;
   Double dTotalCoefficients;
   Double dSum;
   int nPosition;

public:
   Nonizer(const String &mode)
   {
      auto coefficients = Parser::getDefaultParser().parse(mode, " (),;").getExpression();
      nCoefficients = coefficients.size();
      pdCoefficients = new Double[nCoefficients];
      int i = 0;

      dTotalCoefficients = 0;

      while ( !coefficients.empty() )
      {
         dTotalCoefficients += (pdCoefficients[i++] = coefficients.front().getValue(0, 0, 0));
         coefficients.pop_front();
      }
   }
   ~Nonizer()
   {
      delete[] pdCoefficients;
   }
   void reset(int nValue = 255)
   {
      UNUSED(nValue);
      dSum = 0;
      nPosition = 0;
   }
   void add(int nValue)
   {
      if (nPosition < nCoefficients)
         dSum += pdCoefficients[nPosition++] * nValue;
   }
   Byte finalize() const
   {
      return clip<Byte, Int64>(convert<Int64, Double>(dSum / dTotalCoefficients));
   }
};

class Minimizer {

   int nMin;

public:

   Minimizer(const String &mode) { UNUSED(mode); }
   void reset() { nMin = 255; }
   void add(int nValue) { nMin = min<int>( nMin, nValue ); }
   Byte finalize() const { return static_cast<Byte>(nMin); }

};

class Medianizer {

   int nSize;
   int elements[256];

public:

   Medianizer(const String &mode) { UNUSED(mode); }
   void reset() { memset( elements, 0, sizeof( elements ) ); nSize = 0; }
   void reset(int nValue) { reset(); add( nValue ); }
   void add(int nValue) { elements[nValue]++; nSize++; }
   Byte finalize() const
   {
      int nCount = 0;
      int nIdx = -1;
      const int nLowHalf = (nSize + 1) >> 1;
      while ( nCount < nLowHalf )
         nCount += elements[++nIdx];

      if ( nSize & 1 ) /* nSize odd -> median belong to the middle class */
         return static_cast<Byte>(nIdx);
      else
      {
         if ( nCount >= nLowHalf + 1 ) /* nSize even, but middle class owns both median elements */
            return static_cast<Byte>(nIdx);
         else /* nSize even, middle class owns only one element, it's the lowest, so we search for the next one */
         {
            int nSndIdx = nIdx;
            while ( !elements[++nSndIdx] ) {}
            return static_cast<Byte>((nIdx + nSndIdx + 1) >> 1); /* and we return the average */
         }
      }
   }
};

template<int n>
class MedianizerBetter
{
   int nSize;
   int elements[256];
   int elements2[256 >> n];

public:
   MedianizerBetter(const String &mode) { UNUSED(mode); }
   void reset() { nSize = 0; memset( elements, 0, sizeof( elements ) ); memset( elements2, 0, sizeof( elements2 ) ); }
   void add(int nValue) { elements[nValue]++; elements2[nValue>>n]++; nSize++; }

   Byte finalize() const
   {
      int nCount = 0;
      int nIdx = -1;
      const int nLowHalf = (nSize + 1) >> 1;

      while ( nCount < nLowHalf )
         nCount += elements2[++nIdx]; /* low resolution search */

      nCount -= elements2[nIdx];
      nIdx <<= n;
      nIdx--;

      while ( nCount < nLowHalf )
         nCount += elements[++nIdx]; /* high resolution search */

      if ( nSize & 1 ) /* nSize odd -> median belong to the middle class */
         return static_cast<Byte>(nIdx);
      else
      {
         if ( nCount >= nLowHalf + 1 ) /* nSize even, but middle class owns both median elements */
            return static_cast<Byte>(nIdx);
         else /* nSize even, middle class owns only one element, it's the lowest, so we search for the next one */
         {
            int nSndIdx = nIdx;
            while ( !elements[++nSndIdx] ) {}
            return static_cast<Byte>((nIdx + nSndIdx + 1) >> 1); /* and we return the average */
         }
      }
   }
};

class Maximizer {

   int nMax;

public:

   Maximizer(const String &mode) { UNUSED(mode); }
   void reset() { nMax = 0; }
   void add(int nValue) { nMax = max<int>( nMax, nValue ); }
   Byte finalize() const { return static_cast<Byte>(nMax); }

};

class Rangizer {

   int nMin, nMax;

public:

   Rangizer(const String &mode) { UNUSED(mode); }
   void reset() { nMin = 255; nMax = 0; }
   void add(int nValue) { nMin = min<int>( nMin, nValue ); nMax = max<int>( nMax, nValue ); }
   Byte finalize() const { return static_cast<Byte>(nMax - nMin); }

};

template<typename T>
class Averager
{

   T nSum, nCount;

public:

   Averager(const String &mode) { UNUSED(mode); }
   void reset() { nSum = 0; nCount = 0; }
   void add(int nValue) { nSum += nValue; nCount++; }
   Byte finalize() const { return static_cast<Byte>(rounded_division<T>( nSum, nCount )); }

};

template<typename T>
class Deviater
{
   T nSum, nCount;
   Int64 nSum2;

public:

   Deviater(const String &mode) { UNUSED(mode); }
   void reset() { nSum = 0; nSum2 = 0; nCount = 0; }
   void reset(int nValue) { nSum = nValue; nSum2 = nValue * nValue; nCount = 1; }
   void add(int nValue) { nSum += nValue; nSum2 += nValue * nValue; nCount++; }
   Byte finalize() const { return convert<Byte, Double>( sqrt( ( Double( nSum2 ) * nCount - Double( nSum ) * Double( nSum ) ) / ( Double( nCount ) * nCount ) ) ); }

};

} } } }

#endif
