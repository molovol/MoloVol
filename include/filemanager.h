#ifndef FILEMANAGER_H

#define FILEMANAGER_H

#include "atom.h"
#include <vector>
#include <string>

// The file manager is an external component that compartmentalises the code used for
// importing data. It works for the Model class.
namespace FileMngr{
  typedef std::pair<std::string,std::array<double,3>> SymbolPositionPair;

  // Main functions
  std::vector<Atom> readFileXYZ(const std::string&);

  // Aux functions
  std::string strToValidSymbol(std::string str);
  std::vector<std::string> splitLine(const std::string& line);
}

#endif

