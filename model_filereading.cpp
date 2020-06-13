#include "model.h"
#include "atom.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream> // stringstream
#include <cassert>
#include <exception>

// split line into substrings when separated by whitespaces
static inline std::vector<std::string> splitLine(std::string& line){ 
  std::istringstream iss(line);
  std::vector<std::string> substrings((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
  return substrings;
}

// reads radii from a file specified by the filepath and 
// stores them in an unordered map that is an attribute of
// the Model object
void Model::readRadiiFromFile(std::string& filepath){
 
  //TODO add case handling for when file has already been read
  //TODO if map empty: ask user via dialog boy if they want to
  //TODO reimport the file
  //TODO make a function "consultUser(string)"
  //if(radii.empty()){std::cout << "empty" << std::endl;}

  std::string line;
  std::ifstream inp_file(filepath);

  while(getline(inp_file,line)){
    std::vector<std::string> substrings = splitLine(line);
    if(substrings.size() == 2){
      radii[substrings[0]] = std::stod(substrings[1]);
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

  int n_atom;
  std::vector<Atom> list_of_atoms;

//if (inp_file.is_open()){  //TODO consider adding an exception, for when file in not valid

  // we iterate through the lines in the input file
  std::string line;
  std::ifstream inp_file(filepath);

  int line_counter = 0;
  // iterate through lines
  while(getline(inp_file,line)){
  
    if(line_counter==0){ // first line contains number of atoms
      n_atom = std::stoi(line);
    }
    

    else if(line_counter == 1){ // comment line (ignore)
    }
    
    else { // subsequent lines contain atom positional data
      std::vector<std::string> substrings = splitLine(line);

      // create new atom and add to storage vector (assumes file format: Element_Symbol x y z)
      Atom at = Atom(std::stod(substrings[1]), 
                     std::stod(substrings[2]), 
                     std::stod(substrings[3]), 
                     substrings[0], 
                     radii[substrings[0]]);
      list_of_atoms.push_back(at);
    }
    line_counter++;
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
