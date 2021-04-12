
#include "model.h"
#include "controller.h"
#include "atom.h"
#include "misc.h"
#include "special_chars.h"
#include <chrono>
#include <array>
#include <string>
#include <vector>


////////////////////////////////////
// CONTROLLER-MODEL COMMUNICATION //
////////////////////////////////////

bool Model::setParameters(std::string file_path,
            std::string output_dir,
            bool inc_hetatm,
            bool analyze_unit_cell,
            bool probe_mode,
            double r_probe1,
            double r_probe2,
            double grid_step,
            int max_depth,
            bool make_report,
            bool make_full_map,
            bool make_cav_maps,
            std::unordered_map<std::string, double> rad_map,
            std::vector<std::string> included_elem,
            double max_radius){
  if(!setProbeRadii(r_probe1, r_probe2, probe_mode)){
    _data.success = false;
    return false;
  }
  _data.atom_file_path = file_path;
  output_folder = output_dir;
  _data.inc_hetatm = inc_hetatm;
  _data.analyze_unit_cell = analyze_unit_cell;
  _data.grid_step = grid_step;
  _data.max_depth = max_depth;
  _data.make_report = make_report;
  _data.make_full_map = make_full_map;
  _data.make_cav_maps = make_cav_maps;
  radius_map = rad_map;
  _data.included_elements = included_elem;
  _max_atom_radius = max_radius;
  return true;
}

void Model::setTotalCalcTime(double elapsed_time){
  _data.total_elapsed_seconds = elapsed_time;
}

bool Model::setProbeRadii(const double r_1, const double r_2, const bool probe_mode){
  toggleProbeMode(probe_mode);
  setProbeRad1(r_1);
  if (optionProbeMode()){
    if (r_1 > r_2){
      Ctrl::getInstance()->notifyUser("Probes radii invalid!\nSet probe 2 radius > probe 1 radius.");
      return false;
    }
    else{
      setProbeRad2(r_2);
    }
  }
  return true;
}

CalcReportBundle Model::generateVolumeData(){
  // process atom data for unit cell analysis if the option it ticked
  if(optionAnalyzeUnitCell()){
    if(!processUnitCell()){
      _data.success = false;
      return _data;
    }
  }

  // determine which atoms will be taken into account
  setAtomListForCalculation();
  // place atoms in a binary tree for faster access
  storeAtomsInTree();
  // set size of the box containing all atoms
  defineCell();

  generateChemicalFormula();

  // create a report bundle
  return calcVolume();
}

void Model::setAtomListForCalculation(){
  std::vector<std::tuple<std::string,double,double,double>>& atom_coordinates = (_data.analyze_unit_cell) ? processed_atom_coordinates : raw_atom_coordinates;
  atoms.clear();

  for(size_t i = 0; i < atom_coordinates.size(); i++){
    if(isIncluded(std::get<0>(atom_coordinates[i]), _data.included_elements)){
      Atom at = Atom(std::get<1>(atom_coordinates[i]),
                     std::get<2>(atom_coordinates[i]),
                     std::get<3>(atom_coordinates[i]),
                     std::get<0>(atom_coordinates[i]),
                     radius_map[std::get<0>(atom_coordinates[i])],
                     elem_Z[std::get<0>(atom_coordinates[i])]);
      atoms.push_back(at);
    }
  }
}

// generates a simple table to be displayed by the GUI. only uses standard library and base types in order to
// avoid dependency issues
std::vector<std::tuple<std::string, int, double>> Model::generateAtomList(){
  std::vector<std::tuple<std::string, int, double>> atoms_for_list;
  // Element0: Element symbol
  // Element1: Number of Atoms with that symbol
  // Element2: Radius
  for(auto elem : atom_amounts){
    atoms_for_list.push_back(std::make_tuple(elem.first, elem.second, radius_map[elem.first]));
  }
  return atoms_for_list;
}

// TODO should be obselete because radius_map is set by setParameters
// but it is still used in Model::readRadiusFileSetMaps and Ctrl::unittestExcluded()
void Model::setRadiusMap(std::unordered_map<std::string, double> map){
  radius_map = map;
  return;
}

void Model::generateChemicalFormula(){
  std::string chemical_formula_suffix = "";
  std::string chemical_formula_prefix = "";
  std::map<std::string, int> atom_list = (_data.analyze_unit_cell) ? unit_cell_atom_amounts : atom_amounts;
  for(auto elem : atom_list){
    std::string symbol = elem.first;
    // if element is included by user in gui
    if (isIncluded(symbol, _data.included_elements)){
      std::string subscript = std::to_string(elem.second);
      // by convention: carbon comes first, then hydrogen, then in alphabetical order
      if (symbol == "C"){
        chemical_formula_prefix = symbol + subscript + chemical_formula_prefix;
      }
      else if (symbol == "H"){
        chemical_formula_prefix += symbol + subscript;
      }

      else {
        chemical_formula_suffix += symbol + subscript;
      }
    }
  }
  _data.chemical_formula = chemical_formula_prefix + chemical_formula_suffix;
}

// TODO remove is unused
CalcReportBundle Model::getBundle(){
  return _data;
}

///////////////////////////
// CALCULATION FUNCTIONS //
///////////////////////////

void Model::defineCell(){
  _cell = Space(atoms, _data.grid_step, _data.max_depth, optionProbeMode()? getProbeRad2() : getProbeRad1());
  return;
}

void Model::storeAtomsInTree(){
  atomtree = AtomTree(atoms);
}

CalcReportBundle Model::calcVolume(){
  // save the date and time of calculation for output files
  _time_stamp = timeNow();

  auto start = std::chrono::steady_clock::now();
  _cell.assignTypeInGrid(atomtree, getProbeRad1(), getProbeRad2(), optionProbeMode()); // assign each voxel in grid a type
  auto end = std::chrono::steady_clock::now();
  _data.type_assignment_elapsed_seconds = std::chrono::duration<double>(end-start).count();

  // TODO remove when unnecessary
  //_cell.printGrid(); // for testing

  start = std::chrono::steady_clock::now();
  _data.volumes = _cell.getVolume();
  end = std::chrono::steady_clock::now();
  _data.volume_tally_elapsed_seconds = std::chrono::duration<double>(end-start).count();

  return _data;
}

// TODO remove when unnecessary
void Model::debug(){
  std::array<double,3> cell_min = _cell.getMin();
  std::array<double,3> cell_max = _cell.getMax();

  for(int dim = 0; dim < 3; dim++){
    std::cout << "Cell Limit in Dim " << dim << ":" << cell_min[dim] << " and " << cell_max[dim] << std::endl;
  }
  return;
}

////////////////////////////////////////////
// CRYSTAL UNIT CELL PROCESSING FUNCTIONS //
////////////////////////////////////////////

/*
To obtain data usable by the software from a unit cell, we need
1) to apply symmetry from the space group to the atom supplied in the structure file
2) to convert non-orthogonal unit cells in the corresponding orthogonal cells (rectangular parallelepiped) that will be divided in cubic voxels
3) to move atoms atoms inside this orthogonal cell
4) to generate a supercell to analyze the atoms surrounding the cell that will touch the cell and/or probes
5) to remove atoms at the limits of the supercell that are useless for the calculation (to increase the efficiency)
*/

bool Model::processUnitCell(){
  /* This data processing is performed by the functions below:
  1) find orthogonal unit cell matrix
  2) create symmetry elements from base structure
  3) if atoms outside the orthogonal cell, move them inside
  4) remove duplicate atoms (allow 0.01-0.05 A error)
  5) create supercell at least 3x3x3 but big enough to include a radius around central unit cell = 2*(largestAtom+probe1+probe2+gridstep)
  6) create atom map based on unit cell limits + radius= 2*(max_atom_rad+probe1+probe2+gridstep)
  7) write structure file with processed atom list
  */
  double radius_limit = 2*(_max_atom_radius+getProbeRad1()+getProbeRad2()+_data.grid_step);
  if(space_group == ""){
    Ctrl::getInstance()->notifyUser("Space group not found!\nCheck the structure file\nor untick the unit cell analysis checkbox.");
    return false;
  }
  for(int i = 0; i < 6; i++){
    if(cell_param[i] == 0){
      Ctrl::getInstance()->notifyUser("Unit cell parameters invalid!\nCheck the structure file\nor untick the unit cell analysis checkbox.");
      return false;
    }
  }
  processed_atom_coordinates.clear();
  processed_atom_coordinates = raw_atom_coordinates;
  orthogonalizeUnitCell();
  if(!symmetrizeUnitCell()){
    return false;
  }
  moveAtomsInsideCell();
  removeDuplicateAtoms();
  countAtomsInUnitCell(); // for report
  writeXYZfile(processed_atom_coordinates, "orthogonal_cell");
  generateSupercell(radius_limit);
  generateUsefulAtomMapFromSupercell(radius_limit);
  writeXYZfile(processed_atom_coordinates, "orthogonal_cell_with_neighboring_atoms");
  return true;
}

// 1) find orthogonal unit cell matrix = cartesian coordinates of the unit cell axes
void Model::orthogonalizeUnitCell(){
  /*
  Formulae to find the cartesian coordinates of axes A, B, C:
  Ax , Ay , Az
  Bx , By , Bz
  Cx , Cy , Cz
  =
  A , 0 , 0
  B*cos(gamma) , B*sin(gamma) , 0
  C*cos(beta) , C*((cos(alpha)*sin(gamma))+((cos(beta)-(cos(alpha)*cos(gamma)))*sin(gamma-90)/cos(gamma-90))) , Sqrt(C^2-Cx^2-Cy^2)

  Notes:
  alpha: angle between B and C
  beta: angle between A and C
  gamma: angle between A and B

  The origin of ABC and xyz is 0,0,0
  A is always along cartesian x axis
  B is always on the cartesian xy plane with a positive y component (By > 0)
  C is the only axis containing a cartesian z component (Cz > 0)
  */
  const double pi = 3.14159265358979323846;
  // convert unit cell angles from degree to radian
  double alpha = cell_param[3]*pi/180;
  double beta = cell_param[4]*pi/180;
  double gamma = cell_param[5]*pi/180;
  cart_matrix[0][0] = cell_param[0]; // A is always along x axis => Ax = X
  cart_matrix[0][1] = 0; // A is always along x axis => Ay = 0
  cart_matrix[0][2] = 0; // A is always along x axis => Az = 0
  cart_matrix[1][2] = 0; // B is always on xy plane => Bz = 0
  if(cell_param[5] == 90){ // if B is along y axis, no need to do calculations that could result in approximations
    cart_matrix[1][0] = 0;
    cart_matrix[1][1] = cell_param[1];
  }
  else{
    cart_matrix[1][0] = cell_param[1]*std::cos(gamma);
    cart_matrix[1][1] = cell_param[1]*std::sin(gamma);

  }
  if(cell_param[3] == 90 && cell_param[4] == 90 ){ // if C is along z axis, no need to do calculations that could result in approximations
    cart_matrix[2][0] = 0;
    cart_matrix[2][1] = 0;
    cart_matrix[2][2] = cell_param[2];
  }
  else{
    cart_matrix[2][0] = cell_param[2]*std::cos(beta);
    cart_matrix[2][1] = cell_param[2]*((std::cos(alpha)*std::sin(gamma))+((std::cos(beta)-(std::cos(alpha)*std::cos(gamma)))*std::sin(gamma-(pi/2))/std::cos(gamma-(pi/2))));
    cart_matrix[2][2] = std::sqrt(pow(cell_param[2],2)-pow(cart_matrix[2][0],2)-pow(cart_matrix[2][1],2));
  }
  return;
}

// 2) create symmetry elements from base structure
bool Model::symmetrizeUnitCell(){
  std::vector<int> sym_matrix_XYZ;
  std::vector<double> sym_matrix_fraction;
  if(!getSymmetryElements(space_group, sym_matrix_XYZ, sym_matrix_fraction)){
    Ctrl::getInstance()->notifyUser("Space group or symmetry not found!\nCheck the structure and space group files\nor untick the unit cell analysis checkbox.");
    return false;
  }
  /* To convert cartesian coordinates x y z in unit cell coordinates a b c:
  c = z/Cz
  b = (y-(Cy*z/Cz))/By
    = (y-(Cy*c))/By
  a = (x-(Cx*z/Cz)-(Bx*(y-(Cy*z/Cz))/By))/Ax
    = (x-(Cx*c)-(Bx*b))/Ax
  */
  std::vector<std::tuple<std::string, double, double, double>> ABC_coord;
  int atom_number = processed_atom_coordinates.size();
  int sym_number = sym_matrix_XYZ.size()/9;
  for (int i = 0; i < atom_number; i++){
    double atom_x = std::get<1>(processed_atom_coordinates[i]);
    double atom_y = std::get<2>(processed_atom_coordinates[i]);
    double atom_z = std::get<3>(processed_atom_coordinates[i]);
    double atom_abc[3];
    atom_abc[2] = atom_z/cart_matrix[2][2];
    atom_abc[1] = (atom_y-(cart_matrix[2][1]*atom_abc[2]))/cart_matrix[1][1];
    atom_abc[0] = (atom_x-(cart_matrix[2][0]*atom_abc[2])-(cart_matrix[1][0]*atom_abc[1]))/cart_matrix[0][0];
    for (int j = 0; j < sym_number; j++){
      double sym_abc[3];
      for (int k = 0; k < 3; k++){
        sym_abc[k] = sym_matrix_fraction[3*j+k];
        for (int n = 0; n < 3; n++){
          sym_abc[k] += sym_matrix_XYZ[9*j+3*k+n]*atom_abc[n];
        }
      }
      ABC_coord.emplace_back(std::get<0>(processed_atom_coordinates[i]), sym_abc[0], sym_abc[1], sym_abc[2]);
    }
  }
  for (size_t i = 0; i < ABC_coord.size(); i++){ // converts new list of atoms after applying symmetry to cartesian coordinates
    double atom_xyz[3] = {0,0,0};
    double atom_abc[3] = {std::get<1>(ABC_coord[i]), std::get<2>(ABC_coord[i]), std::get<3>(ABC_coord[i])};
    for (int j = 0; j < 3; j++){
      for (int k = 0; k < 3; k++){
        atom_xyz[j] += atom_abc[k]*cart_matrix[k][j];
      }
    }
    processed_atom_coordinates.emplace_back(std::get<0>(ABC_coord[i]), atom_xyz[0], atom_xyz[1], atom_xyz[2]);
  }
  return true;
}

// 3) if atoms outside the orthogonal cell, move them inside
void Model::moveAtomsInsideCell(){
  for(size_t n = 0; n < processed_atom_coordinates.size(); n ++){
    while(std::get<3>(processed_atom_coordinates[n]) < 0){ // need to start with Z axis because the unit cell C axis can have X and Y components
      std::get<3>(processed_atom_coordinates[n]) += cart_matrix[2][2];
      std::get<2>(processed_atom_coordinates[n]) += cart_matrix[2][1];
      std::get<1>(processed_atom_coordinates[n]) += cart_matrix[2][0];
    }
    while(std::get<3>(processed_atom_coordinates[n]) > cart_matrix[2][2]){
      std::get<3>(processed_atom_coordinates[n]) -= cart_matrix[2][2];
      std::get<2>(processed_atom_coordinates[n]) -= cart_matrix[2][1];
      std::get<1>(processed_atom_coordinates[n]) -= cart_matrix[2][0];
    }
    while(std::get<2>(processed_atom_coordinates[n]) < 0){ // then check Y axis because the unit cell B axis can have a X component
      std::get<2>(processed_atom_coordinates[n]) += cart_matrix[1][1];
      std::get<1>(processed_atom_coordinates[n]) += cart_matrix[1][0];
    }
    while(std::get<2>(processed_atom_coordinates[n]) > cart_matrix[1][1]){
      std::get<2>(processed_atom_coordinates[n]) -= cart_matrix[1][1];
      std::get<1>(processed_atom_coordinates[n]) -= cart_matrix[1][0];
    }
    while(std::get<1>(processed_atom_coordinates[n]) < 0){
      std::get<1>(processed_atom_coordinates[n]) += cart_matrix[0][0];
    }
    while(std::get<1>(processed_atom_coordinates[n]) > cart_matrix[0][0]){
      std::get<1>(processed_atom_coordinates[n]) -= cart_matrix[0][0];
    }
  }
}

// 4) remove duplicate atoms (allow 0.01-0.05 A error)
void Model::removeDuplicateAtoms(){
  for(size_t i = 0; i < processed_atom_coordinates.size(); i++){
    for(size_t j = i+1; j < processed_atom_coordinates.size(); j++){
      if(std::get<0>(processed_atom_coordinates[i]) == std::get<0>(processed_atom_coordinates[j]) &&
         std::abs(std::get<1>(processed_atom_coordinates[i])-std::get<1>(processed_atom_coordinates[j])) < 0.05 &&
         std::abs(std::get<2>(processed_atom_coordinates[i])-std::get<2>(processed_atom_coordinates[j])) < 0.05 &&
         std::abs(std::get<3>(processed_atom_coordinates[i])-std::get<3>(processed_atom_coordinates[j])) < 0.05
         ){
           processed_atom_coordinates.erase(processed_atom_coordinates.begin() + j);
           j--;
      }
    }
  }
}

// 4b) create chemical formula of a unit cell for report
void Model::countAtomsInUnitCell(){
  unit_cell_atom_amounts.clear();
  for(size_t i = 0; i < processed_atom_coordinates.size(); i++){
    std::string symbol = std::get<0>(processed_atom_coordinates[i]);
    unit_cell_atom_amounts[symbol]++;
  }
}

// 5) create supercell at least 3x3x3 but big enough to include a radius around central unit cell = 2*(max_atom_rad+probe1+probe2+gridstep)
void Model::generateSupercell(double radius_limit){
  int initial_number_of_atoms = processed_atom_coordinates.size();
  if(cell_param[3] == 90 && cell_param[4] == 90 && cell_param[5] == 90){ // for orthogonal space groups, the algorithm is considerably simpler than for other space groups
    // determine how many cells will be needed in each dimension for the supercell
    int surrounding_cells[3];
    for(int i = 0; i < 3; i++){
      surrounding_cells[i] = 1 + (int)(radius_limit/cart_matrix[i][i]);
    }
    for(int i = -surrounding_cells[0]; i <= surrounding_cells[0]; i++){
      for(int j = -surrounding_cells[1]; j <= surrounding_cells[1]; j++){
        for(int k = -surrounding_cells[2]; k <= surrounding_cells[2]; k++){
          for(int n = 0; n < initial_number_of_atoms; n++){
            // duplicate atoms in each cell of supercell beside the original central cell
            if(i != 0 || j != 0 || k != 0){
              processed_atom_coordinates.emplace_back(std::get<0>(processed_atom_coordinates[n]),
                                            std::get<1>(processed_atom_coordinates[n])+(i*cart_matrix[0][0]),
                                            std::get<2>(processed_atom_coordinates[n])+(j*cart_matrix[1][1]),
                                            std::get<3>(processed_atom_coordinates[n])+(k*cart_matrix[2][2]));
            }
          }
        }
      }
    }
  }
  else { // for non orthogonal space groups, we need to extend the supercell in a specific order because B and C axes can have components in X and/or Y
    // determine how many cells will be needed in each dimension for the supercell
    int C_number = 1 + (int)(radius_limit/cart_matrix[2][2]);
    for(int i = -C_number; i <= C_number; i++){
      int B_number_neg = -1 + (int)((-radius_limit-(i*cart_matrix[2][1]))/cart_matrix[1][1]);
      int B_number_pos = 1 + (int)((radius_limit-(i*cart_matrix[2][1]))/cart_matrix[1][1]);
      for(int j = B_number_neg; j <= B_number_pos; j++){
        int A_number_neg = -1 + (int)((-radius_limit-(i*cart_matrix[2][0])-(j*cart_matrix[1][0]))/cart_matrix[0][0]);
        int A_number_pos = 1+ (int)((radius_limit-(i*cart_matrix[2][0])-(j*cart_matrix[1][0]))/cart_matrix[0][0]);
        for(int k = A_number_neg; k <= A_number_pos; k++){
          for(int n = 0; n < initial_number_of_atoms; n++){
            // duplicate atoms in each cell of supercell beside the original central cell
            if(i != 0 || j != 0 || k != 0){
                processed_atom_coordinates.emplace_back(std::get<0>(processed_atom_coordinates[n]),
                                              std::get<1>(processed_atom_coordinates[n])+(k*cart_matrix[0][0])+(j*cart_matrix[1][0])+(i*cart_matrix[2][0]),
                                              std::get<2>(processed_atom_coordinates[n])+(j*cart_matrix[1][1])+(i*cart_matrix[2][1]),
                                              std::get<3>(processed_atom_coordinates[n])+(i*cart_matrix[2][2]));
            }
          }
        }
      }
    }
  }
}

// 6) create atom map based on cell limits + radius= 2*(max_atom_rad+probe1+probe2+gridstep)
void Model::generateUsefulAtomMapFromSupercell(double radius_limit){
  for(size_t i = 0; i < processed_atom_coordinates.size(); i++){
    if(std::get<1>(processed_atom_coordinates[i]) < -radius_limit ||
       std::get<2>(processed_atom_coordinates[i]) < -radius_limit ||
       std::get<3>(processed_atom_coordinates[i]) < -radius_limit ||
       std::get<1>(processed_atom_coordinates[i]) > cart_matrix[0][0]+radius_limit ||
       std::get<2>(processed_atom_coordinates[i]) > cart_matrix[1][1]+radius_limit ||
       std::get<3>(processed_atom_coordinates[i]) > cart_matrix[2][2]+radius_limit
    ){
      processed_atom_coordinates.erase(processed_atom_coordinates.begin() + i);
           i--;
    }
  }
}
