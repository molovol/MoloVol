#ifndef SPECIAL_CHARS_H

#define SPECIAL_CHARS_H

#include <string>
#include <cassert>

class Symbol{
  public:
    static std::string angstrom();
    static std::string cubed();
    static std::string subscript(int num);
    static std::string subscript(std::string num);

};

#endif


