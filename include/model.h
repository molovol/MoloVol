#ifndef MODEL_H

#define MODEL_H

// class for dealing with program logic
#include "atomtree.h"
#include "space.h"
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>

struct CalcResultBundle;
class AtomTree;
struct Atom;
class Space;
class Model{
  public:
    // radius file import
    bool readRadiiAndAtomNumFromFile(std::string&); // depreciated, use readRadiusFileSetMaps() instead
    bool readRadiusFileSetMaps(std::string&);
    std::unordered_map<std::string, double> importRadiusMap(const std::string&);
    // atom file import
    bool readAtomsFromFile(const std::string&, bool);
    void clearAtomData();
    bool readAtomFile(const std::string&, bool);
    void readFileXYZ(const std::string&);
    void readFilePDB(const std::string&, bool);

    // export
    bool createOutputFolder(std::string);
    void createReport(std::string, std::vector<std::string>, bool);
    void writeXYZfile(std::vector<std::tuple<std::string, double, double, double>>&, std::string);
    void writeSurfaceMap();

    std::vector<std::string> listElementsInStructure();

    // crystal unit cell related functions
    bool getSymmetryElements(std::string, std::vector<int>&, std::vector<double>&);
    bool processUnitCell(double, double, double, double);
    void orthogonalizeUnitCell();
    bool symmetrizeUnitCell();
    void moveAtomsInsideCell();
    void removeDuplicateAtoms();
    void countAtomsInUnitCell();
    void generateSupercell(double);
    void generateUsefulAtomMapFromSupercell(double);
    std::string generateUnitCellChemicalFormula(std::vector<std::string>);

    inline double findRadiusOfAtom(const std::string&);
    inline double findRadiusOfAtom(const Atom&); //TODO has not been tested

    // controller-model communication
    // calls the Space constructor and creates a cell containing all atoms. Cell size is defined by atom positions
    void defineCell(const double, const int);
    void setAtomListForCalculation(const std::vector<std::string>&, bool);
    void setAtomListForCalculation(const std::vector<std::string>&, std::vector<std::tuple<std::string, double, double, double>>&);
    void storeAtomsInTree();
    void linkAtomsToAdjacentAtoms(const double&);
    void linkToAdjacentAtoms(const double&, Atom&);
    CalcResultBundle calcVolume();
    std::vector<std::tuple<std::string, int, double>> generateAtomList();
    void setRadiusMap(std::unordered_map<std::string, double> map);
    bool setProbeRadii(const double&, const double&, bool);

    void debug();
  private:
    std::string calc_time; // stores the time when the calculation was run for output folder and report
    std::map<char,double> volumes_stored; // convenient access to calculated volumes for report
    std::string output_folder = "./"; // default folder is the program folder but it is changed with the output file routine
    std::vector<std::tuple<std::string, double, double, double>> raw_atom_coordinates;
    std::vector<std::tuple<std::string, double, double, double>> processed_atom_coordinates;
    double cell_param[6]; // unit cell parameters in order: A, B, C, alpha, beta, gamma
    double cart_matrix[3][3]; // cartesian coordinates of vectors A, B, C
    std::string space_group;
    std::unordered_map<std::string, double> radius_map;
    std::unordered_map<std::string, int> elem_Z;
    std::map<std::string, int> atom_amounts;
    std::map<std::string, int> unit_cell_atom_amounts; // stores atoms of unit cell to generate chemical formula
    std::vector<Atom> atoms;
    AtomTree atomtree;
    Space _cell;
    double _r_probe1 = 0;
    double _r_probe2 = 0;
};

struct CalcResultBundle{
  bool success = true;
  std::map<char,double> volumes;
  double type_assignment_elapsed_seconds;
  double volume_tally_elapsed_seconds;

  double getTime(){
    return type_assignment_elapsed_seconds+volume_tally_elapsed_seconds;
  }
};

#endif
