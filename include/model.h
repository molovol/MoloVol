#ifndef MODEL_H

#define MODEL_H

// class for dealing with program logic
#include "atomtree.h"
#include "space.h"
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <math.h>

struct CalcReportBundle{
  // calculation returned without error
  bool success = true;
  // file paths
  std::string atom_file_path;
  // switches
  bool inc_hetatm;
  bool analyze_unit_cell;
  bool probe_mode;
  // parameters for calculation
  double grid_step;
  int max_depth;
  double r_probe1 = 0;
  double r_probe2 = 0;
  std::vector<std::string> included_elements;
  std::string chemical_formula;
  // output options
  bool make_report;
  bool make_full_map;
  bool make_cav_maps;
  // calculation results
  std::map<char,double> volumes;
  std::vector<double> cavities;
  std::map<char,double> surfaces;
  // time
  std::vector<double> elapsed_seconds;
  void addTime(const double t){elapsed_seconds.push_back(t);}
  double getTime(const unsigned i){return elapsed_seconds[i];}
  double getTime(){
    double total_seconds = 0;
    for (const double time : elapsed_seconds){
      total_seconds += time;
    }
    return total_seconds;
  }
};

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
    void createReport();
    void writeXYZfile(std::vector<std::tuple<std::string, double, double, double>>&, std::string);
    void writeSurfaceMap();

    std::vector<std::string> listElementsInStructure();

    // crystal unit cell related functions
    bool getSymmetryElements(std::string, std::vector<int>&, std::vector<double>&);
    bool processUnitCell();
    void orthogonalizeUnitCell();
    bool symmetrizeUnitCell();
    void moveAtomsInsideCell();
    void removeDuplicateAtoms();
    void countAtomsInUnitCell();
    void generateSupercell(double);
    void generateUsefulAtomMapFromSupercell(double);

    inline double findRadiusOfAtom(const std::string&);
    inline double findRadiusOfAtom(const Atom&); //TODO has not been tested

    // controller-model communication
    CalcReportBundle generateVolumeData();
    // calls the Space constructor and creates a cell containing all atoms. Cell size is defined by atom positions
    void defineCell();
    void setAtomListForCalculation();
    void storeAtomsInTree();
    void linkAtomsToAdjacentAtoms(const double&);
    void linkToAdjacentAtoms(const double&, Atom&);
    CalcReportBundle calcVolume();
    bool setParameters(std::string, std::string, bool, bool, bool, double, double, double, int, bool, bool, bool, std::unordered_map<std::string, double>, std::vector<std::string>, double);
    CalcReportBundle getBundle(); // TODO remove is unused
    std::vector<std::tuple<std::string, int, double>> generateAtomList();
    void setRadiusMap(std::unordered_map<std::string, double> map);
    bool setProbeRadii(const double, const double, const bool);
    void generateChemicalFormula();

  private:
    CalcReportBundle _data;
    std::string _time_stamp; // stores the time when the calculation was run for output folder and report
    std::string output_folder = "."; // default folder is the program folder but it is changed with the output file routine
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
    double _max_atom_radius = 0;
 
    // access functions for information stored in data
    double getCalcTime(){return _data.getTime();}
    double getProbeRad1(){return _data.r_probe1;}
    void setProbeRad1(double r){_data.r_probe1 = r;}
    double getProbeRad2(){return _data.r_probe2;}
    void setProbeRad2(double r){_data.r_probe2 = r;}
    bool optionProbeMode(){return _data.probe_mode;}
    void toggleProbeMode(bool state){_data.probe_mode = state;}
    bool optionIncludeHetatm(){return _data.inc_hetatm;}
    bool optionAnalyzeUnitCell(){return _data.analyze_unit_cell;}
    bool optionAnalyseUnitCell(){return _data.analyze_unit_cell;}
};



#endif
