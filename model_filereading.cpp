#include "model.h"
#include "atom.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream> // stringstream
#include <cassert>
#include <exception>
#include <iterator>

// split line into substrings when separated by whitespaces
static inline std::vector<std::string> splitLine(std::string& line){
  std::istringstream iss(line);
  std::vector<std::string> substrings((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
  return substrings;
}

// Reads a string and converts it to valid atom symbol with first char upper case then lower case chars
std::string strToValidSymbol(const std::string& str_inp){
  std::string validSymbol = "";
  for (int i = 0; str_inp[i]!=0; i++) {
    if (isalpha(str_inp[i])) {
      validSymbol = validSymbol + str_inp[i];
      if (i==0 && islower(validSymbol[i])) {
        validSymbol[i] = toupper(validSymbol[i]);
      }
      else if (i>0 && isupper(validSymbol[i])) {
        validSymbol[i] = tolower(validSymbol[i]);
      }
    }
    else {
      return validSymbol;
    }
  }
}

// reads radii from a file specified by the filepath and
// stores them in an unordered map that is an attribute of
// the Model object
void Model::readRadiiAndAtomNumFromFile(std::string& filepath){
  // clear unordered_maps to avoid keeping data from previous runs
  radii.clear();
  elem_Z.clear();
  //TODO add case handling for when file has already been read
  //TODO if map empty: ask user via dialog box if they want to
  //TODO reimport the file
  //TODO make a function "consultUser(string)"
  //if(radii.empty()){std::cout << "empty" << std::endl;}

  std::string line;
  std::ifstream inp_file(filepath);

  while(getline(inp_file,line)){
    std::vector<std::string> substrings = splitLine(line);
    if(substrings.size() == 3){
      radii[substrings[1]] = std::stod(substrings[2]);
      elem_Z[substrings[1]] = std::stoi(substrings[0]);
    }
  }
  return;
}

// returns the radius of an atom with a given symbol
inline double Model::findRadiusOfAtom(const std::string& symbol){
  //TODO add exception handling for when no radius was found:
  //if(radii[symbol == 0]){
  //  throw ...;
  //}
  //else{ return...;}
  return radii[symbol];
}

inline double Model::findRadiusOfAtom(const Atom& at){
  return findRadiusOfAtom(at.symbol);
}

void Model::readAtomsFromFile(std::string& filepath){

  int n_atom = 0;
  std::vector<Atom> list_of_atoms;

//if (inp_file.is_open()){  //TODO consider adding an exception, for when file in not valid

  // we iterate through the lines in the input file
  std::string line;
  std::ifstream inp_file(filepath);

  int line_counter = 0; // Might not be useful anymore
  // iterate through lines
  while(getline(inp_file,line)){ // ws to pass empty lines and blank space before atoms if any
    // divide line into "words"
    std::vector<std::string> substrings = splitLine(line);
    // create new atom and add to storage vector if line format corresponds to Element_Symbol x y z
    if (substrings.size() == 4
        && isalpha(substrings[0][0])
        && (isdigit(substrings[1][0]) || ispunct(substrings[1][0])) // ispunct is necessary in case of negative value starting with '-'
        && (isdigit(substrings[2][0]) || ispunct(substrings[2][0]))
        && (isdigit(substrings[3][0]) || ispunct(substrings[3][0]))) {
      Atom at = Atom(std::stod(substrings[1]),
                     std::stod(substrings[2]),
                     std::stod(substrings[3]),
                     strToValidSymbol(substrings[0]),
                     radii[strToValidSymbol(substrings[0])],
                     elem_Z[strToValidSymbol(substrings[0])]);
      list_of_atoms.push_back(at);
      n_atom++;
    }
    line_counter++; // Might not be useful anymore
  }
  // file has been read
  inp_file.close();
  // check if number of atoms matches size of the vector containing the atoms
  assert(n_atom == list_of_atoms.size());
  this->atoms = list_of_atoms;
  return;
}

/*void Model::lookUpRadii(std::string& filepath){

  return;
}*/
