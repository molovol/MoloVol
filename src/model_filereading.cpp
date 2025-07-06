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
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
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
  return data.rad_map.size();
}

// used for importing only the radius map from the radius file
// needed for running the app from the command line
std::unordered_map<std::string, double> Model::extractRadiusMap(const std::string& elem_path){
  return extractDataFromElemFile(elem_path).rad_map;
}

ElementsFileBundle extractDataFromElemFile(const std::string& elem_path) {
    ElementsFileBundle data;

    #ifdef __EMSCRIPTEN__
    char* content_ptr = (char*)EM_ASM_PTR({
        try {
            var content = FS.readFile('/inputfile/elements.txt', {encoding: 'utf8'});
            var lengthBytes = lengthBytesUTF8(content) + 1;
            var stringOnWasmHeap = _malloc(lengthBytes);
            stringToUTF8(content, stringOnWasmHeap, lengthBytes);
            return stringOnWasmHeap;
        } catch(e) {
            return 0;
        }
    });

    if (!content_ptr) {
        return data;
    }

    std::string fileContent(content_ptr);
    free(content_ptr);
    
    std::istringstream inp_stream(fileContent);
    #else
    std::ifstream inp_stream(elem_path);
    #endif

    std::string line;
    bool headerDone = false;

    while (getline(inp_stream, line)) {
        if(line.empty()) {
            continue;
        }

        if(!headerDone) {
            if(line[0] >= '0' && line[0] <= '9') {
                headerDone = true;
            } else {
                continue;
            }
        }

        std::vector<std::string> substrings = ImportMngr::splitLine(line);
        
        if(substrings.size() != 4) {
            continue;
        }

        try {
            int atomic_num = std::stoi(substrings[0]);
            std::string symbol = ImportMngr::strToValidSymbol(substrings[1]);
            double radius = std::stod(substrings[2]);
            double weight = std::stod(substrings[3]);

            if(!symbol.empty()) {
                data.rad_map[symbol] = radius;
                data.weight_map[symbol] = weight;
                data.atomic_num_map[symbol] = atomic_num;
            }
        }
        catch(const std::exception&) {
            continue;
        }
    }

    return data;
}
//////////////////////
// ATOM FILE IMPORT //
//////////////////////

bool Model::readAtomsFromFile(const std::string& filepath, bool include_hetatm){
  clearAtomData();
  
  // Debug output for filename and extension
  std::string extension = fileExtension(filepath);
  std::vector<Atom> atom_list;
  
  // XYZ file import
  if (extension == "xyz"){
    atom_list = ImportMngr::readFileXYZ(filepath);
  }  
  // PDB file import
  else if (extension == "pdb"){
    const std::pair<std::vector<Atom>,UnitCell> import_data = ImportMngr::readFilePDB(filepath, include_hetatm);
    atom_list = import_data.first;
    
    _space_group = import_data.second.space_group;
    for (size_t i = 0; i < _cell_param.size(); ++i){
      _cell_param[i] = import_data.second.parameters[i];
    }
  }
  // CIF file import
  else if (extension == "cif"){
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

