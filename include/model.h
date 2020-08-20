#ifndef MODEL_H

#define MODEL_H

// class for dealing with program logic
#include "atomtree.h"
#include "space.h"
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <filesystem>

namespace fs = std::filesystem;

class AtomTree;
struct Atom;
class Space;
class Model{
  public:
    void importFiles(std::string&, std::string&, bool);
    void readRadiiAndAtomNumFromFile(std::string&);
    void readAtomsFromFileXYZ(std::string&);
    void readAtomsFromFilePDB(std::string&, bool);
    bool importFilesChanged(std::string&, std::string&);
    bool filesExist(const std::array<std::string,2>& paths) const;
    bool filesExist(const std::string& path1, const std::string& path2) const;

    inline double findRadiusOfAtom(const std::string&);
    inline double findRadiusOfAtom(const Atom&); //TODO has not been tested
    // calls the Space constructor and creates a cell containing all atoms. Cell size is defined by atom positions
    void defineCell(const double&, const int&);
    void storeAtomsInTree();
    void updateAtomRadii();
    void findCloseAtoms(const double&); //TODO
    void calcVolume();
    std::vector<std::tuple<std::string, int, double>> generateAtomList();
    void setRadiusMap(std::unordered_map<std::string, double> map);
    void debug();
  private:
    std::unordered_map<std::string, double> radius_map;
    std::unordered_map<std::string, int> elem_Z;
    std::map<std::string, int> atom_amounts;

    std::array<fs::path,2> filepaths_last_imported;
    std::array<fs::file_time_type,2> files_last_written;

    std::vector<Atom> atoms;
    AtomTree atomtree;
    Space cell;
};

#endif
