
#include "filemanager.h"
#include "controller.h"
#include "misc.h"
#include <utility>
#include <fstream>
#include <sstream>

std::vector<Atom> FileMngr::readFileXYZ(const std::string& filepath){
  // Validate and read atom line
  // If invalid, returns a pair, whose first value is an empty string
  auto readAtomLine = [](const std::string& line){
    
    // Atom lines have four entries, separated by one or more white spaces.
    const std::vector<std::string> substrings = splitLine(line);
    if (substrings.size() != 4) {return SymbolPositionPair();}
    
    // If first item cannot be converted to a valid element symbol return false
    std::string symbol = strToValidSymbol(substrings[0]);
    if (symbol.empty()){return SymbolPositionPair();}

    // Validate second to fourth item are numeric
    std::array<double,3> position;
    for (size_t i=1; i<4; ++i){
      try {
        size_t str_pos = 0; // Last string position successfully evaluated by std::stod()
        position[i-1] = std::stod(substrings[i], &str_pos);
        if (substrings[i].size() != str_pos) {return SymbolPositionPair();}
      } 
      catch (const std::invalid_argument& e) {return SymbolPositionPair();}
    }
    return std::make_pair(symbol, position);
  };

  std::string line;
  std::ifstream inp_file(filepath);

  bool invalid_entry_encountered = false;
  bool first_atom_line_encountered = false;

  std::vector<Atom> atom_list;
  while(getline(inp_file,line)){
    if (line.empty()){} // Skip blank lines in any case
    const auto sp_pair = readAtomLine(line);
    if (sp_pair.first.empty()){
      // Display an error if the atom lines are interrupted by a non-blank, non-atom line
      if (first_atom_line_encountered){
        invalid_entry_encountered = true;
      }
      continue;
    }
    first_atom_line_encountered = true;

    atom_list.push_back(Atom(sp_pair));
  }
  inp_file.close();
  if (invalid_entry_encountered){Ctrl::getInstance()->displayErrorMessage(105);}
  return atom_list;
}

// split line into substrings when separated by whitespaces
std::vector<std::string> FileMngr::splitLine(const std::string& line){
  std::istringstream iss(line);
  std::vector<std::string> substrings((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
  return substrings;
}

// Reads a string and converts it to valid atom symbol: first character uppercase followed by lowercase characters
std::string FileMngr::strToValidSymbol(std::string str){
  StrMngr::removeWhiteSpaces(str);
  StrMngr::removeEOL(str);
  // Return empty if str is empty or begins with non-alphabetic character
  if (str.size() == 0 || !isalpha(str[0])){return "";}
  // Only for first character in sequence, convert to uppercase
  str[0] = toupper(str[0]);
  for (size_t i = 1; i < str.size(); i++) {
    if (isalpha(str[i])) {
      str[i] = tolower(str[i]);
    }
  }
  return str;
}

