#ifndef MODEL_H

#define MODEL_H

// class for dealing with program logic
#include "atomtree.h"
#include "space.h"
#include "cavity.h"
#include "importmanager.h"
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <math.h>

struct CalcReportBundle{
  // calculation returned without error
  bool success;
  // file paths
  std::string atom_file_path;
  // switches
  bool inc_hetatm;
  bool analyze_unit_cell;
  bool calc_surface_areas;
  bool probe_mode;
  // parameters for calculation
  double grid_step;
  int max_depth;
  double r_probe1;
  double r_probe2;
  std::vector<std::string> included_elements;
  std::string chemical_formula;
  double molar_mass;
  // output options
  bool make_report;
  bool make_full_map;
  bool make_cav_maps;
  // crystallographic structures
  std::vector<std::tuple<std::string, double, double, double>> orth_cell;
  std::vector<std::tuple<std::string, double, double, double>> supercell;
  // volumes
  std::map<char,double> volumes;
  // surfaces
  double surf_vdw;
  double surf_molecular;
  double surf_probe_excluded;
  double surf_probe_accessible;
  double getSurfVdw(){return surf_vdw;}
  double getSurfMolecular(){return surf_molecular;}
  double getSurfProbeExcluded(){return surf_probe_excluded;}
  double getSurfProbeAccessible(){return surf_probe_accessible;}
  // cavity volumes and surfaces
  std::vector<Cavity> cavities;
  double getCavVolume(const unsigned char i){return cavities[i].getVolume();}
  std::array<double,3> getCavCentre(const unsigned char);
  std::array<double,3> getCavCenter(const unsigned char i){return getCavCentre(i);}
  double getCavSurfCore(const unsigned char i) const {return cavities[i].getSurfCore();}
  double getCavSurfShell(const unsigned char i) const {return cavities[i].getSurfShell();}
  // time
  std::vector<double> elapsed_seconds;
  void addTime(const double t){elapsed_seconds.push_back(t);}
  double getTime(const unsigned i){return elapsed_seconds[i];}
  double getTime();
};

namespace ImportMngr{struct UnitCell;}

class AtomTree;
struct Atom;
class Space;
class Model{
  typedef std::array<std::array<double,3>,3> MatR3;
  typedef ImportMngr::SymbolPositionPair SymbolPositionPair;
  typedef ImportMngr::UnitCell UnitCell;
  typedef std::vector<std::tuple<std::string, double, double, double>> RawAtomData;
  public:
    // elements file import
    bool importElemFile(const std::string&);
    std::unordered_map<std::string, double> extractRadiusMap(const std::string&);
    // atom file import
    bool readAtomsFromFile(const std::string&, bool);
    void clearAtomData();

    // export
    void createReport();
    void createReport(std::string);
    void writeCrystStruct();
    void writeCrystStruct(std::string);
    void writeXYZfile(const RawAtomData&, const std::string, const std::string);
    void writeTotalSurfaceMap();
    void writeTotalSurfaceMap(const std::string);
    void writeCavitiesMaps();
    void writeCavitiesMaps(const std::string);
    void writeSurfaceMap(const std::string, double, std::array<unsigned long int,3>, 
        std::array<double,3>, std::array<unsigned int,3>, std::array<unsigned int,3>, 
        const bool=false, const unsigned char=0);

    std::vector<std::string> listElementsInStructure();

    // crystal unit cell related functions
    bool processUnitCell();

    double findRadiusOfAtom(std::string) const;
    double findRadiusOfAtom(const Atom&) const;

    // controller-model communication
    CalcReportBundle generateData();
    CalcReportBundle generateVolumeData();
    CalcReportBundle generateSurfaceData();
    const Container3D<Voxel>& getSurfaceData() const;
    std::array<double,3> getCellOrigin() const;
    const AtomTree& getAtomTree() const;

    // calls the Space constructor and creates a cell containing all atoms. Cell size is defined by atom positions
    void defineCell();
    std::vector<Atom> convertAtomCoordinates(const RawAtomData&, const std::vector<std::string>&);
    void linkAtomsToAdjacentAtoms(const double&);
    void linkToAdjacentAtoms(const double&, Atom&);
    bool setParameters(const std::string, const std::string, const bool, const bool, 
        const bool, const bool, const double, const double, const double, const int, 
        const bool, const bool, const bool, const std::unordered_map<std::string, double>, 
        const std::vector<std::string>);
    std::vector<std::tuple<std::string, int, double>> generateAtomList();
    void setRadiusMap(std::unordered_map<std::string, double> map);
    std::unordered_map<std::string,double> getRadiusMap();
    bool setProbeRadii(const double, const double, const bool);

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
    bool optionCalcSurfaceAreas(){return _data.calc_surface_areas;}

  private:
    CalcReportBundle _data;
    std::string _time_stamp; // stores the time when the calculation was run for output folder and report
    std::string _output_folder = "."; // default folder is the program folder but it is changed with the output file routine
    RawAtomData _raw_atom_coordinates;
    RawAtomData _processed_atom_coordinates;
    std::array<double,6> _cell_param; // unit cell parameters in order: A, B, C, alpha, beta, gamma
    MatR3 _cart_matrix; // cartesian coordinates of vectors A, B, C
    std::string _space_group;
    std::vector<int> _sym_matrix_XYZ;
    std::vector<double> _sym_matrix_fraction;
    std::unordered_map<std::string, double> _radius_map;
    std::unordered_map<std::string, double> _elem_weight;
    std::unordered_map<std::string, int> _elem_Z;
    std::vector<Atom> _atoms;
    Space _cell;
    double _max_atom_radius = 0;

    void prepareVolumeCalc();
    std::map<std::string, int> atomCount(const RawAtomData&);
    std::map<std::string, int> atomCount(const std::vector<Atom>&);

    // crystal unit cell related functions
    bool getSymmetryElements(std::string, std::vector<int>&, std::vector<double>&);
    bool symmetrizeUnitCell();
    void moveAtomsInsideCell();
    void removeDuplicateAtoms();
    void generateSupercell(double);
    void generateUsefulAtomMapFromSupercell(double);
};



#endif
