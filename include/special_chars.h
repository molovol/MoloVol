#ifndef SPECIAL_CHARS_H

#define SPECIAL_CHARS_H

#include <string>
#include <wx/wx.h>

class Symbol{
  public:
    static std::wstring angstrom();
    static std::wstring squared();
    static std::wstring cubed();
    static std::wstring numSubscript(int num);
    static std::wstring numSubscript(std::string num);
    static wchar_t digitSubscript(char digit);
    static std::wstring generateChemicalFormulaUnicode(std::string);

};

#endif


