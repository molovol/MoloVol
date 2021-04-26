
#include "special_chars.h"
#include <iostream>
#include <cassert>

std::wstring Symbol::angstrom(){
  return L"\u212B";
}

std::wstring Symbol::cubed(){
  return L"\u00B3";
}

// from an int, produce a string of utf-8 encoded subscripts
// TODO not used and untested
std::wstring Symbol::numSubscript(int num){
  std::wstring retval = L"";

  while (num > 0){
    wchar_t subscript = 0x2080;           // unicode encoding of subscript "0"
    int digit = num%10;
    subscript = subscript + digit;
    retval.push_back(subscript);
    num = num/10;
  }
  return retval;
}

// from a numeric string, produce a string of unicode encoded subscripts
// TODO not used and untested
std::wstring Symbol::numSubscript(std::string num){
  std::wstring retval = L"";

  // iterate over all chars in input
  for (char c : num){
    assert(c >= '0' && c <= '9');
    wchar_t subscript = 0x2080;           // unicode encoding of subscript "0"
    int digit = c - '0';                  // convert numeric char to corresponding int
    subscript = subscript + digit;        // adding a digit to subscript "0" return subscript of that digit
    retval.push_back(subscript);
  }
  return retval;
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


