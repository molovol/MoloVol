#ifndef MODEL_H

#define MODEL_H

// class for dealing with program logic
#include "atomtree.h"
#include "space.h"
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>

class AtomTree;
struct Atom;
class Space;
class Model{
  public:
    void readRadiiAndAtomNumFromFile(std::string&);
    inline double findRadiusOfAtom(const std::string&);
    inline double findRadiusOfAtom(const Atom&); //TODO has not been tested
    void listAtomTypesFromFile(std::string&);
    void readAtomsFromFile(std::string&);
    // calls the Space constructor and creates a cell containing all atoms. Cell size is defined by atom positions
    void defineCell(const double&, const int&);
    void storeAtomsInTree();
    void findCloseAtoms(const double&); //TODO
    void calcVolume();
    std::vector<std::tuple<std::string, int, double>> generateAtomList();
    // TODO: move to cpp
    void setRadiusMap(std::unordered_map<std::string, double> map);
    void debug();
  private:
    std::unordered_map<std::string, double> radius_map;
    std::unordered_map<std::string, int> elem_Z;
    std::map<std::string, int> atom_amounts;
    
    std::vector<Atom> atoms;
    AtomTree atomtree;
    Space cell;
};

#endif
