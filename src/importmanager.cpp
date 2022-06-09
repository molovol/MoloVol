
#include "importmanager.h"
#include "controller.h"
#include "misc.h"
#include <map>
#include <utility>
#include <iostream>
#include <fstream>
#include <sstream>

////////////////
// XYZ IMPORT //
////////////////
std::vector<Atom> ImportMngr::readFileXYZ(const std::string& filepath){
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

////////////////
// PDB IMPORT //
////////////////
std::pair<std::vector<Atom>,ImportMngr::UnitCell> ImportMngr::readFilePDB(const std::string& filepath, bool include_hetatm){
  // LAMBDA DEFINITIONS
  // Follows the official specifications for PDB files as detailed in "Protein 
  // Data Bank Contents Guide: Atomic Coordinate Entry Format Description" Version 3.3
  // http://www.wwpdb.org/documentation/file-format-content/format33/v3.3.html
  typedef std::map<std::string,std::pair<int,int>> FieldColumns;
  static const FieldColumns s_atom_fields = {
    {"lineLength", {0,80}}, {"record", {0, 6}}, {"serial", {6, 5}}, {"name", {12, 4}}, {"altLoc", {16, 1}}, 
    {"resName", {17, 3}}, {"chainID", {21, 1}}, {"resSeq", {22, 4}}, {"iCode", {26, 1}}, 
    {"x", {30, 8}}, {"y", {38, 8}}, {"z", {46, 8}}, {"occupancy", {54, 6}}, {"tempFactor", {60, 6}}, 
    {"element", {76, 2}}, {"charge", {78, 2}}};

  static const FieldColumns s_cryst1_fields = {
    {"lineLength", {0, 70}}, {"record", {0, 6}}, {"a", {6, 9}}, {"b", {15, 9}}, {"c", {24, 9}}, {"alpha", {33, 7}},
    {"beta", {40, 7}}, {"gamma", {47, 7}}, {"sGroup", {55, 11}}, {"z", {66, 4}}};

  auto sectionFields = [](std::string line, const FieldColumns& field_map){
    // line may be shorter than specified by the PDB standard, but still valid if
    // only whitespace characters are missing. Append whitespaces to line until it
    // has the correct length and valid date the content later
    StrMngr::extendToLength(line,field_map.at("lineLength").second);

    std::map<std::string,std::string> fields;
    for (const auto& entry : field_map){
      if (entry.first == "lineLength"){continue;}
      fields[entry.first] = line.substr(entry.second.first, entry.second.second);
    }
    return fields;
  };

  auto extractRecordName = [](std::string line){
    // Make sure line is at least 6 characters long
    StrMngr::extendToLength(line, 6);
    line = line.substr(0,6);
    StrMngr::removeWhiteSpaces(line);
    return line;
  };
  // END OF LAMBDA DEFINITIONS

  std::string line;
  std::ifstream inp_file(filepath);
  bool invalid_symbol_detected = false;
  bool invalid_cell_params = false;
  bool invalid_atom_line = false;

  std::vector<Atom> atom_list;
  UnitCell uc;
  while(getline(inp_file,line)){
    StrMngr::removeEOL(line);
    if (extractRecordName(line) == "ATOM" || (include_hetatm && extractRecordName(line) == "HETATM")){
      // Partition line according to the fields specified by the PDB file standard
      const std::map<std::string,std::string> fields = sectionFields(line, s_atom_fields);

      // Evaluate atom symbol
      std::string symbol = strToValidSymbol(fields.at("name"));
      if (symbol.empty()){
        invalid_symbol_detected = true;
        continue;
      }
      
      // Evaluate atom coordinates
      std::array<double,3> coord;
      size_t i = 0;
      bool invalid_coord = false;
      for (std::string k : {"x", "y", "z"}){
        try{coord[i] = std::stod(fields.at(k));}
        catch (const std::invalid_argument& e){
          invalid_coord = true;
          invalid_atom_line = true;
          break;
        }
        ++i;
      }
      if (invalid_coord){continue;}

      // Evaluate charge
      signed charge = 0;
      // Avoid exception due to empty string
      std::string str_charge = fields.at("charge");
      StrMngr::removeWhiteSpaces(str_charge);
      if (!str_charge.empty()){
        try{
          charge = std::stoi(fields.at("charge"));
        }
        catch (const std::invalid_argument& e){}
      }

      // Store symbol and coordinates
      atom_list.push_back(Atom(std::make_pair(symbol, coord), charge));
    }
    else if (extractRecordName(line) == "CRYST1"){
      // Partition line according to the fields specified by the PDB file standard
      const std::map<std::string,std::string> fields = sectionFields(line, s_cryst1_fields);

      // Evaluate unit cell parameters
      size_t i = 0;
      for (std::string k : {"a", "b", "c", "alpha", "beta", "gamma"}){
        try{uc.parameters[i] = std::stod(fields.at(k));}
        catch (const std::invalid_argument& e){invalid_cell_params = true;}
        ++i;
      }
      uc.space_group = fields.at("sGroup");
      StrMngr::removeWhiteSpaces(uc.space_group);
    }
  }
  inp_file.close();
  if (invalid_symbol_detected){Ctrl::getInstance()->displayErrorMessage(105);}
  if (invalid_cell_params){Ctrl::getInstance()->displayErrorMessage(112);}
  if (invalid_atom_line){Ctrl::getInstance()->displayErrorMessage(114);}
  return std::make_pair(atom_list,uc);
}

///////////////////
// AUX FUNCTIONS //
///////////////////
// split line into substrings when separated by whitespaces
std::vector<std::string> ImportMngr::splitLine(const std::string& line){
  std::istringstream iss(line);
  std::vector<std::string> substrings((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
  return substrings;
}

// Reads a string and converts it to valid atom symbol: first character uppercase followed by lowercase characters
std::string ImportMngr::strToValidSymbol(std::string str){
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

