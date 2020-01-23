#ifndef __Common_Avs2x_Filter_H__
#define __Common_Avs2x_Filter_H__

#include "params.h"
#include "../common/constraints/constraints.h"
#include <avs/cpuid.h>

namespace Filtering { namespace Avisynth2x {

static CpuFlags AvsToInternalCpuFlags(int avsCpuFlags) {
  int flags = CPU_NONE;
  if (avsCpuFlags & CPUF_AVX2) flags |= CPU_AVX2;
  if (avsCpuFlags & CPUF_AVX) flags |= CPU_AVX;
  if (avsCpuFlags & CPUF_SSE4_2) flags |= CPU_SSE4_2;
  if (avsCpuFlags & CPUF_SSE4_1) flags |= CPU_SSE4_1;
  if (avsCpuFlags & CPUF_SSSE3) flags |= CPU_SSSE3;
  if (avsCpuFlags & CPUF_SSE3) flags |= CPU_SSE3;
  if (avsCpuFlags & CPUF_SSE2) flags |= CPU_SSE2;
  return flags;
}


template<class T>
class Filter : public GenericVideoFilter
{
    T _filter;
    Signature signature;
    int inputConfigSize; // to prevent MT static init problems with /Zc:threadSafeInit- settings for WinXP

    static AVSValue __cdecl _create(AVSValue args, void *user_data, IScriptEnvironment *env)
    {
      UNUSED(user_data);
      return new Filter<T>(args[0].AsClip(), GetParameters(args, T::filter_signature(), env), env);
    }
public:
    Filter(::PClip child, const Parameters &parameters, IScriptEnvironment *env) : GenericVideoFilter(child), _filter(parameters, AvsToInternalCpuFlags(env->GetCPUFlags())), signature(T::filter_signature())
    {
        inputConfigSize = _filter.input_configuration().size();
        // This is a warning left here intentionally, why multithreading and threadSafeInit- for winXp causes big troubles sometimes.
        // When the above line is missing, problems kick in _filter.get_frame(n, destination, env)
        // Why: "input_configuration" has static initializer that has problems in multithreaded environment
        // when the XP compatible /Zc:threadSafeInit- switch is used for compiling in Visual Studio
        // https://docs.microsoft.com/en-us/cpp/build/reference/zc-threadsafeinit-thread-safe-local-static-initialization
        // In non-threadSafeInit mode (XP) when get_frame is called in multithreaded environment,
        // the initialization is started in thread#1 and at specific timing conditions (e.g. debug mode is not OK) this initialization is not finished yet when
        // thread#2 also calls into input_configuration().size().
        // Real life problems: the proper size value is "2", but for thread#2 still "0" is reported!
        // This resulted in zero sized local PVideoFrame array to be allocated, but later, when the initialization
        // is finished in an other thread, size() turnes into "2". It needs only some 1/10000th seconds, but the problem is there by then.
        // This zero sized array is then indexed with the proper size of "2" from 0..1 -> Access Violation
        // Debuglog: Masktools2 Getframe #1
        //  Masktools2 Getframe #0
        //  Masktools2 Getframe 0, clipcount = 0 // should be 2!!
        //  Masktools2 Getframe 0, clipcount2 = 2 // meanwhile the init was done in the background, we get 2 which is correct
        //  Masktools2 Getframe 1, clipcount = 2
        if (_filter.is_error())
        {
            env->ThrowError((signature.getName() + " : " + _filter.get_error()).c_str());
        }
    }

    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment *env) override
    {

      // v2.2.15-
      // lut, lutxy, lutxyz, lutxyza: 'use_expr' parameters option to pass expression to the Expr filter of avs+
      // which is much faster than masktools2 realtime interpreted pixel-by-pixel calculation
      // expr_need_process[4] and expr_list[4] are all filled for avs+ requirements
      // copy-plane or fill operators are also converted to valid expression strings (single "x" or like "128")
      const bool effective_expr_need_process = _filter.expr_need_process[0] || _filter.expr_need_process[1] || _filter.expr_need_process[2] || _filter.expr_need_process[3];
      if (effective_expr_need_process) {
        const int planecount = plane_counts[_filter.colorspace()];

        // lut, lutxy, lutxyz, lutxyza: InPlaceTwoFrame: size is less by one, first clip is explicit
        int realInputConfigSize = inputConfigSize + 1;
        int param_length = realInputConfigSize + planecount + 1 + 1;

        // c+s+[format]s[optAvx2]b[optSingleMode]b[optSSE2]b[scale_inputs]s[clamp_float]i
        // Incompatibility note: Avisynth+ 3.4 treates clamp_float as bool, matching masktools <= 2.2.19.
        // In masktools 2.2.20 this parameter type was changed to integer
        const char *arg_names[4 + 4 + 2]; // worst case
        for (int i = 0; i < param_length - 2; i++)
          arg_names[i] = nullptr;
        arg_names[param_length - 2] = (const char *)"scale_inputs";
        arg_names[param_length - 1] = (const char *)"clamp_float";

        std::vector<AVSValue> new_args(param_length, AVSValue());

        for (int i = 0; i < realInputConfigSize; i++)
          new_args[i] = _filter.get_childs()[i]->get_avs_clip();
        for (int i = 0; i < planecount; i++)
          new_args[i + realInputConfigSize] = _filter.expr_list[i].c_str();

        new_args[param_length - 2] = _filter.expr_scale_inputs.c_str();
        new_args[param_length - 1] = _filter.expr_clamp_float;

        AVSValue clip_out = env->Invoke("Expr", AVSValue(new_args.data(), param_length), arg_names);
        PVideoFrame dst = clip_out.AsClip()->GetFrame(n, env);
        return dst;
      }

      // filter is not a lut or lut w/o "Expr" call
      PVideoFrame dst = _filter.is_in_place() ? child->GetFrame(n, env) : env->NewVideoFrame(vi);

      if (_filter.is_in_place()) {
        env->MakeWritable(&dst);
      }

      Frame<Byte> destination = dynamic_cast<Clip *>((Filtering::Clip *)_filter.get_childs()[0].get())->ConvertTo<Byte>(dst);

      _filter.get_frame(n, destination, env);

      return dst;
    }

    int __stdcall SetCacheHints(int cachehints, int frame_range) override {
        return cachehints == CACHE_GET_MTMODE ? MT_NICE_FILTER : 0;
    }

    static void create(IScriptEnvironment *env)
    {
#ifdef MT_DUAL_SIGNATURES
      String signature1 = SignatureToString(T::filter_signature(), true); // integer
      env->AddFunction(T::filter_signature().getName().c_str(), signature1.c_str(), _create, NULL);
      // order is important! 
      String signature2 = SignatureToString(T::filter_signature(), false); // float
      if (signature1 != signature2)
        env->AddFunction(T::filter_signature().getName().c_str(), signature2.c_str(), _create, NULL);
      // compatibility: alternative parameter list, int and float
      // or else Avisynth would find the function from another, earlier loaded masktools2 version
#else
      String signature = SignatureToString(T::filter_signature(), false); // float
      env->AddFunction(T::filter_signature().getName().c_str(), signature.c_str(), _create, NULL);
#endif
    }
};

} } // namespace Avisynth2x, Filtering

#endif
