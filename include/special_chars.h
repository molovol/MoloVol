#ifndef SPECIAL_CHARS_H

#define SPECIAL_CHARS_H

#include <string>
#ifdef MOLOVOL_GUI
#include <wx/wx.h>
#endif
class Symbol{
  public:
    static std::wstring angstrom();
    static std::wstring squared();
    static std::wstring cubed();
    static wchar_t digitSubscript(char digit);
    static std::wstring generateChemicalFormulaUnicode(std::string);

    static void limit2ascii();
    static void allow_unicode();
  private:
    static bool s_ascii;
};

#endif


