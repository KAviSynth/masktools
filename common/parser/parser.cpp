#include "parser.h"

using namespace Filtering;

Parser::Parser::Parser()
{
}

Parser::Parser::Parser(const String &parsed_string, const String &separators)
{
   parse(parsed_string, separators);
}

Parser::Parser &Parser::Parser::addSymbol(const Symbol &symbol)
{
   symbols.push_back(symbol);

   return *this;
}

const Parser::Symbol *Parser::Parser::findSymbol(const String &value) const
{
    for (auto &symbol: symbols) {
        if (symbol.value == value || symbol.value2 == value) {
            return &symbol;
        }
    }

   return nullptr;
}

Parser::Symbol Parser::Parser::stringToSymbol(const String &value) const
{ 
    auto found = findSymbol(value);
    return found == nullptr
        ? Symbol(value, Symbol::NUMBER, 0, NULL)
        : *found;
}

Parser::Parser &Parser::Parser::parse(const String &parsed_string, const String &separators)
{
   this->parsed_string = parsed_string;

   size_t nPos = parsed_string.find_first_not_of(separators, 0);
   size_t nEndPos;

   elements.clear();

   while ( nPos != String::npos && (nEndPos = parsed_string.find_first_of(separators, nPos)) != String::npos )
   {
      elements.push_back(stringToSymbol(parsed_string.substr(nPos, nEndPos - nPos)));
      nPos = parsed_string.find_first_not_of(separators, nEndPos);
   }

   if ( nPos != String::npos )
      elements.push_back(stringToSymbol(parsed_string.substr(nPos)));

   return *this;
}

String Parser::Parser::getParsedString() const
{
   return parsed_string;
}

int Parser::Parser::count() const
{
   return elements.size();
}

Parser::Parser Parser::getDefaultParser()
{
   Parser parser;

   /* arithmetic operators */
   parser.addSymbol(Symbol::Addition).addSymbol(Symbol::Division).addSymbol(Symbol::Multiplication).addSymbol(Symbol::Substraction).addSymbol(Symbol::Modulo).addSymbol(Symbol::Power);
   /* comparison operators */
   parser.addSymbol(Symbol::Equal).addSymbol(Symbol::Equal2).addSymbol(Symbol::NotEqual).addSymbol(Symbol::Inferior).addSymbol(Symbol::InferiorStrict).addSymbol(Symbol::Superior).addSymbol(Symbol::SuperiorStrict);
   /* logic operators */
   parser.addSymbol(Symbol::And).addSymbol(Symbol::Or).addSymbol(Symbol::AndNot).addSymbol(Symbol::Xor);
   /* unsigned binary operators */
   parser.addSymbol(Symbol::AndUB).addSymbol(Symbol::OrUB).addSymbol(Symbol::XorUB).addSymbol(Symbol::NegateUB).addSymbol(Symbol::PosShiftUB).addSymbol(Symbol::NegShiftUB);
   /* signed binary operators */
   parser.addSymbol(Symbol::AndSB).addSymbol(Symbol::OrSB).addSymbol(Symbol::XorSB).addSymbol(Symbol::NegateSB).addSymbol(Symbol::PosShiftSB).addSymbol(Symbol::NegShiftSB);
   /* ternary operator */
   parser.addSymbol(Symbol::Interrogation);
   /* function */
   parser.addSymbol(Symbol::Abs).addSymbol(Symbol::Acos).addSymbol(Symbol::Asin).addSymbol(Symbol::Atan).addSymbol(Symbol::Cos).addSymbol(Symbol::Exp).addSymbol(Symbol::Log).addSymbol(Symbol::Sin).addSymbol(Symbol::Tan).addSymbol(Symbol::Min).addSymbol(Symbol::Max).addSymbol(Symbol::Clip);
   /* rounding */
   parser.addSymbol(Symbol::Round).addSymbol(Symbol::Floor).addSymbol(Symbol::Trunc).addSymbol(Symbol::Ceil);
   /* number */
   parser.addSymbol(Symbol::Pi);
   /* auto bitdepth conversion: BITDEPTH (bitdepth) and two functions */
   parser.addSymbol(Symbol::BITDEPTH).addSymbol(Symbol::UpscaleByShift).addSymbol(Symbol::UpscaleByStretch);
   /* special bit-depth adaptive constants */
   parser.addSymbol(Symbol::RANGE_HALF).addSymbol(Symbol::RANGE_MAX).addSymbol(Symbol::RANGE_SIZE);
   parser.addSymbol(Symbol::YMIN).addSymbol(Symbol::YMAX);
   parser.addSymbol(Symbol::CMIN).addSymbol(Symbol::CMAX);

   /* Symbol X, X and Y, X and Y and Z are added additionally at before processing lut expression */
   /* like this:       
      Parser::Parser parser = Parser::getDefaultParser().addSymbol(Parser::Symbol::X).addSymbol(Parser::Symbol::Y);
  */

   return parser;
}