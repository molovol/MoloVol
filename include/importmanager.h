#ifndef FILEMANAGER_H

#define FILEMANAGER_H

#include "atom.h"
#include <vector>
#include <string>

// The file manager is an external component that compartmentalises the code used for
// importing data. It works for the Model class.
namespace ImportMngr{
  typedef std::pair<std::string,std::array<double,3>> SymbolPositionPair;

  // Return struct
  struct UnitCell{
    typedef std::array<std::array<double,3>,3> MatR3;
    
    UnitCell() : space_group(""), sym_matrix_XYZ(std::vector<int>()), sym_matrix_fraction(std::vector<double>()) {};
  
    std::string space_group;
    std::array<double,6> parameters;
    std::array<std::array<double,3>,3> cart_matrix;
    std::vector<int> sym_matrix_XYZ;
    std::vector<double> sym_matrix_fraction;
  };

  // Main functions
  std::vector<Atom> readFileXYZ(const std::string&);
  std::pair<std::vector<Atom>,UnitCell> readFilePDB(const std::string&, bool);

  // Aux functions
  std::string strToValidSymbol(std::string str);
  std::vector<std::string> splitLine(const std::string& line);
}

#endif

