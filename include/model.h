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
    // file reading
    bool readRadiiAndAtomNumFromFile(std::string&); // depreciated, use readRadiusFileSetMaps() instead
    bool readRadiusFileSetMaps(std::string&);
    bool readAtomsFromFile(const std::string&, bool);
    void readFileXYZ(const std::string&);
    void readFilePDB(const std::string&, bool);

    bool getSymmetryElements(std::string, std::vector<int>&, std::vector<double>&);
    bool createOutputFolder();
    void createReport(std::string, std::vector<std::string>);
    void writeXYZfile(std::vector<std::tuple<std::string, double, double, double>>&, std::string);
    bool processUnitCell(double, double, double, double);
    void orthogonalizeUnitCell();
    bool symmetrizeUnitCell();
    void moveAtomsInsideCell();
    void removeDuplicateAtoms();
    void generateSupercell(double);
    void generateUsefulAtomMapFromSupercell(double);

    inline double findRadiusOfAtom(const std::string&);
    inline double findRadiusOfAtom(const Atom&); //TODO has not been tested

    // controller-model communication
    // calls the Space constructor and creates a cell containing all atoms. Cell size is defined by atom positions
    void defineCell(const double&, const int&);
    void setAtomListForCalculation(const std::vector<std::string>&, bool);
    void storeAtomsInTree();
    void linkAtomsToAdjacentAtoms(const double&);
    void linkToAdjacentAtoms(const double&, Atom&);
    void calcVolume();
    std::vector<std::tuple<std::string, int, double>> generateAtomList();
    void setRadiusMap(std::unordered_map<std::string, double> map);
    bool setProbeRadii(const double&, const double&, bool);

    void debug();
  private:
    std::string output_folder = "./"; // default folder is the program folder but it is changed with the output file routine
    std::vector<std::tuple<std::string, double, double, double>> raw_atom_coordinates;
    std::vector<std::tuple<std::string, double, double, double>> processed_atom_coordinates;
    double cell_param[6]; // unit cell parameters in order: A, B, C, alpha, beta, gamma
    double cart_matrix[3][3]; // cartesian coordinates of vectors A, B, C
    std::string space_group;
    std::unordered_map<std::string, double> raw_radius_map;
    std::unordered_map<std::string, double> radius_map;
    std::unordered_map<std::string, int> elem_Z;
    std::map<std::string, int> atom_amounts;
    std::vector<Atom> atoms;
    AtomTree atomtree;
    Space cell;
    double r_probe1 = 0;
    double r_probe2 = 0;
};

#endif
