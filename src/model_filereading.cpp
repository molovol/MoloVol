#include "model.h"
#include "atom.h"
#include "controller.h"
#include "misc.h"
#include "exception.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream> // stringstream
#include <cassert>
#include <exception>
#include <iterator>
#include <algorithm>

///////////////////////////////
// AUX FUNCTION DECLARATIONS //
///////////////////////////////

std::string strToValidSymbol(std::string str);
static inline std::vector<std::string> splitLine(const std::string& line);

struct ElementsFileBundle{ // data bundle for elements file import
  std::unordered_map<std::string, double> rad_map;
  std::unordered_map<std::string, double> weight_map;
  std::unordered_map<std::string, int> atomic_num_map;
};

struct UnitCell{
  UnitCell() : space_group("") {};

  std::array<double,6> parameters;
  std::string space_group;
};

//////////////////////////
// ELEMENTS FILE IMPORT //
//////////////////////////
ElementsFileBundle extractDataFromElemFile(const std::string& elem_path);

// generates three maps for assigning a radius, weight and atomic number respectively, to an element symbol
// sets the maps to members of the model class
bool Model::importElemFile(const std::string& elem_path){
  ElementsFileBundle data = extractDataFromElemFile(elem_path);
  if (data.rad_map.size() == 0) {return false;}

  setRadiusMap(data.rad_map);
  _elem_weight = data.weight_map;
  _elem_Z = data.atomic_num_map;
  return true;
}

// used for importing only the radius map from the radius file
// needed for running the app from the command line
std::unordered_map<std::string, double> Model::extractRadiusMap(const std::string& elem_path){
  return extractDataFromElemFile(elem_path).rad_map;
}

ElementsFileBundle extractDataFromElemFile(const std::string& elem_path){
  ElementsFileBundle data;

  auto hasCorrectFormat = [](std::vector<std::string> substrings){
    if (substrings.size() != 4){return false;}
    if (substrings[0].find_first_not_of("0123456789") != std::string::npos){return false;}
    for (char i : {2,3}){
      if (substrings[i].find_first_not_of("0123456789E.+e") != std::string::npos){return false;}
    }
    return true;
  };

  std::string line;
  std::ifstream inp_file(elem_path);
  bool invalid_symbol_detected = false;
  bool invalid_radius_value = false;
  bool invalid_weight_value = false;
  while(getline(inp_file,line)){
    std::vector<std::string> substrings = splitLine(line);
    // substrings[0]: Atomic Number
    // substrings[1]: Element Symbol
    // substrings[2]: Radius
    // substrings[3]: Weight
    if(hasCorrectFormat(substrings)){
      substrings[1] = strToValidSymbol(substrings[1]);
      // skip entry if element symbol invalid
      if (substrings[1].empty()){
        invalid_symbol_detected = true;
      }
      else {
        try{data.rad_map[substrings[1]] = std::stod(substrings[2]);}
        catch (const std::invalid_argument& e){
          data.rad_map[substrings[1]] = 0;
          invalid_radius_value = true;
        }
        try{data.weight_map[substrings[1]] = std::stod(substrings[3]);}
        catch (const std::invalid_argument& e){
          data.weight_map[substrings[1]] = 0;
          invalid_weight_value = true;
        }
        try{data.atomic_num_map[substrings[1]] = std::stoi(substrings[0]);}
        catch (const std::invalid_argument& e){
          data.atomic_num_map[substrings[1]] = 0;
        }
      }
    }
  }
  if (invalid_symbol_detected) {Ctrl::getInstance()->displayErrorMessage(106);}
  if (invalid_radius_value) {Ctrl::getInstance()->displayErrorMessage(107);}
  if (invalid_weight_value) {Ctrl::getInstance()->displayErrorMessage(108);}
  return data;
}

//////////////////////
// ATOM FILE IMPORT //
//////////////////////

bool Model::readAtomsFromFile(const std::string& filepath, bool include_hetatm){
  // it is very hard to tell what this function does, because of the heavy use of global variables.
  // this function should ideally end in a set function, and the other functions should have return
  // values -JM
  clearAtomData();

  std::vector<Atom> atom_list;
  try{

    // XYZ file import
    if (fileExtension(filepath) == "xyz"){
      atom_list = readFileXYZ(filepath);
    }  
    // PDB file import
    else if (fileExtension(filepath) == "pdb"){
      const std::pair<std::vector<Atom>,UnitCell> import_data = readFilePDB(filepath, include_hetatm);
      atom_list = import_data.first;
      
      _space_group = import_data.second.space_group;
      for (size_t i = 0; i < _cell_param.size(); ++i){
        _cell_param[i] = import_data.second.parameters[i];
      }
    }
    // CIF file import
    else if (fileExtension(filepath) == "cif"){
      try{atom_list = readFileCIF(filepath);}
      catch (const ExceptInvalidCellParams& e){throw;}
    }
    // Invalid file 
    else {throw ExceptIllegalFileExtension();}
   
  }
  catch(const ExceptInvalidCellParams& e) {throw;}

  // Collect data
  for (const Atom& elem : atom_list){
    _atom_count[elem.symbol]++;
    _raw_atom_coordinates.push_back(
        std::make_tuple(elem.symbol, elem.pos_x, elem.pos_y, elem.pos_z));
  }

  if (_raw_atom_coordinates.size() == 0){ // If no atom is detected in the input file, the file is deemed invalid
    throw ExceptInvalidInputFile();
  }
  return true;
}

void Model::clearAtomData(){
  _atom_count.clear();
  _raw_atom_coordinates.clear();
  _space_group = "";
  _sym_matrix_XYZ.clear();
  _sym_matrix_fraction.clear();
  for(int i = 0; i < 6; i++){
    _cell_param[i] = 0;
  }
}

std::vector<Atom> Model::readFileXYZ(const std::string& filepath){
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

std::pair<std::vector<Atom>,UnitCell> Model::readFilePDB(const std::string& filepath, bool include_hetatm){
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

// this function will successfully extract data from standard cif files such as those generated by Mercury program
// however, due to the permissive format of cif files, this function might fail in some occasions
std::vector<Atom> Model::readFileCIF(const std::string& filepath){
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

      std::vector<std::string> substrings = splitLine(line);
      for (size_t i = 0; i < s_cell_data_keywords.size(); ++i){
        if (containsKeyword(line,s_cell_data_keywords[i])){
          try{_cell_param[i] = std::stod(substrings[1]);}
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
          symop_list.emplace_back(line);
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
            std::vector<std::string> values = splitLine(line);
            if (atom_headers.size() != values.size()){
              // TODO: exception, invalid atom line
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

  // the atom coordinates are needed in Cartesian coordinates in angstrom
  // thus, the a, b, c fractions need to be converted to Cartesian coordinates
  // find the Cartesian matrix defining the unit cell axes A, B, C and store in _cart_matrix
  // Ax [0][0], Ay [0][1], Az [0][2]
  // Bx [1][0], By [1][1], Bz [1][2]
  // Cx [2][0], Cy [2][1], Cz [2][2]
  _cart_matrix = orthogonalizeUnitCell();

  if(space_group_P1){ // set identity symmetry element
    for(int i = 0; i < 3; i++){
      for(int j = 0; j < 3; j++){
        if(i == j){
          _sym_matrix_XYZ.emplace_back(1);
        }
        else{
          _sym_matrix_XYZ.emplace_back(0);
        }
      }
      _sym_matrix_fraction.emplace_back(0);
    }
  }
  else{
    convertCifSymmetryElements(symop_list);
  }
  auto atom_list_result = convertCifAtomsList(atom_data, _cart_matrix);
  if (!atom_list_result.first){
    Ctrl::getInstance()->displayErrorMessage(105); // at least one atom line could not be read
  }
  return atom_list_result.second;
}

// used in unittest
std::vector<std::string> Model::listElementsInStructure(){
  std::vector<std::string> list;
  for (auto elem : _atom_count){
    list.push_back(elem.first);
  }
  return list;
}

// symmetry elements in cif files are stored as strings that are not convenient for calculations
// this function converts the list of strings in convenient matrix
bool Model::convertCifSymmetryElements(const std::vector<std::string> &symop_list){
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

  for(size_t i = 0; i < symop_list.size(); i++){
    // stringstream class check1
    std::stringstream check1(symop_list[i]);
    // Tokenizing separated by comma
    std::string token;
    for (int j = 0; getline(check1, token, ','); j++){
      for (char coord : {'x','y','z'}){
        _sym_matrix_XYZ.push_back(evalToken(token, coord));
      }

      // if there is a fraction in the substring, convert to double
      _sym_matrix_fraction.push_back(extractFrac(token));
    }
  }
  return true;
}

std::pair<bool,std::vector<Atom>> Model::convertCifAtomsList(
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
    const std::string symbol = strToValidSymbol(atom_data.at("_atom_site_type_symbol")[i]);

    atom_list.emplace_back(std::make_pair(symbol, xyz));

    // store the full list of atom coordinates from the input file
  }
  return std::make_pair(all_lines_valid,atom_list);
}

bool Model::getSymmetryElements(std::string group, std::vector<int> &sym_matrix_XYZ, std::vector<double> &sym_matrix_fraction){
  for (size_t i = 0; i<group.size(); i++) { // convert space group to upper case chars to compare with the list
    group[i] = toupper(group[i]);
  }
  group = "'" + group + "'";
  std::ifstream sym_file(getResourcesDir() + "/space_groups.txt");
  std::string sym_line;
  bool group_found = 0;
  bool sym_matrix = 0;
  while (getline (sym_file, sym_line)){
    if(sym_matrix){
      if(sym_line.find("Space group end") != std::string::npos){
        return true; // end function when all symmetry elements of the matching space group are stored in vectors
      }
      else{
        std::vector<std::string> sym_matrix_line = splitLine(sym_line); // store 12 parameters of the symmetry matrix
        std::stringstream ss;
        if(sym_matrix_line.size() == 12){
          for (int i = 0; i < 9; i++){
            std::stringstream ss(sym_matrix_line[i]);
            int matrix_elem = 0;
            ss >> matrix_elem;
            sym_matrix_XYZ.emplace_back(matrix_elem); // stores the Aa, Ab, Ac, Ba, Bb, Bc, Ca, Cb, Cc matrix elements as +1, 0 or -1
          }
          for (int i = 9; i < 12; i++){
            sym_matrix_fraction.emplace_back(std::stod(sym_matrix_line[i])); // stores the AA, BB, CC matrix elements as fractions 0, 1/6, 1/4, 1/3, 1/2, 2/3, 3/4, 5/6
          }
        }
        else {
          return false; // return false if a matrix line was found with less than 12 elements which should not happen with the space group list file provided
        }
      }
    }
    else if(group_found && sym_line.find("_symmetry_equiv_pos_as_matrix") != std::string::npos){
      sym_matrix = 1; // activate second switch when we reach the symmetry matrix part of the matching space group
    }
    else if(sym_line.find("_symmetry_space_group_name_H-M") != std::string::npos && sym_line.find(group) != std::string::npos){
      group_found = 1; // activate switch when matching space group is found
    }
  }
  return false; // return false when no matching space group was found in the list file or no file found
}

////////////////////////
// METHOD DEFINITIONS //
////////////////////////

// returns the radius of an atom with a given symbol
double Model::findRadiusOfAtom(const std::string& symbol){
  return _radius_map[symbol];
}

double Model::findRadiusOfAtom(const Atom& at){
  return findRadiusOfAtom(at.symbol);
}

double Model::findWeightOfAtom(const std::string& symbol){
  return _elem_weight[symbol];
}

///////////////////
// AUX FUNCTIONS //
///////////////////

// split line into substrings when separated by whitespaces
static inline std::vector<std::string> splitLine(const std::string& line){
  std::istringstream iss(line);
  std::vector<std::string> substrings((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
  return substrings;
}

// Reads a string and converts it to valid atom symbol: first character uppercase followed by lowercase characters
std::string strToValidSymbol(std::string str){
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

