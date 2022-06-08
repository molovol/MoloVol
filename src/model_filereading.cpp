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

struct ElementsFileBundle{ // data bundle for elements file import
  std::unordered_map<std::string, double> rad_map;
  std::unordered_map<std::string, double> weight_map;
  std::unordered_map<std::string, int> atomic_num_map;
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
    std::vector<std::string> substrings = FileMngr::splitLine(line);
    // substrings[0]: Atomic Number
    // substrings[1]: Element Symbol
    // substrings[2]: Radius
    // substrings[3]: Weight
    if(hasCorrectFormat(substrings)){
      substrings[1] = FileMngr::strToValidSymbol(substrings[1]);
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
  
  // XYZ file import
  if (fileExtension(filepath) == "xyz"){
    atom_list = FileMngr::readFileXYZ(filepath);
  }  
  // PDB file import
  else if (fileExtension(filepath) == "pdb"){
    const std::pair<std::vector<Atom>,UnitCell> import_data = FileMngr::readFilePDB(filepath, include_hetatm);
    atom_list = import_data.first;
    
    _space_group = import_data.second.space_group;
    for (size_t i = 0; i < _cell_param.size(); ++i){
      _cell_param[i] = import_data.second.parameters[i];
    }
  }
  // CIF file import
  else if (fileExtension(filepath) == "cif"){
    try{
      const std::pair<std::vector<Atom>,UnitCell> import_data = readFileCIF(filepath);
      atom_list = import_data.first;
      _cell_param = import_data.second.parameters;
      _cart_matrix = import_data.second.cart_matrix;
      _sym_matrix_XYZ = import_data.second.sym_matrix_XYZ;
      _sym_matrix_fraction = import_data.second.sym_matrix_fraction;
    }
    catch (const ExceptInvalidCellParams& e){throw;}
  }
  // Invalid file 
  else {throw ExceptIllegalFileExtension();}
  
  // Assemble atom data
  for (const Atom& elem : atom_list){
    _atom_count[elem.symbol]++;
    _raw_atom_coordinates.push_back(
        std::make_tuple(elem.symbol, elem.pos_x, elem.pos_y, elem.pos_z));
  }

  if (_raw_atom_coordinates.empty()){ // If no atom is detected in the input file, the file is deemed invalid
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


// this function will successfully extract data from standard cif files such as those generated by Mercury program
// however, due to the permissive format of cif files, this function might fail in some occasions
std::pair<std::vector<Atom>,typename Model::UnitCell> Model::readFileCIF(const std::string& filepath){
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

      std::vector<std::string> substrings = FileMngr::splitLine(line);
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
            std::vector<std::string> values = FileMngr::splitLine(line);
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
  uc.cart_matrix = orthogonalizeUnitCell(uc.parameters);

  auto atom_list_result = convertCifAtomsList(atom_data, uc.cart_matrix);
  if (!atom_list_result.first){
    Ctrl::getInstance()->displayErrorMessage(105); // at least one atom line could not be read
  }
  return std::make_pair(atom_list_result.second, uc);
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
typename Model::SymMatData Model::convertCifSymmetryElements(const std::vector<std::string> &symop_list){
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
    const std::string symbol = FileMngr::strToValidSymbol(atom_data.at("_atom_site_type_symbol")[i]);

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
        std::vector<std::string> sym_matrix_line = FileMngr::splitLine(sym_line); // store 12 parameters of the symmetry matrix
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


