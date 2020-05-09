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
    void readRadiiFromFile(std::string&);
    inline double findRadiusOfAtom(const std::string&);
    inline double findRadiusOfAtom(const Atom&); //* has not been tested
    void readAtomsFromFile(std::string&);
    // calls the Space constructor and creates a cell containing all atoms. Cell size is defined by atom positions
    void defineCell();
    void defineCell(const double&, const int&);
    //void calcVolume(const double&);
    void debug();
  private:
    std::vector<Atom> atoms;
    Space* cell;
    std::unordered_map<std::string,double> radii;
};

#endif
