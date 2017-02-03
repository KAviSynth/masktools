#ifndef __Mt_Clamp_H__
#define __Mt_Clamp_H__

#include "../../../common/base/filter.h"

namespace Filtering { namespace MaskTools { namespace Filters { namespace Support { namespace Clamp {

typedef void(Processor)(Byte *pDst, ptrdiff_t nDstPitch, const Byte *pSrc1, ptrdiff_t nSrc1Pitch, const Byte *pSrc2, ptrdiff_t nSrc2Pitch, int nWidth, int nHeight, int nOvershoot, int nUndershoot);

Processor clamp_c;
extern Processor *clamp_sse2;
extern Processor *clamp_asse2;

class Clamp : public MaskTools::Filter
{
    int overshoot, undershoot;
    ProcessorList<Processor> processors;

protected:
    virtual void process(int n, const Plane<Byte> &dst, int nPlane, const Filtering::Frame<const Byte> frames[3], const Constraint constraints[3]) override
    {
        UNUSED(n);
        processors.best_processor(constraints[nPlane])(dst.data(), dst.pitch(),
            frames[0].plane(nPlane).data(), frames[0].plane(nPlane).pitch(),
            frames[1].plane(nPlane).data(), frames[1].plane(nPlane).pitch(),
            dst.width(), dst.height(), overshoot, undershoot);
    }

public:
    Clamp(const Parameters &parameters) : MaskTools::Filter(parameters, FilterProcessingType::INPLACE)
    {
        undershoot = parameters["undershoot"].toInt();
        overshoot = parameters["overshoot"].toInt();

        /* add the processors */
        processors.push_back(Filtering::Processor<Processor>(&clamp_c, Constraint(CPU_NONE, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 1), 0));
        processors.push_back(Filtering::Processor<Processor>(clamp_sse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_NONE, 1), 1));
        processors.push_back(Filtering::Processor<Processor>(clamp_asse2, Constraint(CPU_SSE2, MODULO_NONE, MODULO_NONE, ALIGNMENT_16, 16), 2));
    }

    InputConfiguration &input_configuration() const { return InPlaceThreeFrame(); }

    static Signature filter_signature()
    {
        Signature signature = "mt_clamp";

        signature.add(Parameter(TYPE_CLIP, ""));
        signature.add(Parameter(TYPE_CLIP, ""));
        signature.add(Parameter(TYPE_CLIP, ""));
        signature.add(Parameter(0, "overshoot"));
        signature.add(Parameter(0, "undershoot"));

        return add_defaults(signature);
    }
};

} } } } }

#endif
