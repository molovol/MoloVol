#include "model.h"
#include "atom.h"
#include "controller.h"
#include "misc.h"
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

bool isAtomLine(const std::vector<std::string>& substrings);
std::string strToValidSymbol(std::string str);
static inline std::vector<std::string> splitLine(std::string& line);

/////////////////
// FILE IMPORT //
/////////////////

// generates two two maps for assigning a radius/ atomic number respectively, to a element symbol
// sets the maps to members of the model class
bool Model::readRadiiAndAtomNumFromFile(std::string& radius_path){
  std::unordered_map<std::string, double> rad_map;
  std::unordered_map<std::string, int> atomic_num_map;

  std::string line;
  std::ifstream inp_file(radius_path);
  while(getline(inp_file,line)){
    std::vector<std::string> substrings = splitLine(line);
    // substings[0]: Atomic Number
    // substings[1]: Element Symbol
    // substings[2]: Radius
    if(substrings.size() == 3){
      // TODO: make sure substrings[1] is converted to valid symbol
      rad_map[substrings[1]] = std::stod(substrings[2]);
      atomic_num_map[substrings[1]] = std::stoi(substrings[0]);
    }
  }

  if (rad_map.size() == 0) {return false;}
  raw_radius_map = rad_map;
  elem_Z = atomic_num_map;
  return true;
}

bool Model::readAtomsFromFile(const std::string& filepath, bool include_hetatm){
  // clear previous data
  atom_amounts.clear();
  raw_atom_coordinates.clear();
  space_group = "";
  for(int i = 0; i < 6; i++){
    cell_param[i] = 0;
  }

  if (fileExtension(filepath) == "xyz"){
    readFileXYZ(filepath);
  }
  else if (fileExtension(filepath) == "pdb"){
    readFilePDB(filepath, include_hetatm);
  }
  else { // The browser does not allow other file formats but a user could manually write the path to an invalid file
    Ctrl::getInstance()->notifyUser("Invalid structure file format!");
    return false;
  }
  if (raw_atom_coordinates.size() == 0){ // If no atom is detected in the input file, the file is deemed invalid
    Ctrl::getInstance()->notifyUser("Invalid structure file!");
    return false;
  }
  return true;
}

void Model::readFileXYZ(const std::string& filepath){

//if (inp_file.is_open()){  //TODO consider adding an exception, for when file in not valid
  std::string line;
  std::ifstream inp_file(filepath);

  // iterate through lines
  while(getline(inp_file,line)){
    // divide line into "words"
    std::vector<std::string> substrings = splitLine(line);
    // create new atom and add to storage vector if line format corresponds to Element_Symbol x y z
    if (isAtomLine(substrings)) {

      std::string valid_symbol = strToValidSymbol(substrings[0]);
      atom_amounts[valid_symbol]++; // adds one to counter for this symbol

      // if a key leads to multiple z-values, set z-value to 0 (?)
      if (elem_Z.count(valid_symbol) > 0){
        elem_Z[valid_symbol] = 0;
      }
      // Stores the full list of atom coordinates from the input file
      raw_atom_coordinates.emplace_back(valid_symbol,
                                        std::stod(substrings[1]),
                                        std::stod(substrings[2]),
                                        std::stod(substrings[3]));
    }
  }
  // file has been read
  inp_file.close();
}

void Model::readFilePDB(const std::string& filepath, bool include_hetatm){

//if (inp_file.is_open()){  //TODO consider adding an exception, for when file in not valid
  std::string line;
  std::ifstream inp_file(filepath);

  // iterate through lines
  while(getline(inp_file,line)){
    if (line.substr(0,6) == "ATOM  " || (include_hetatm == true && line.substr(0,6) == "HETATM")){
      // Element symbol is located at characters 77 and 78, right-justified in the official pdb format
      std::string symbol = line.substr(76,2);
      // Some software generate pdb files with symbol left-justified instead of right-justified
      // Therefore, it is better to check both characters and erase any white space
      symbol.erase(std::remove(symbol.begin(), symbol.end(), ' '), symbol.end());
      symbol = strToValidSymbol(symbol);
      atom_amounts[symbol]++; // adds one to counter for this symbol

      // if a key leads to multiple z-values, set z-value to 0 (?)
      if (elem_Z.count(symbol) > 0){
        elem_Z[symbol] = 0;
      }
      // Stores the full list of atom coordinates from the input file
      raw_atom_coordinates.emplace_back(symbol,
                                        std::stod(line.substr(30,8)),
                                        std::stod(line.substr(38,8)),
                                        std::stod(line.substr(46,8)));
    }
    else if (line.substr(0,6) == "CRYST1"){
      cell_param[0] = std::stod(line.substr(6,9));
      cell_param[1] = std::stod(line.substr(15,9));
      cell_param[2] = std::stod(line.substr(24,9));
      cell_param[3] = std::stod(line.substr(33,7));
      cell_param[4] = std::stod(line.substr(40,7));
      cell_param[5] = std::stod(line.substr(47,7));
      space_group = line.substr(55,11); // note: mercury recognizes only 10 chars but official PDB format is 11 chars
      space_group.erase(std::remove(space_group.begin(), space_group.end(), ' '), space_group.end()); // remove white spaces
    }
  }
  // file has been read
  inp_file.close();
}

bool Model::getSymmetryElements(std::string group, std::vector<int> &sym_matrix_XYZ, std::vector<double> &sym_matrix_fraction){
  for (int i = 0; i<group.size(); i++) { // convert space group to upper case chars to compare with the list
    group[i] = toupper(group[i]);
  }
  group = "'" + group + "'";
  std::ifstream sym_file("./inputfile/space_groups.txt");
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
inline double Model::findRadiusOfAtom(const std::string& symbol){
  //TODO add exception handling for when no radius was found:
  //if(radius_map[symbol == 0]){
  //  throw ...;
  //}
  //else{ return...;}
  return radius_map[symbol];
}

inline double Model::findRadiusOfAtom(const Atom& at){
  return findRadiusOfAtom(at.symbol);
}

///////////////////
// AUX FUNCTIONS //
///////////////////

// split line into substrings when separated by whitespaces
static inline std::vector<std::string> splitLine(std::string& line){
  std::istringstream iss(line);
  std::vector<std::string> substrings((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
  return substrings;
}

bool isAtomLine(const std::vector<std::string>& substrings) {
  if (substrings.size() == 4) {
// TODO: decide whether element symbol substring should be checked as starting with a letter
//  if (isalpha(substrings[0][0])){
    for (char i=1; i<4; i++){
      // using a try-block feels hacky, but the alternative using strtod does as well
      try {
        size_t str_pos = 0; // will contain the last position in the string that was successfully evaluated by std::stod()
        std::stod(substrings[i], &str_pos);
        if (substrings[i].size() != str_pos) {
          return false;
        }
      }
      catch (const std::invalid_argument& ia) {
        return false;
      }
    }
/* TODO bis
  }
  else {
  	return false;
  }*/
    return true;
  }
  return false;
}

// reads a string and converts it to valid atom symbol: first character uppercase followed by lowercase characters
std::string strToValidSymbol(std::string str){
  // iterate over all characters in string
  for (int i = 0; i<str.size(); i++) {
// TODO: decide whether non-alphabetic characters should be erased or not
//    if (isalpha(str[i])){
      // only for first character in sequence, convert to uppercase
      if (i==0) {
        str[i] = toupper(str[i]);
      }
      // for all other characters, convert to lowercase
      else {
        str[i] = tolower(str[i]);
      }
// TODO bis
//    }
//    else { // Remove number or charges from atoms so that "Pd2+" becomes "Pd"
//      str.erase(i, str.size());
//    }
  }
  return str;
}
