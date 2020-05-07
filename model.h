#ifndef MODEL_H

#define MODEL_H

// class for dealing with program logic
#include <iostream>
#include <vector>
#include <unordered_map>

struct Atom;
class Space;
class Model{
  public:
    //* add member function to read in hashtable for radii from file
    //void readRadiiFromFile(std::string& radii_file);
    void readRadiiFromFile(std::string&);
    inline double findRadiusOfAtom(const std::string&);
    inline double findRadiusOfAtom(const Atom&); //* has not been tested
    void readAtomsFromFile(std::string&);
    // calls the Space constructor and creates a cell containing all atoms. Cell size is defined by atom positions
    void defineCell();
    void debug();
  private:
    std::vector<Atom> atoms;
    Space* cell;
    //* add hashtable for radii
    std::unordered_map<std::string,double> radii;
};

#endif
