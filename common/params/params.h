#ifndef __Common_Params_H__
#define __Common_Params_H__

#include "../clip/clip.h"
#include "../constraints/constraints.h"

namespace Filtering {

typedef enum {

   TYPE_INT,
   TYPE_FLOAT,
   TYPE_STRING,
   TYPE_BOOL,
   TYPE_CLIP,

   TYPE_UNDEFINED,

} Type;

/* generic """polymorphic""" class, whose value can be defined or not */
class Value {

   Type     type;
   bool     defined;

   PClip    val_clip;
   int      val_int;
   String   val_string;
   bool     val_bool;
   double   val_float;

public:

   Value() : type(TYPE_UNDEFINED), defined(false) { };
   Value(int n) : type(TYPE_INT), defined(true), val_int(n), val_float((float)n) { }
   Value(double d) : type(TYPE_FLOAT), defined(true), val_float(d), val_int((int)d) { }
   Value(float d) : type(TYPE_FLOAT), defined(true), val_float(d), val_int((int)d) { }
   Value(const String &s) : type(TYPE_STRING), defined(true), val_string(s) { }
   Value(bool b) : type(TYPE_BOOL), defined(true), val_bool(b) { }
   Value(const PClip& c) : type(TYPE_CLIP), defined(true), val_clip(c) { }
   Value(Type t) : type(t), defined(false) { }
   Value(const Value &v) : type(v.type), defined(v.defined), val_clip(v.val_clip), val_int(v.val_int), 
                            val_string(v.val_string), val_bool(v.val_bool), val_float(v.val_float) { }

   Type get_type() const { return type; }

   int toInt() const { return val_int; }
   double toFloat() const { return val_float; }
   bool toBool() const { return val_bool; }
   String toString() const { return val_string; }
   PClip toClip() const { return val_clip; }

   bool is_defined() const { return defined; }
   void set_defined(bool d) { defined = d; }

   bool undefinedOrEmptyString() {
       return !is_defined() || val_string.empty();
   }
};

/* parameter will be used to defined a filter signature, but also to contain the actual value of
   a parameter once the filter has been called */
class Parameter {

   Value       value;
   String      name;
   bool        intAlternative; // compatibility: float parameter appears as int in the secondary signature

public:

   Parameter() : value(), name(""), intAlternative(false) { }
   Parameter(Type type) : value(type), name(""), intAlternative(false) { }
   Parameter(const Value &value) : value(value), name(""), intAlternative(false) { }
   Parameter(const Value &value, const String &name, bool intAlternative) : value(value), name(name), intAlternative(intAlternative) { }

   void set_defined(bool d) { value.set_defined( d ); }
   bool isNamed() const { return !name.empty(); }
   Type getType() const { return value.get_type(); }
   String getName() const { return name; }
   Value getValue() const { return value; }
   bool getIntAlternative() const { return intAlternative; }
};

/* list of parameters */
class Parameters : public std::vector<Parameter> {

public:

    /* handy accessor */
    Value operator[](const String &_name) const
    {
        for(auto &p :*this) {
            if (_name == p.getName()) {
                return p.getValue();
            }
        }
        return Value();
    }

    Value operator[](int n) const
    {
        return std::vector<Parameter>::operator [](n).getValue();
    }
};

/* signature of the filter */
class Signature {

    Parameters              parameters;
    String                  name;

public:

    Signature(const String &_name) : name(_name) { }
    void add(const Parameter &parameter) { parameters.push_back(parameter); }

    String getName() const { return name; }
    Value operator[](const String &_name) const { return parameters[_name]; }
    Parameter operator[](int index) const { return parameters.at(index); }
    int count() const { return int(parameters.size()); }
};

} // namespace Filtering

#endif
