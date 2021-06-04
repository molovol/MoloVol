
#include "special_chars.h"
#include <iostream>
#include <cassert>

std::wstring Symbol::angstrom(){
  return L"\u212B";
}

std::wstring Symbol::squared(){
  return L"\u00B2";
}

std::wstring Symbol::cubed(){
  return L"\u00B3";
}

// from a numeric char, return the unicode encoded subscripts
wchar_t Symbol::digitSubscript(char digit){
  wchar_t subscript = 0x2080;           // unicode encoding of subscript "0"
  int value = digit - '0';              // convert numeric char to corresponding int
  subscript = subscript + value;        // adding a digit to subscript "0" return subscript of that digit
  return subscript;
}

std::wstring Symbol::generateChemicalFormulaUnicode(std::string chemical_formula){
  std::wstring chemical_formula_unicode;
  for (size_t i = 0; i < chemical_formula.size(); i++){
    if (isalpha(chemical_formula[i])){
      // it is much simpler to convert string to wxString to wstring than directly from string to wstring
      wxString buffer(chemical_formula[i]);
      chemical_formula_unicode.append(buffer.wxString::ToStdWstring());
    }
    else {
      chemical_formula_unicode += digitSubscript(chemical_formula[i]);
    }
  }
  return chemical_formula_unicode;
}


