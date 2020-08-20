#include "model.h"
#include "atom.h"
#include "controller.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream> // stringstream
#include <cassert>
#include <exception>
#include <iterator>
#include <algorithm>

namespace fs = std::filesystem;

///////////////////////////////
// AUX FUNCTION DECLARATIONS //
///////////////////////////////

bool isAtomLine(const std::vector<std::string>& substrings);
std::string strToValidSymbol(std::string str);
static inline std::vector<std::string> splitLine(std::string& line);
std::string fileExtension(const std::string& path);

/////////////////
// FILE IMPORT //
/////////////////

void Model::importFiles(std::string& atom_filepath, std::string& radius_filepath, bool incl_hetatm){

  // radius file must be imported before atom file, because atom file import requires the radius map
  readRadiiAndAtomNumFromFile(radius_filepath);
  readAtomsFromFile(atom_filepath, incl_hetatm);

  // save filepaths and last write times
  filepaths_last_imported[0] = fs::path(atom_filepath);
  filepaths_last_imported[1] = fs::path(radius_filepath);
  for (char i = 0; i < 2; i++){
    files_last_written[i] = fs::last_write_time(filepaths_last_imported[i]);
  }
}

bool Model::importFilesChanged(std::string& current_atom_filepath, std::string& current_radius_filepath){

  std::array<std::string,2> current = {current_atom_filepath, current_radius_filepath};

  if (!filesExist(current)) {return true;} // this exception is handled in the load routine

  for (int i = 0; i < 2; i++){

    fs::path current_filepath = fs::path(current[i]);
    fs::file_time_type current_file_last_written = fs::last_write_time(current_filepath);

    // files have changed if the file path has changed, or when the file has been rewritten since last import
    if (filepaths_last_imported[i] != current_filepath || files_last_written[i] != current_file_last_written){
      return true;
    }
  }
  return false;
}

// reads radii from a file specified by the filepath and
// stores them in an unordered map that is an attribute of
// the Model object
void Model::readRadiiAndAtomNumFromFile(std::string& filepath){
  // clear unordered_maps to avoid keeping data from previous runs
  radius_map.clear();
  elem_Z.clear();

  std::string line;
  std::ifstream inp_file(filepath);

  while(getline(inp_file,line)){
    std::vector<std::string> substrings = splitLine(line);
    if(substrings.size() == 3){
      // TODO: make sure substrings[1] is converted to valid symbol
      radius_map[substrings[1]] = std::stod(substrings[2]);
      elem_Z[substrings[1]] = std::stoi(substrings[0]);
    }
  }
  return;
}

void Model::readAtomsFromFile(std::string& filepath, bool include_hetatm){

  std::vector<Atom> list_of_atoms;
  atom_amounts.clear();
  
  if (fileExtension(filepath) == "xyz"){
    readFileXYZ(list_of_atoms, filepath);
  }
  else if (fileExtension(filepath) == "pdb"){
    readFilePDB(list_of_atoms, filepath, include_hetatm);
  }
  else { // The browser does not allow other file formats but a user could manually write the path to an invalid file
    Ctrl::getInstance()->notifyUser("Invalid structure file format!");
  }

  atoms = list_of_atoms;
  storeAtomsInTree();
}

void Model::readFileXYZ(std::vector<Atom>& list_of_atoms, std::string& filepath){

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

      Atom at = Atom(std::stod(substrings[1]),
                     std::stod(substrings[2]),
                     std::stod(substrings[3]),
                     valid_symbol,
                     findRadiusOfAtom(valid_symbol),
                     elem_Z[valid_symbol]);
      list_of_atoms.push_back(at);
    }
  }
  // file has been read
  inp_file.close();
}

void Model::readFilePDB(std::vector<Atom>& list_of_atoms, std::string& filepath, bool include_hetatm){

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

      Atom at = Atom(std::stod(line.substr(30,8)),
                     std::stod(line.substr(38,8)),
                     std::stod(line.substr(46,8)),
                     symbol,
                     findRadiusOfAtom(symbol),
                     elem_Z[symbol]);
      list_of_atoms.push_back(at);
    }
  }
  // file has been read
  inp_file.close();
}

bool Model::filesExist(const std::array<std::string,2>& paths) const {
  return (std::filesystem::exists(paths[0]) && std::filesystem::exists(paths[1]));
}

bool Model::filesExist(const std::string& path1, const std::string& path2) const {
  std::array<std::string,2> paths = {path1, path2};
  return filesExist(paths);
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

std::string fileExtension(const std::string& path){
  // will cause an issue, if there is a dot in the middle of the file AND no file extension
  std::string after_dot = "";
  int dot_pos = path.find_last_of(".");
  if (dot_pos != std::string::npos){  
    return path.substr(dot_pos+1);
  }
  else{
    return "invalid";
  }
}

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
