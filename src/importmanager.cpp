#include "importmanager.h"
#include "crystallographer.h"
#include "misc.h"

// This is a temporary fix so that we can wite unit tests for sections of the code
#ifndef LIBRARY_BUILD
#include "controller.h"
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

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
  if (invalid_entry_encountered){
    #ifndef LIBRARY_BUILD
    Ctrl::getInstance()->displayErrorMessage(105);
    #endif
    }
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
    // has the correct length and validate the content later
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

      // Evaluate atom symbol
      std::string symbol = strToValidSymbol(fields.at("element"), charge);
      if (symbol.empty()){
        invalid_symbol_detected = true;
        continue;
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
  if (invalid_symbol_detected){
    #ifndef LIBRARY_BUILD
    Ctrl::getInstance()->displayErrorMessage(105);
    #endif
  }
  if (invalid_cell_params){
    #ifndef LIBRARY_BUILD
    Ctrl::getInstance()->displayErrorMessage(112);
    #endif
  }
  if (invalid_atom_line){
    #ifndef LIBRARY_BUILD
    Ctrl::getInstance()->displayErrorMessage(114);
    #endif
  }
  return std::make_pair(atom_list,uc);
}

////////////////
// CIF IMPORT //
////////////////
// this function will successfully extract data from standard cif files such as those generated by Mercury program
// however, due to the permissive format of cif files, this function might fail in some occasions
std::pair<std::vector<Atom>,ImportMngr::UnitCell> ImportMngr::readFileCIF(const std::string& filepath){
  std::string line; // line in cif file
  std::string no_ws_line; // line in cif file with whitespaces removed
  std::ifstream inp_file(filepath);

  bool space_group_P1 = false;
  bool symmetry_data_acquired = false;
  bool atom_data_acquired = false;
  char cell_data_acquired = 0;
  char loop = 0; // 0: out of list, 1: in list, 2: symop headers list, 3: symop value list, 4: atom headers list, 5: atom data list
  std::vector<std::string> symop_list;
  std::vector<std::string> atom_headers;
  std::map<std::string,std::vector<std::string>> atom_data;

  UnitCell uc = UnitCell();

  auto containsKeyword = [](const std::string& line, const std::string& keyword=""){
    // default: contains any keyword
    if (keyword.empty()){return (line[0] == '_');}
    // contains specific keyword
    else {return (line.find(keyword) != std::string::npos);}
  };

  while(getline(inp_file,line)){ // read file line by line
    StrMngr::removeEOL(line);
    // the information we need is case insensitive
    // therefore, it is safer to directly convert the lines to lower case
    std::for_each(line.begin(), line.end(), [](char& c){c = ::tolower(c);});

    // in cif files white_space are sometimes meaningful, sometimes not
    // thus, it is convenient to have a copy of a cif line with no white_space
    no_ws_line = line;
    StrMngr::removeWhiteSpaces(no_ws_line);

    if(no_ws_line[0] == '#' || no_ws_line.empty()){continue;} // skip comment and empty lines

    // KEYWORD SEARCH
    // cif files usually contain explicit symmetry operations
    // however, space group P1 does not have any symmetry operation except the identity
    // therefore, the symmetry operation list may not be present for this group
    // and it is useful to keep track of the space group if it is P1
    if(containsKeyword(line,"_space_group_name_") && containsKeyword(no_ws_line,"'p1'")){
        space_group_P1 = true;
    }
    // list of symmetry operations or atom coordinates will start with "loop_"
    else if(!loop && containsKeyword(line,"loop_")){
      loop = 1; // we have entered a loop
      continue;
    }
    else {
      static const std::array<std::string,6> s_cell_data_keywords = {
        "_cell_length_a", "_cell_length_b", "_cell_length_c",
        "_cell_angle_alpha", "_cell_angle_beta", "_cell_angle_gamma"};

      std::vector<std::string> substrings = ImportMngr::splitLine(line);
      for (size_t i = 0; i < s_cell_data_keywords.size(); ++i){
        if (containsKeyword(line,s_cell_data_keywords[i])){
          try{uc.parameters[i] = std::stod(substrings[1]);}
          catch (const std::invalid_argument& e){throw ExceptInvalidCellParams();}
          cell_data_acquired++;
          break;
        }
      }
    }

    // LOOP
    // Using if instead of switch because the individual if statements are evaluated sequencially
    if (loop){
      // identify whether this loop contains interesting data
      if(loop == 1){
        if(!symmetry_data_acquired
            && (containsKeyword(line,"_space_group_symop_") || containsKeyword(line,"_symmetry_equiv_pos_"))){
          // symmetry data loop
          loop = 2;
        }
        else if(!atom_data_acquired && containsKeyword(line,"_atom_site_")){
          // atom data loop
          loop = 4;
        }
        else if(!containsKeyword(no_ws_line)){ // we have left the loop without finding an interesting keyword
          loop = 0;
        }
      }

      if(loop == 2){
        // in list of headers (data name) for symmetry operations
        // there are either 1 or 2 headers in these symop loops
        // it is unnecessary to store these headers as they are irrelevant to the symop data analysis algorithm

        // if no data name is found, the list of symmetry operations started
        if(!containsKeyword(no_ws_line)){
          loop = 3;
        }
      }

      if(loop == 3){
        // in list of symmetry operations
        // if a data name is found, the list of symmetry operation value ended
        if(containsKeyword(no_ws_line)){
          loop = 0;
          symmetry_data_acquired = true;
        }
        else if(containsKeyword(line,"loop_")){
          loop = 1;
          symmetry_data_acquired = true;
        }
        else{
          // store the line containing symmetry operation data for later processing
          symop_list.push_back(line);
          continue;
        }
      }

      if(loop == 4){
        // in list of headers (data name) for atoms
        // if no data name is found, the list of atom parameters started
        if(!containsKeyword(no_ws_line)){
          loop = 5;
        }
        else{
          // store atom headers in order of appearance because only certain types of atom data is useful
          // and it is necessary to know in what order they appear
          atom_headers.push_back(no_ws_line);
          atom_data[no_ws_line] = {};
          continue;
        }
      }

      if(loop == 5){
        // in list of atom parameters
        // if a data name is found, the list of atom parameters ended
        if(containsKeyword(no_ws_line)){
          loop = 0;
          atom_data_acquired = true;
        }
        else if(containsKeyword(line,"loop_")){
          loop = 1;
          atom_data_acquired = true;
        }
        else{
          { // Save atom line data as map
            std::vector<std::string> values = ImportMngr::splitLine(line);
            if (atom_headers.size() != values.size()){
              // TODO: exception, invalid atom line
              continue;
            }
            for (size_t i = 0; i < values.size(); ++i){
              atom_data.at(atom_headers[i]).push_back(values[i]);
            }
          }
          continue;
        }
      }
    }

    // when all useful data is extracted, stop reading the cif file
    if(atom_data_acquired && symmetry_data_acquired && cell_data_acquired > 5){
      break;
    }
  }
  inp_file.close();

  if(space_group_P1){ // set identity symmetry element
    for(int i = 0; i < 3; i++){
      for(int j = 0; j < 3; j++){
        uc.sym_matrix_XYZ.push_back(i==j? 1 : 0);
      }
      uc.sym_matrix_fraction.push_back(0);
    }
  }
  else{
    auto sm = convertCifSymmetryElements(symop_list);
    uc.sym_matrix_XYZ = sm.first;
    uc.sym_matrix_fraction = sm.second;
  }

  // the atom coordinates are needed in Cartesian coordinates in angstrom
  // thus, the a, b, c fractions need to be converted to Cartesian coordinates
  // find the Cartesian matrix defining the unit cell axes A, B, C and store in cart_matrix
  // Ax [0][0], Ay [0][1], Az [0][2]
  // Bx [1][0], By [1][1], Bz [1][2]
  // Cx [2][0], Cy [2][1], Cz [2][2]
  uc.cart_matrix = Cryst::orthogonalizeUnitCell(uc.parameters);

  auto atom_list_result = convertCifAtomsList(atom_data, uc.cart_matrix);
  if (!atom_list_result.first){
    #ifndef LIBRARY_BUILD
    Ctrl::getInstance()->displayErrorMessage(105); // at least one atom line could not be read
    #endif
  }
  return std::make_pair(atom_list_result.second, uc);
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
std::string ImportMngr::strToValidSymbol(std::string str, signed charge){
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
  // Add charge at the end
  if (charge){
    str += std::to_string(abs(charge)) + (charge > 0? "+" : "-");
  }

  return str;
}

std::string ImportMngr::stripCharge(const std::string& symbol){
  auto it = find_if_not(symbol.begin(), symbol.end(), isalpha);
  return std::string(symbol.begin(), it);
}

// symmetry elements in cif files are stored as strings that are not convenient for calculations
// this function converts the list of strings in convenient matrix
typename ImportMngr::SymMatData ImportMngr::convertCifSymmetryElements(const std::vector<std::string> &symop_list){
  auto evalToken = [](const std::string& token, const char coord){
    if (token.find({'-',coord}) != std::string::npos){
      return -1;
    }
    return (token.find(coord) != std::string::npos)? 1 : 0;
  };

  auto extractFrac = [](const std::string& line){
    size_t divide_pos = line.find('/');
    // prevent accessing positions in string that are out of range
    if(divide_pos != std::string::npos && divide_pos > 0 && divide_pos < line.size()-1){
      char char_num = line[divide_pos-1];
      char char_denom = line[divide_pos+1];
      if (isdigit(char_num) && isdigit(char_denom)){
        double num = char_num - '0';
        double denom = char_denom - '0';
        int sign = (divide_pos > 1 && line[divide_pos-2] == '-')? 1 : -1;
        return sign * num/denom;
      }
    }
    return double(0);
  };

  SymMatData sym_matrix_data;
  for(size_t i = 0; i < symop_list.size(); i++){
    // stringstream class check1
    std::stringstream check1(symop_list[i]);
    // Tokenizing separated by comma
    std::string token;
    for (int j = 0; getline(check1, token, ','); j++){
      for (char coord : {'x','y','z'}){
        sym_matrix_data.first.push_back(evalToken(token, coord));
      }

      // if there is a fraction in the substring, convert to double
      sym_matrix_data.second.push_back(extractFrac(token));
    }
  }
  return sym_matrix_data;
}

std::pair<bool,std::vector<Atom>> ImportMngr::convertCifAtomsList(
    const std::map<std::string,std::vector<std::string>>& atom_data, const MatR3& cart_matrix){
  // The following keywords contain the information that we're interested in
  static const std::vector<std::string> s_keywords = {
    "_atom_site_type_symbol", "_atom_site_fract_x", "_atom_site_fract_y", "_atom_site_fract_z"};
  
  bool all_lines_valid = true;
  std::vector<Atom> atom_list;

  // Make sure all atom information is available
  for (auto& kw : s_keywords){
    all_lines_valid &= atom_data.count(kw);
  }
  if (!all_lines_valid){return std::make_pair(all_lines_valid,atom_list);}
  
  // Go through all atom entries
  for (size_t i = 0; i < atom_data.begin()->second.size(); ++i){
    
    // save the fractional positional data from atom_list
    std::array<double,3> abc_frac;
    try {
      for (size_t j = 0; j < 3; ++j){
        abc_frac[j] = std::stod(atom_data.at(s_keywords[1+j])[i]);
      }
    }
    catch (std::invalid_argument& e){
      all_lines_valid = false;
      continue; // Skip atom line
    }

    // apply the matrix to the fractional coordinates to get cartesian coordinates
    std::array<double,3> xyz = {0,0,0};
    for(int i = 0; i < 3; ++i){ // loop for x, y ,z
      for(int j = 0; j < 3; ++j){ // loop for a, b ,c
        xyz[i] += abc_frac[j] * cart_matrix[j][i];
      }
    }

    // get the element symbol and keep track of their number
    const std::string symbol = ImportMngr::strToValidSymbol(atom_data.at("_atom_site_type_symbol")[i]);

    atom_list.emplace_back(std::make_pair(symbol, xyz));

    // store the full list of atom coordinates from the input file
  }
  return std::make_pair(all_lines_valid,atom_list);
}

