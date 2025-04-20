#include "model.h"
#include "atom.h"
#include "controller.h"
#include "misc.h"
#include "exception.h"
#include "importmanager.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream> // stringstream
#include <cassert>
#include <exception>
#include <iterator>
#include <algorithm>

///////////////////
// IMPORT STRUCT //
///////////////////

struct ElementsFileBundle{ // data bundle for elements file import
  std::unordered_map<std::string, double> rad_map;
  std::unordered_map<std::string, double> weight_map;
  std::unordered_map<std::string, int> atomic_num_map;
};

//////////////////////////
// ELEMENTS FILE IMPORT //
//////////////////////////
// TODO: Move to importmanager.h
ElementsFileBundle extractDataFromElemFile(const std::string& elem_path);

// generates three maps for assigning a radius, weight and atomic number respectively, to an element symbol
// sets the maps to members of the model class
bool Model::importElemFile(const std::string& elem_path){
  ElementsFileBundle data = extractDataFromElemFile(elem_path);
  setRadiusMap(data.rad_map);
  _elem_weight = data.weight_map;
  _elem_Z = data.atomic_num_map;
  
  return (data.rad_map.size());
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
    std::vector<std::string> substrings = ImportMngr::splitLine(line);
    // substrings[0]: Atomic Number
    // substrings[1]: Element Symbol
    // substrings[2]: Radius
    // substrings[3]: Weight
    if(hasCorrectFormat(substrings)){
      substrings[1] = ImportMngr::strToValidSymbol(substrings[1]);
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
  clearAtomData();

  std::vector<Atom> atom_list;
  // XYZ file import
  if (fileExtension(filepath) == "xyz"){
    atom_list = ImportMngr::readFileXYZ(filepath);
  }  
  // PDB file import
  else if (fileExtension(filepath) == "pdb"){
    const std::pair<std::vector<Atom>,UnitCell> import_data = ImportMngr::readFilePDB(filepath, include_hetatm);
    atom_list = import_data.first;
    
    _space_group = import_data.second.space_group;
    for (size_t i = 0; i < _cell_param.size(); ++i){
      _cell_param[i] = import_data.second.parameters[i];
    }
  }
  // CIF file import
  else if (fileExtension(filepath) == "cif"){
    try{
      const std::pair<std::vector<Atom>,UnitCell> import_data = ImportMngr::readFileCIF(filepath);
      atom_list = import_data.first;
      _cell_param = import_data.second.parameters;
      _cart_matrix = import_data.second.cart_matrix;
      _sym_matrix_XYZ = import_data.second.sym_matrix_XYZ;
      _sym_matrix_fraction = import_data.second.sym_matrix_fraction;
    }
    catch (const ExceptInvalidCellParams& e){
      Ctrl::getInstance()->displayErrorMessage(112);
      return false;
    }
  }
  // File extension not supported 
  else {
    Ctrl::getInstance()->displayErrorMessage(103);
    return false;
  }

  for (const Atom& elem : atom_list){
    _raw_atom_coordinates.emplace_back(elem.symbol, elem.pos_x, elem.pos_y, elem.pos_z);
  }
  
  // If no atom is detected in the input file, the file is deemed invalid
  if (atom_list.empty()){
    Ctrl::getInstance()->displayErrorMessage(102);
    return false;
  }

  return true;
}

void Model::clearAtomData(){
  _raw_atom_coordinates.clear();
  _space_group = "";
  _sym_matrix_XYZ.clear();
  _sym_matrix_fraction.clear();
  for(int i = 0; i < 6; i++){
    _cell_param[i] = 0;
  }
}

// used in unittest
std::vector<std::string> Model::listElementsInStructure(){
  std::vector<std::string> list;

  auto atom_count = atomCount(_raw_atom_coordinates);
  for (auto elem : atom_count){
    list.push_back(elem.first);
  }
  return list;
}

// TODO: This function seems misplaced
bool Model::getSymmetryElements(std::string group, std::vector<int> &sym_matrix_XYZ, std::vector<double> &sym_matrix_fraction){
  for (size_t i = 0; i<group.size(); i++) { // convert space group to upper case chars to compare with the list
    group[i] = toupper(group[i]);
  }
  group = "'" + group + "'";

#if defined(_WIN32)
  std::string sep = "\\";
#else
  std::string sep = "/";
#endif

  std::ifstream sym_file(getResourcesDir() + sep + "space_groups.txt");
  std::string sym_line;
  bool group_found = 0;
  bool sym_matrix = 0;
  while (getline (sym_file, sym_line)){
    if(sym_matrix){
      if(sym_line.find("Space group end") != std::string::npos){
        return true; // end function when all symmetry elements of the matching space group are stored in vectors
      }
      else{
        std::vector<std::string> sym_matrix_line = ImportMngr::splitLine(sym_line); // store 12 parameters of the symmetry matrix
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

// Returns the radius of an atom with a given symbol
// If the symbol is not intially found, the function strips everything after the first
// non-letter character and tries to look up the radius again. If the symbol is still not
// found, defaults to 0
double Model::findRadiusOfAtom(std::string symbol) const {
  if (_radius_map.count(symbol)){
    return _radius_map.at(symbol);
  }

  symbol = ImportMngr::stripCharge(symbol);

  if (_radius_map.count(symbol)){
    return _radius_map.at(symbol);
  }
  
  return 0;
}

double Model::findRadiusOfAtom(const Atom& at) const {
  return findRadiusOfAtom(at.symbol);
}

