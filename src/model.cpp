#include "model.h"
#include "crystallographer.h"
#include "controller.h"
#include "atom.h"
#include "misc.h"
#include "special_chars.h"
#include "exception.h"
#include <chrono>
#include <array>
#include <string>
#include <vector>

//////////////////////
// CALCRESULTBUNDLE //
//////////////////////

std::array<double,3> CalcReportBundle::getCavCentre(const unsigned char i){
  std::array<double,3> cav_ctr;
  for (char j = 0; j < 3; ++j){
    cav_ctr[j] = (cavities[i].min_bound[j] + cavities[i].max_bound[j])/2;
  }
  return cav_ctr;
}

double CalcReportBundle::getTime(){
  double total_seconds = 0;
  for (const double time : elapsed_seconds){
    total_seconds += time;
  }
  return total_seconds;
}

////////////////////////////////////
// CONTROLLER-MODEL COMMUNICATION //
////////////////////////////////////

bool Model::setParameters(const std::string file_path,
            const std::string output_dir,
            const bool inc_hetatm,
            const bool analyze_unit_cell,
            const bool calc_surface_areas,
            const bool probe_mode,
            const double r_probe1,
            const double r_probe2,
            const double grid_step,
            const int max_depth,
            const bool make_report,
            const bool make_full_map,
            const bool make_cav_maps,
            const std::unordered_map<std::string, double> rad_map,
            const std::vector<std::string> included_elem){
  if(!setProbeRadii(r_probe1, r_probe2, probe_mode)){
    _data.success = false;
    return false;
  }
  _data.atom_file_path = file_path;
  _output_folder = output_dir;
  if (_output_folder.empty() && (make_report || make_full_map || make_cav_maps)){
    Ctrl::getInstance()->displayErrorMessage(302);
    return false;
  }
  _data.inc_hetatm = inc_hetatm;
  _data.analyze_unit_cell = analyze_unit_cell;
  _data.calc_surface_areas = calc_surface_areas;
  _data.grid_step = grid_step;
  _data.max_depth = max_depth;
  _data.make_report = make_report;
  _data.make_full_map = make_full_map;
  _data.make_cav_maps = make_cav_maps;
  setRadiusMap(rad_map);
  _data.included_elements = included_elem;
  _max_atom_radius = 0;
  for (auto& elem : _data.included_elements){
    if (findRadiusOfAtom(elem) > _max_atom_radius){_max_atom_radius = findRadiusOfAtom(elem);}
  }
  return true;
}

bool Model::setProbeRadii(const double r_1, const double r_2, const bool probe_mode){
  toggleProbeMode(probe_mode);
  setProbeRad1(r_1);
  if (optionProbeMode()){
    // The following conditions is checked twice: after pressing calc button in MainFrame::OnCalc and here
    // This second check would be unnecessary when using the GUI but
    // adding it here makes the back engine Model error-proof independently from the GUI
    if (r_1 > r_2){
      Ctrl::getInstance()->displayErrorMessage(104);
      return false;
    }
    else{
      setProbeRad2(r_2);
    }
  }
  return true;
}

///////////////////////
// CALCULATION ENTRY //
///////////////////////

CalcReportBundle Model::generateData(){
  // save the date and time of calculation for output files
  _time_stamp = timeNow();
  CalcReportBundle data;
  data = generateVolumeData();
  if(Ctrl::getInstance()->getAbortFlag()){return data;}
  // surface calculation requires running the volume calculation first, but shouldn't be inside the volume calc function
  if (optionCalcSurfaceAreas() && data.success){
    data = generateSurfaceData();
  }
  return data;
}

CalcReportBundle Model::generateVolumeData(){
  prepareVolumeCalc();
  if (!_data.success){return _data;} // if there's been an error during preparation

  { // assign each voxel in grid a type
    auto start = std::chrono::steady_clock::now();
    bool cavities_exceeded = false;
    _cell.assignTypeInGrid(_atoms, _data.cavities, getProbeRad1(), getProbeRad2(), optionProbeMode(), cavities_exceeded);
    if(Ctrl::getInstance()->getAbortFlag()){
      _data.success = false;
      return _data;
    }
    if(cavities_exceeded){Ctrl::getInstance()->displayErrorMessage(201);}
    auto end = std::chrono::steady_clock::now();
    _data.addTime(std::chrono::duration<double>(end-start).count());
  }
  { // sum total volume
    auto start = std::chrono::steady_clock::now();
    _cell.sumVolume(_data.volumes, _data.cavities, _data.analyze_unit_cell);

    // sort cavities by volume from largest to smallest
    inverseSort(_data.cavities);
    auto end = std::chrono::steady_clock::now();
    _data.addTime(std::chrono::duration<double>(end-start).count());
  }
  return _data;
}

double calcMolarMass(const std::map<std::string,int>&, const std::unordered_map<std::string,double>&);
std::string generateChemicalFormula(const std::map<std::string,int>&, const std::vector<std::string>&);

void Model::prepareVolumeCalc(){
  auto start = std::chrono::steady_clock::now();
  // clear calculation times from previous runs
  _data.elapsed_seconds.clear();
  // reset the success value to avoid lingering errors from previous failed calculations
  _data.success = true;
  // clear previous results
  _data.volumes.clear();
  _data.cavities.clear();

  // process atom data for unit cell analysis if the option is ticked
  if(optionAnalyzeUnitCell()){
    if(!processUnitCell()){
      _data.success = false;
      return;
    }
  }

  // Convert raw atom coordinates into vector of Atom structures and exclude elements
  _atoms = convertAtomCoordinates(
      _data.analyze_unit_cell ? _processed_atom_coordinates : _raw_atom_coordinates,
      _data.included_elements);

  // set size of the box containing all atoms
  defineCell();

  // Generate chemical formula and calculate molar mass
  std::map<std::string, int> atom_count;
  if (_data.analyze_unit_cell){
    atom_count = atomCount(convertAtomCoordinates(_data.orth_cell, _data.included_elements));
  }
  else{
    atom_count = atomCount(_atoms);
  }
  _data.molar_mass = calcMolarMass(atom_count, _elem_weight);
  _data.chemical_formula = generateChemicalFormula(atom_count, _data.included_elements);
  
  auto end = std::chrono::steady_clock::now();
  _data.addTime(std::chrono::duration<double>(end-start).count());
}

CalcReportBundle Model::generateSurfaceData(){
  // requires volume calculation!
  auto start = std::chrono::steady_clock::now();
  Ctrl::getInstance()->updateStatus("Calculating surface areas...");
  Ctrl::getInstance()->updateProgressBar(0);

  // TODO: make this variable static
  std::vector<std::vector<char>> solid_types =
  { {0b00000011},
    {0b00000011, 0b00000101},
    {0b00001001, 0b00010001},
    {0b00001001} };

  const int total_surfaces = solid_types.size() + 2*_data.cavities.size();
  auto percentageDone = [](double num, double denom){return int(100*num/denom);};

  // full structure surfaces
  {
    std::vector<double> surfaces; // temporary
    for(size_t i = 0; i < solid_types.size(); ++i){
      // special case for this set of types, to avoid recalculation
      if (i == 2 && !optionProbeMode()){
        surfaces.push_back(surfaces[1]);
      }
      else {
        surfaces.push_back(_cell.calcSurfArea(solid_types[i]));
      }
      Ctrl::getInstance()->updateProgressBar(percentageDone(i+1,total_surfaces));
      // check abort flag
      if(Ctrl::getInstance()->getAbortFlag()){
        _data.success = false;
        return _data;
      }
    }
    // from temporary container to _data
    _data.surf_vdw = surfaces[0];
    _data.surf_molecular = surfaces[1];
    _data.surf_probe_excluded = surfaces[2];
    _data.surf_probe_accessible = surfaces[3];
  }

  // cavity surfaces
  for (size_t i = 0; i < _data.cavities.size(); ++i){
    Cavity& cav = _data.cavities[i];

    cav.surf_shell = _cell.calcSurfArea(solid_types[2], cav.id, cav.min_index, cav.max_index);
    Ctrl::getInstance()->updateProgressBar(percentageDone(solid_types.size() + i*2 + 1,total_surfaces));

    cav.surf_core = _cell.calcSurfArea(solid_types[3], cav.id, cav.min_index, cav.max_index);
    Ctrl::getInstance()->updateProgressBar(percentageDone(solid_types.size() + i*2 + 2,total_surfaces));

    if(Ctrl::getInstance()->getAbortFlag()){
      _data.success = false;
      return _data;
    }
  }

  auto end = std::chrono::steady_clock::now();
  _data.addTime(std::chrono::duration<double>(end-start).count());

  return _data;
}

std::vector<Atom> Model::convertAtomCoordinates(const RawAtomData& atom_coordinates, 
    const std::vector<std::string>& included_elements){

  std::vector<Atom> included_atom_list;
  for(size_t i = 0; i < atom_coordinates.size(); i++){
    if(isIncluded(std::get<0>(atom_coordinates[i]), included_elements)){
      Atom at = Atom(std::get<1>(atom_coordinates[i]),
                     std::get<2>(atom_coordinates[i]),
                     std::get<3>(atom_coordinates[i]),
                     std::get<0>(atom_coordinates[i]),
                     findRadiusOfAtom(std::get<0>(atom_coordinates[i])),
                     _elem_Z[std::get<0>(atom_coordinates[i])]);
      included_atom_list.push_back(at);
    }
  }
  return included_atom_list;
}

// Update GUI with the list of imported atoms. Look up the radii from the previously imported 
// elements file. Uses only standard library containers, to avoid dependency issues.
std::vector<std::tuple<std::string, int, double>> Model::generateAtomList(){
  std::vector<std::tuple<std::string, int, double>> atoms_for_list;
 
  std::map<std::string, int> atom_count = atomCount(_raw_atom_coordinates);
  for(auto elem : atom_count){
    atoms_for_list.push_back(std::make_tuple(elem.first, elem.second, findRadiusOfAtom(elem.first)));
  }
  
  return atoms_for_list;
}

std::map<std::string, int> Model::atomCount(const RawAtomData& raw_atom_data){
  std::map<std::string, int> atom_count;
  for (auto elem : raw_atom_data){
    atom_count[std::get<0>(elem)]++;
  }
  return atom_count;
}

std::map<std::string, int> Model::atomCount(const std::vector<Atom>& raw_atom_data){
  std::map<std::string, int> atom_count;
  for (auto elem : raw_atom_data){
    atom_count[elem.symbol]++;
  }
  return atom_count;
}

void Model::setRadiusMap(std::unordered_map<std::string, double> map){
  _radius_map = map;
}

std::unordered_map<std::string,double> Model::getRadiusMap(){
  return _radius_map;
}

std::string generateChemicalFormula(const std::map<std::string,int>& n_atoms, const std::vector<std::string>& included_elem){
  std::string chemical_formula = "";
  std::string prefix = "";

  // Create a new map, but ignore charges and skip excluded elements
  std::map<std::string, int> n_atoms_ignore_charges;
  for (auto elem : n_atoms){
    if (!isIncluded(elem.first, included_elem)){continue;}
    n_atoms_ignore_charges[ImportMngr::stripCharge(elem.first)] += elem.second;
  }

  // iterate through map in lexographic order
  for (auto elem : n_atoms_ignore_charges){
    std::string symbol = elem.first;
    std::string subscript = std::to_string(elem.second);
    // by convention: carbon comes first, then hydrogen, then in alphabetical order
    if (symbol == "C" || symbol == "H"){
      prefix += symbol + (subscript != "1"? subscript : "");
    }
    else {
      chemical_formula += symbol + (subscript != "1"? subscript : "");
    } 
  }
  return prefix + chemical_formula;
}

double calcMolarMass(const std::map<std::string,int>& atom_list, const std::unordered_map<std::string,double>& elem_weight){

  auto findWeightOfAtom = [elem_weight](std::string symbol){
    if (elem_weight.count(symbol)){
      return elem_weight.at(symbol);
    }

    symbol = ImportMngr::stripCharge(symbol);

    if (elem_weight.count(symbol)){
      return elem_weight.at(symbol);
    }

    return double(0);
  };

  double molar_mass = 0;
  for(auto elem : atom_list){
    molar_mass += findWeightOfAtom(elem.first) * elem.second;
  }
  return molar_mass;
}

///////////////////////////
// CALCULATION FUNCTIONS //
///////////////////////////

void Model::defineCell(){
  std::array<double, 3> unit_cell_limits = {0,0,0};
  if(optionAnalyzeUnitCell()){
    unit_cell_limits = {_cart_matrix[0][0], _cart_matrix[1][1], _cart_matrix[2][2]};
  }
  _cell = Space(_atoms, _data.grid_step, _data.max_depth, optionProbeMode()? getProbeRad2() : getProbeRad1(), optionAnalyzeUnitCell(), unit_cell_limits);
  return;
}

////////////////////////////////////////////
// CRYSTAL UNIT CELL PROCESSING FUNCTIONS //
////////////////////////////////////////////
// TODO: If possible, move eventually to crystallographer.h
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
  5) create supercell at least 3x3x3 but big enough to include a radius around central unit cell = gridstep + largest_atom radius + 2*largest probe radius
  6) create atom map based on unit cell limits + radius = gridstep + largest_atom radius + 2*largest probe radius
  7) write structure file with processed atom list
  */
  double radius_limit = _data.grid_step + _max_atom_radius + 2*( (_data.probe_mode) ? getProbeRad2() : getProbeRad1() );
  if(fileExtension(_data.atom_file_path) == "pdb" && _space_group == ""){
    Ctrl::getInstance()->displayErrorMessage(111);
    return false;
  }

  for(int i = 0; i < 6; i++){
    if(_cell_param[i] == 0){
      Ctrl::getInstance()->displayErrorMessage(112);
      return false;
    }
  }
  _processed_atom_coordinates = _raw_atom_coordinates;

  _cart_matrix = Cryst::orthogonalizeUnitCell(_cell_param);

  if(!symmetrizeUnitCell()){
    return false;
  }
  moveAtomsInsideCell();
  removeDuplicateAtoms();
  _data.orth_cell = _processed_atom_coordinates;
  
  generateSupercell(radius_limit);
  generateUsefulAtomMapFromSupercell(radius_limit);
  _data.supercell = _processed_atom_coordinates;
  return true;
}

// 1) find orthogonal unit cell matrix = cartesian coordinates of the unit cell axes
// Moved to file crystallographer.h

// 2) create symmetry elements from base structure
bool Model::symmetrizeUnitCell(){
  if(fileExtension(_data.atom_file_path) == "pdb"){
    if(!getSymmetryElements(_space_group, _sym_matrix_XYZ, _sym_matrix_fraction)){
      Ctrl::getInstance()->displayErrorMessage(113);
      return false;
    }
  }
  /* To convert cartesian coordinates x y z in unit cell coordinates a b c:
  c = z/Cz
  b = (y-(Cy*z/Cz))/By
    = (y-(Cy*c))/By
  a = (x-(Cx*z/Cz)-(Bx*(y-(Cy*z/Cz))/By))/Ax
    = (x-(Cx*c)-(Bx*b))/Ax
  */
  std::vector<std::tuple<std::string, double, double, double>> ABC_coord;
  int atom_number = _processed_atom_coordinates.size();
  int sym_number = _sym_matrix_XYZ.size()/9;
  for (int i = 0; i < atom_number; i++){
    double atom_x = std::get<1>(_processed_atom_coordinates[i]);
    double atom_y = std::get<2>(_processed_atom_coordinates[i]);
    double atom_z = std::get<3>(_processed_atom_coordinates[i]);
    double atom_abc[3];
    atom_abc[2] = atom_z/_cart_matrix[2][2];
    atom_abc[1] = (atom_y-(_cart_matrix[2][1]*atom_abc[2]))/_cart_matrix[1][1];
    atom_abc[0] = (atom_x-(_cart_matrix[2][0]*atom_abc[2])-(_cart_matrix[1][0]*atom_abc[1]))/_cart_matrix[0][0];
    for (int j = 0; j < sym_number; j++){
      double sym_abc[3];
      for (int k = 0; k < 3; k++){
        sym_abc[k] = _sym_matrix_fraction[3*j+k];
        for (int n = 0; n < 3; n++){
          sym_abc[k] += _sym_matrix_XYZ[9*j+3*k+n]*atom_abc[n];
        }
      }
      ABC_coord.emplace_back(std::get<0>(_processed_atom_coordinates[i]), sym_abc[0], sym_abc[1], sym_abc[2]);
    }
  }
  for (size_t i = 0; i < ABC_coord.size(); i++){ // converts new list of atoms after applying symmetry to cartesian coordinates
    double atom_xyz[3] = {0,0,0};
    double atom_abc[3] = {std::get<1>(ABC_coord[i]), std::get<2>(ABC_coord[i]), std::get<3>(ABC_coord[i])};
    for (int j = 0; j < 3; j++){
      for (int k = 0; k < 3; k++){
        atom_xyz[j] += atom_abc[k]*_cart_matrix[k][j];
      }
    }
    _processed_atom_coordinates.emplace_back(std::get<0>(ABC_coord[i]), atom_xyz[0], atom_xyz[1], atom_xyz[2]);
  }
  return true;
}

// 3) if atoms outside the orthogonal cell, move them inside
void Model::moveAtomsInsideCell(){
  for(size_t n = 0; n < _processed_atom_coordinates.size(); n ++){
    while(std::get<3>(_processed_atom_coordinates[n]) < 0){ // need to start with Z axis because the unit cell C axis can have X and Y components
      std::get<3>(_processed_atom_coordinates[n]) += _cart_matrix[2][2];
      std::get<2>(_processed_atom_coordinates[n]) += _cart_matrix[2][1];
      std::get<1>(_processed_atom_coordinates[n]) += _cart_matrix[2][0];
    }
    while(std::get<3>(_processed_atom_coordinates[n]) > _cart_matrix[2][2]){
      std::get<3>(_processed_atom_coordinates[n]) -= _cart_matrix[2][2];
      std::get<2>(_processed_atom_coordinates[n]) -= _cart_matrix[2][1];
      std::get<1>(_processed_atom_coordinates[n]) -= _cart_matrix[2][0];
    }
    while(std::get<2>(_processed_atom_coordinates[n]) < 0){ // then check Y axis because the unit cell B axis can have a X component
      std::get<2>(_processed_atom_coordinates[n]) += _cart_matrix[1][1];
      std::get<1>(_processed_atom_coordinates[n]) += _cart_matrix[1][0];
    }
    while(std::get<2>(_processed_atom_coordinates[n]) > _cart_matrix[1][1]){
      std::get<2>(_processed_atom_coordinates[n]) -= _cart_matrix[1][1];
      std::get<1>(_processed_atom_coordinates[n]) -= _cart_matrix[1][0];
    }
    while(std::get<1>(_processed_atom_coordinates[n]) < 0){
      std::get<1>(_processed_atom_coordinates[n]) += _cart_matrix[0][0];
    }
    while(std::get<1>(_processed_atom_coordinates[n]) > _cart_matrix[0][0]){
      std::get<1>(_processed_atom_coordinates[n]) -= _cart_matrix[0][0];
    }
  }
}

// 4) remove duplicate atoms (allow 0.01-0.05 A error)
void Model::removeDuplicateAtoms(){
  for(size_t i = 0; i < _processed_atom_coordinates.size(); i++){
    for(size_t j = i+1; j < _processed_atom_coordinates.size(); j++){
      if(std::get<0>(_processed_atom_coordinates[i]) == std::get<0>(_processed_atom_coordinates[j]) &&
         std::abs(std::get<1>(_processed_atom_coordinates[i])-std::get<1>(_processed_atom_coordinates[j])) < 0.05 &&
         std::abs(std::get<2>(_processed_atom_coordinates[i])-std::get<2>(_processed_atom_coordinates[j])) < 0.05 &&
         std::abs(std::get<3>(_processed_atom_coordinates[i])-std::get<3>(_processed_atom_coordinates[j])) < 0.05
         ){
           _processed_atom_coordinates.erase(_processed_atom_coordinates.begin() + j);
           j--;
      }
    }
  }
}

// 5) create supercell at least 3x3x3 but big enough to include a radius around central unit cell = gridstep + largest_atom radius + 2*largest probe radius
void Model::generateSupercell(double radius_limit){
  int initial_number_of_atoms = _processed_atom_coordinates.size();
  if(_cell_param[3] == 90 && _cell_param[4] == 90 && _cell_param[5] == 90){ // for orthogonal space groups, the algorithm is considerably simpler than for other space groups
    // determine how many cells will be needed in each dimension for the supercell
    int surrounding_cells[3];
    for(int i = 0; i < 3; i++){
      surrounding_cells[i] = 1 + (int)(radius_limit/_cart_matrix[i][i]);
    }
    for(int i = -surrounding_cells[0]; i <= surrounding_cells[0]; i++){
      for(int j = -surrounding_cells[1]; j <= surrounding_cells[1]; j++){
        for(int k = -surrounding_cells[2]; k <= surrounding_cells[2]; k++){
          for(int n = 0; n < initial_number_of_atoms; n++){
            // duplicate atoms in each cell of supercell beside the original central cell
            if(i != 0 || j != 0 || k != 0){
              _processed_atom_coordinates.emplace_back(std::get<0>(_processed_atom_coordinates[n]),
                                            std::get<1>(_processed_atom_coordinates[n])+(i*_cart_matrix[0][0]),
                                            std::get<2>(_processed_atom_coordinates[n])+(j*_cart_matrix[1][1]),
                                            std::get<3>(_processed_atom_coordinates[n])+(k*_cart_matrix[2][2]));
            }
          }
        }
      }
    }
  }
  else { // for non orthogonal space groups, we need to extend the supercell in a specific order because B and C axes can have components in X and/or Y
    // determine how many cells will be needed in each dimension for the supercell
    int C_number = 1 + (int)(radius_limit/_cart_matrix[2][2]);
    for(int i = -C_number; i <= C_number; i++){
      int B_number_neg = -1 + (int)((-radius_limit-(i*_cart_matrix[2][1]))/_cart_matrix[1][1]);
      int B_number_pos = 1 + (int)((radius_limit-(i*_cart_matrix[2][1]))/_cart_matrix[1][1]);
      for(int j = B_number_neg; j <= B_number_pos; j++){
        int A_number_neg = -1 + (int)((-radius_limit-(i*_cart_matrix[2][0])-(j*_cart_matrix[1][0]))/_cart_matrix[0][0]);
        int A_number_pos = 1+ (int)((radius_limit-(i*_cart_matrix[2][0])-(j*_cart_matrix[1][0]))/_cart_matrix[0][0]);
        for(int k = A_number_neg; k <= A_number_pos; k++){
          for(int n = 0; n < initial_number_of_atoms; n++){
            // duplicate atoms in each cell of supercell beside the original central cell
            if(i != 0 || j != 0 || k != 0){
                _processed_atom_coordinates.emplace_back(std::get<0>(_processed_atom_coordinates[n]),
                                              std::get<1>(_processed_atom_coordinates[n])+(k*_cart_matrix[0][0])+(j*_cart_matrix[1][0])+(i*_cart_matrix[2][0]),
                                              std::get<2>(_processed_atom_coordinates[n])+(j*_cart_matrix[1][1])+(i*_cart_matrix[2][1]),
                                              std::get<3>(_processed_atom_coordinates[n])+(i*_cart_matrix[2][2]));
            }
          }
        }
      }
    }
  }
}

// 6) create atom map based on cell limits + radius= gridstep + largest_atom radius + 2*largest probe radius
void Model::generateUsefulAtomMapFromSupercell(double radius_limit){
  // for large crystal structures, it is much faster to create a new temporary vector
  // than to erase elements in the original vector
  std::vector<std::tuple<std::string, double, double, double>> limited_atom_list;
  for(size_t i = 0; i < _processed_atom_coordinates.size(); i++){
    if(std::get<1>(_processed_atom_coordinates[i]) >= -radius_limit &&
       std::get<2>(_processed_atom_coordinates[i]) >= -radius_limit &&
       std::get<3>(_processed_atom_coordinates[i]) >= -radius_limit &&
       std::get<1>(_processed_atom_coordinates[i]) <= _cart_matrix[0][0]+radius_limit &&
       std::get<2>(_processed_atom_coordinates[i]) <= _cart_matrix[1][1]+radius_limit &&
       std::get<3>(_processed_atom_coordinates[i]) <= _cart_matrix[2][2]+radius_limit
    ){
      limited_atom_list.emplace_back(_processed_atom_coordinates[i]);
    }
  }
  _processed_atom_coordinates = limited_atom_list;
}

