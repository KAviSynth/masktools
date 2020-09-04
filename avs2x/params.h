#ifndef __Common_Avs2x_Params_H__
#define __Common_Avs2x_Params_H__

#include "clip.h"
#include "../common/params/params.h"

namespace Filtering { namespace Avisynth2x {

static Value AVSValueToValue(const AVSValue& value, const Parameter &param)
{
    switch (param.getType())
    {
    case TYPE_INT: return Value(value.AsInt());
    case TYPE_STRING: return Value(String(value.AsString()));
    case TYPE_BOOL: return Value(value.AsBool());
    case TYPE_CLIP: return std::shared_ptr<Filtering::Clip>(new Clip(value.AsClip()));
    case TYPE_FLOAT: return Value(value.AsFloat());
    default: return Value();
    }
}

static String ParameterToString(const Parameter &parameter, bool intAlternative)
{
    String str = "";

    if (parameter.isNamed())
        str.append("[").append(parameter.getName()).append("]");

    switch (parameter.getType())
    {
    case TYPE_INT: return str.append("i");
    case TYPE_FLOAT: return intAlternative ? str.append("i") : str.append("f");
    case TYPE_STRING: return str.append("s");
    case TYPE_CLIP: return str.append("c");
    case TYPE_BOOL: return str.append("b");
    default: assert(0); return "";
    }
}

String SignatureToString(const Signature &signature, bool intAlternative)
{
    String str;

    for (int i = 0; i < signature.count(); i++)
      str.append(ParameterToString(signature[i], intAlternative));

    return str;
}

Parameter GetParameter(const AVSValue &value, const Parameter &default_param)
{
    if (value.Defined()) {
        return Parameter(AVSValueToValue(value, default_param), default_param.getName(), false); // false: n/a
    }

    Parameter parameter = default_param;
    parameter.set_defined(false);

    return parameter;
}

Parameters GetParameters(const AVSValue &args, const Signature &signature, bool has_avs_expr_support, IScriptEnvironment *env)
{
    Parameters parameters;

    UNUSED(env);

    for (int i = 0; i < signature.count(); i++) {
      if (signature[i].getName() == "use_expr" && !has_avs_expr_support) {
        // v2.26 when no "Expr" support in avisynth then set use_expr to undefined (default 0)
        parameters.push_back(GetParameter(AVSValue(), signature[i]));
      }
      else
        parameters.push_back(GetParameter(args[i], signature[i]));
    }

    return parameters;
}

} }

#endif
