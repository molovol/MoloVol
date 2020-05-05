#include "model.h"
#include "atom.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream> // stringstream
#include <cassert>

// split line into substrings when separated by whitespaces
static inline std::vector<std::string> splitLine(std::string& line){ 
  std::istringstream iss(line);
  std::vector<std::string> substrings((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
  return substrings;
}

//std::vector<Atom> readAtomsFromFile(std::string& filepath){
void Model::readAtomsFromFile(std::string& filepath){

  int n_atom;
  std::vector<Atom> list_of_atoms; // will contain the atoms

//if (inp_file.is_open()){  //* consider adding an exception, for when file in not valid

  // we iterate through the lines in the input file
  std::string line;
  std::ifstream inp_file(filepath);
  // init counter to keep track of line number
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

      // create new atom and add to storage vector (assumes format: Element_Symbol x y z)
      Atom at = Atom(std::stod(substrings[1]), std::stod(substrings[2]), std::stod(substrings[3]), substrings[0]);
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
