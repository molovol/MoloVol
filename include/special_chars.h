#ifndef SPECIAL_CHARS_H

#define SPECIAL_CHARS_H

#include <string>
#include <wx/wx.h>

class Symbol{
  public:
    static std::wstring angstrom();
    static std::wstring squared();
    static std::wstring cubed();
    static wchar_t digitSubscript(char digit);
    static std::wstring generateChemicalFormulaUnicode(std::string);
};

#endif


