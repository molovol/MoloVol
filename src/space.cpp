#include "space.h"
#include "atom.h"
#include "voxel.h"
#include "atomtree.h"
#include "misc.h"
#include "exception.h"
#include <cmath>
#include <cassert>
#include <stdexcept>

/////////////////
// CONSTRUCTOR //
/////////////////

Space::Space(std::vector<Atom> &atoms, const double bot_lvl_vxl_dist, const int depth, const double r_probe)
  :grid_size(bot_lvl_vxl_dist), max_depth(depth){
  setBoundaries(atoms,r_probe+2*bot_lvl_vxl_dist);
  initGrid();
}

///////////////////////////////
// FUNCTIONS FOR CONSTRUCTOR //
///////////////////////////////

// member function loops through all atoms (previously extracted
// from file) and saves the minimum and maximum positions in all
// three cartesian directions out of all atoms. Also finds max
// radius of all atoms and sets the space boundaries slightly so
// that all atoms fit the space.
void Space::setBoundaries(const std::vector<Atom> &atoms, const double add_space){
  double max_radius = 0;
  for(size_t at = 0; at < atoms.size(); at++){
    std::array<double,3> atom_pos = atoms[at].getPos();

    // if we are at the first atom, then the atom position is min/max by default
    if(at == 0){
      cart_min = atom_pos;
      cart_max = atom_pos;
      max_radius = atoms[at].rad;
    }

    // after the first atom, begin comparing the atom positions to min/max and save if exceeds previously saved values
    else{
      for(int dim = 0; dim < 3; dim++){
        if(atom_pos[dim] > cart_max[dim]){
          cart_max[dim] = atom_pos[dim];
        }
        if(atom_pos[dim] < cart_min[dim]){
          cart_min[dim] = atom_pos[dim];
        }
        if(atoms[at].rad > max_radius){
          max_radius = atoms[at].rad;
        }
      }
    }
  }
  // expand boundaries by a little more than the largest atom found
  for(int dim = 0; dim < 3; dim++){
    cart_min[dim] -= (add_space + max_radius);
    cart_max[dim] += (add_space + max_radius);
  }
  // align the grid with origin (0,0,0) of atomic Cartesian coordinates, useful for unit cell analysis
  for(int dim = 0; dim < 3; dim++){
    if(std::fmod(cart_min[dim],grid_size) != 0){
      cart_min[dim] -= std::fmod(cart_min[dim],grid_size);
    }
  }
  return;
}

// based on the grid step and the octree max_depth, this function produces a
// 3D grid (in form of a 1D vector) that contains all top level voxels.
void Space::initGrid(){
  _grid.clear();
  // determine how many top lvl voxels in each direction are needed
  for (int dim = 0; dim < 3; dim++){
    n_gridsteps[dim] = std::ceil (std::ceil( (getSize())[dim] / grid_size ) / std::pow(2,max_depth) );
  }
  // initialise 3d tensors for each octree level
  for (int lvl = 0; lvl <= max_depth; ++lvl){
    _grid.push_back(Container3D<Voxel>( n_gridsteps[0]*pow(2,max_depth-lvl),
                                        n_gridsteps[1]*pow(2,max_depth-lvl),
                                        n_gridsteps[2]*pow(2,max_depth-lvl)));
  }
}

/////////////////////
// TYPE ASSIGNMENT //
/////////////////////

// sets all voxel's types, determined by the input atoms
void Space::assignTypeInGrid(const AtomTree& atomtree, const double r_probe1, const double r_probe2, bool probe_mode, bool &error_cav){
  // save variable that all voxels need access to for their type determination as static members of Voxel class
  Voxel::prepareTypeAssignment(this, atomtree);
  printf("\nVoxels assignment progress:\n");
  if (probe_mode){
    // first run algorithm with the larger probe to exclude most voxels - "masking mode"
    Voxel::storeProbe(r_probe2, true);
    printf("\nAssigning probe 2 core and atoms:\n");
    assignAtomVsCore();

    printf("\nAssigning probe 2 shell:\n");
    assignShellVsVoid();
    printf("\nAssigning probe 1 core:\n");
  }
  else{
    printf("\nAssigning probe 1 core and atoms:\n");
  }
  Voxel::storeProbe(r_probe1, false);
  assignAtomVsCore();

  printf("\nIdentifying cavities:\n");
  try{identifyCavities();}
  catch (const std::overflow_error& e){
    error_cav = true; // TODO: consider making a bool function if no other error could arise and only this one should be reported
  }

  printf("\nAssigning probe 1 shell and excluded void:\n");
  assignShellVsVoid();
}

void Space::assignAtomVsCore(){
  // calculate position of first voxel
  const std::array<double,3> vxl_origin = getOrigin();
  // calculate side length of top level voxel
  const double vxl_dist = grid_size * pow(2,max_depth);
  std::array<double,3> vxl_pos;
  std::array<unsigned,3> top_lvl_index;
  for(top_lvl_index[0] = 0; top_lvl_index[0] < n_gridsteps[0]; top_lvl_index[0]++){
    vxl_pos[0] = vxl_origin[0] + vxl_dist * (0.5 + top_lvl_index[0]);
    for(top_lvl_index[1] = 0; top_lvl_index[1] < n_gridsteps[1]; top_lvl_index[1]++){
      vxl_pos[1] = vxl_origin[1] + vxl_dist * (0.5 + top_lvl_index[1]);
      for(top_lvl_index[2] = 0; top_lvl_index[2] < n_gridsteps[2]; top_lvl_index[2]++){
        vxl_pos[2] = vxl_origin[2] + vxl_dist * (0.5 + top_lvl_index[2]);
        // voxel position is deliberately not stored in voxel object to reduce memory cost
        getTopVxl(top_lvl_index).evalRelationToAtoms(top_lvl_index, vxl_pos, max_depth);
      }
    }
    printf("%i%% done\n", int(100*(double(top_lvl_index[0])+1)/double(n_gridsteps[0])));
  }
}

void Space::identifyCavities(){
  std::array<unsigned int,3> vxl_index;
  unsigned char id = 1;
  for(vxl_index[0] = 0; vxl_index[0] < n_gridsteps[0]; vxl_index[0]++){
    for(vxl_index[1] = 0; vxl_index[1] < n_gridsteps[1]; vxl_index[1]++){
      for(vxl_index[2] = 0; vxl_index[2] < n_gridsteps[2]; vxl_index[2]++){
        try{
          descendToCore(id,vxl_index,getMaxDepth()); // id gets iterated inside this function
        }
        catch (const std::overflow_error& e){
          throw;
        }
      }
    }
    printf("%i%% done\n", int(100*(double(vxl_index[0])+1)/double(n_gridsteps[0])));
  }
}

void Space::descendToCore(unsigned char& id, const std::array<unsigned,3> index, int lvl){
  Voxel& vxl = getVxlFromGrid(index,lvl);
  if (!vxl.isCore()){return;}
  if (!vxl.hasSubvoxel()){
    if(vxl.floodFill(id, index, lvl)){
      if (id == 0b11111111){
        throw std::overflow_error("Too many isolated cavities detected!");
      }
      id++;
    }
  }
  else {
    std::array<unsigned,3> subindex;
    for (char i = 0; i < 2; ++i){
      subindex[0] = index[0]*2 + i;
      for (char j = 0; j < 2; ++j){
        subindex[1] = index[1]*2 + j;
        for (char k = 0; k < 2; ++k){
          subindex[2] = index[2]*2 + k;
          try {descendToCore(id, subindex, lvl-1);}
          catch (const std::overflow_error& e){throw;}
        }
      }
    }
  }
}

void Space::assignShellVsVoid(){
  std::array<unsigned int,3> vxl_index;
  for(vxl_index[0] = 0; vxl_index[0] < n_gridsteps[0]; vxl_index[0]++){
    for(vxl_index[1] = 0; vxl_index[1] < n_gridsteps[1]; vxl_index[1]++){
      for(vxl_index[2] = 0; vxl_index[2] < n_gridsteps[2]; vxl_index[2]++){
        getTopVxl(vxl_index).evalRelationToVoxels(vxl_index, max_depth);
      }
    }
    printf("%i%% done\n", int(100*(double(vxl_index[0])+1)/double(n_gridsteps[0])));
  }
}

void Space::getVolume(std::map<char,double>& volumes,
                      std::vector<double>& cavities_core,
                      std::vector<double>& cavities_shell,
                      std::vector<std::array<double,3>>& cav_min,
                      std::vector<std::array<double,3>>& cav_max,
                      std::vector<std::array<size_t,3>>& cav_min_index,
                      std::vector<std::array<size_t,3>>& cav_max_index){
  // clear all output variables
  volumes.clear();
  cavities_core.clear();
  cavities_shell.clear();
  cav_min.clear();
  cav_max.clear();
  cav_min_index.clear();
  cav_max_index.clear();
  // create maps used for tallying voxels
  std::map<char, unsigned> type_tally;
  std::map<unsigned char, unsigned> id_core_tally;
  std::map<unsigned char, unsigned> id_shell_tally;
  // contain the boundaries in which all voxels of a given ID are contained
  std::map<unsigned char, std::array<unsigned,3>> id_min;
  std::map<unsigned char, std::array<unsigned,3>> id_max;
  // set id 0 in maps for convenience when converting to vector
  id_core_tally[0] = 0;
  id_shell_tally[0] = 0;
  for(char i = 0; i < 3; i++){
    id_min[0][i] = 0;
    id_max[0][i] = 0;
  }

  // go through all top level voxels and search recursively for pure types. once found, each bottom level voxel
  // adds 1 to the tally. at the same time the boundaries of the cavities are determined
  std::array<unsigned,3> top_lvl_index;
  for (top_lvl_index[0] = 0; top_lvl_index[0] < n_gridsteps[0]; top_lvl_index[0]++){
    for (top_lvl_index[1] = 0; top_lvl_index[1] < n_gridsteps[1]; top_lvl_index[1]++){
      for (top_lvl_index[2] = 0; top_lvl_index[2] < n_gridsteps[2]; top_lvl_index[2]++){
        getTopVxl(top_lvl_index).tallyVoxelsOfType(type_tally, id_core_tally, id_shell_tally, id_min, id_max, top_lvl_index, max_depth);
      }
    }
  }

  // calculate the volume of a single bottom level voxel
  double unit_volume = pow(getVxlSize(),3);
  // convert from units of bottom level voxels to units of volume
  for (auto& [type,tally] : type_tally) {
    volumes[type] = tally * unit_volume;
  }
  // allocate memory
  cavities_core = std::vector<double> (id_core_tally.size());
  cavities_shell = std::vector<double> (id_shell_tally.size());
  cav_min = std::vector<std::array<double,3>> (id_core_tally.size());
  cav_max = std::vector<std::array<double,3>> (id_core_tally.size());
  cav_min_index = std::vector<std::array<size_t,3>> (id_core_tally.size());
  cav_max_index = std::vector<std::array<size_t,3>> (id_core_tally.size());
  // convert from units of bottom level voxels to units of volume
  // convert from index to spatial coordinates
  // copy values from map to vector for more efficient storage and access
  for (auto& [id,tally] : id_core_tally) {
    cavities_core[id] = tally * unit_volume;
    cavities_shell[id] = id_shell_tally[id] * unit_volume;
    for (char i = 0; i < 3; ++i){
      cav_min[id][i] = getOrigin()[i] + getVxlSize()*id_min[id][i];
      cav_max[id][i] = getOrigin()[i] + getVxlSize()*(id_max[id][i]+1);
      cav_min_index[id][i] = id_min[id][i];
      cav_max_index[id][i] = id_max[id][i];
    }
  }
}

void Space::getUnitCellVolume(std::map<char,double>& volumes,
                              std::vector<double>& cavities_core,
                              std::vector<double>& cavities_shell,
                              std::vector<std::array<double,3>>& cav_min,
                              std::vector<std::array<double,3>>& cav_max,
                              std::vector<std::array<size_t,3>>& cav_min_index,
                              std::vector<std::array<size_t,3>>& cav_max_index,
                              std::array<double,3> unit_cell_limits){
  // clear all output variables
  volumes.clear();
  cavities_core.clear();
  cavities_shell.clear();
  cav_min.clear();
  cav_max.clear();
  cav_min_index.clear();
  cav_max_index.clear();
  // create maps used for tallying voxels
  std::map<char, double> type_tally;
  std::map<unsigned char, double> id_core_tally;
  std::map<unsigned char, double> id_shell_tally;
  // contain the boundaries in which all voxels of a given ID are contained
  std::map<unsigned char, std::array<unsigned,3>> id_min;
  std::map<unsigned char, std::array<unsigned,3>> id_max;

  std::vector<char> types_to_tally{0b00000011,0b00000101,0b00001001,0b00010001,0b00100001,0b01000001};

  std::array<unsigned int,3> unit_cell_start_index;
  std::array<unsigned int,3> unit_cell_end_index;
  std::array<double,3> unit_cell_mod_index;
  for(int i = 0; i < 3; i++){
    // +0.5 to avoid rounding errors
    unit_cell_start_index[i] = int(0.5 - cart_min[i]/grid_size);
    // no rounding because fmod() in unit_cell_mod_index will correct any rounding error in unit_cell_end_index
    unit_cell_end_index[i] = unit_cell_start_index[i] + unit_cell_limits[i]/grid_size;
    unit_cell_mod_index[i] = std::fmod(unit_cell_limits[i],grid_size);
  }
  std::array<unsigned int,3> bot_lvl_index;
  for (bot_lvl_index[0] = unit_cell_start_index[0]; bot_lvl_index[0] < unit_cell_end_index[0]; bot_lvl_index[0]++){
    for (bot_lvl_index[1] = unit_cell_start_index[1]; bot_lvl_index[1] < unit_cell_end_index[1]; bot_lvl_index[1]++){
      for (bot_lvl_index[2] = unit_cell_start_index[2]; bot_lvl_index[2] < unit_cell_end_index[2]; bot_lvl_index[2]++){
        tallyVoxelsUnitCell(bot_lvl_index, 1, type_tally, id_core_tally, id_shell_tally, id_min, id_max);
      }
    }
  }
  // for unit cells that are not proportional to grid_size, it is necessary to only consider a partial voxel volume for the voxels partially overlapping with the unit cell
  for(int n = 0; n < 3; n++){
    int i = n;
    int j = (n+1)%3;
    int k = (n+2)%3;

    // count voxels in the three end faces of the unit cell
    bot_lvl_index[i] = unit_cell_end_index[i];
    for (bot_lvl_index[j] = unit_cell_start_index[j]; bot_lvl_index[j] < unit_cell_end_index[j]; bot_lvl_index[j]++){
      for (bot_lvl_index[k] = unit_cell_start_index[k]; bot_lvl_index[k] < unit_cell_end_index[k]; bot_lvl_index[k]++){
        double voxel_fraction = unit_cell_mod_index[i]/grid_size;
        tallyVoxelsUnitCell(bot_lvl_index, voxel_fraction, type_tally, id_core_tally, id_shell_tally, id_min, id_max);
      }
    }

    // count voxels in the three edges connecting end faces of the unit cell
    bot_lvl_index[j] = unit_cell_end_index[j];
    for (bot_lvl_index[k] = unit_cell_start_index[k]; bot_lvl_index[k] < unit_cell_end_index[k]; bot_lvl_index[k]++){
      double voxel_fraction = unit_cell_mod_index[i]*unit_cell_mod_index[j]/(grid_size*grid_size);
      tallyVoxelsUnitCell(bot_lvl_index, voxel_fraction, type_tally, id_core_tally, id_shell_tally, id_min, id_max);
    }
  }

  // add last end corner voxel
  for(int i = 0; i < 3; i++){
    bot_lvl_index[i] = unit_cell_end_index[i];
  }
  double voxel_fraction = unit_cell_mod_index[0]*unit_cell_mod_index[1]*unit_cell_mod_index[2]/(grid_size*grid_size*grid_size);
  tallyVoxelsUnitCell(bot_lvl_index, voxel_fraction, type_tally, id_core_tally, id_shell_tally, id_min, id_max);

  // calculate the volume of a single bottom level voxel
  double unit_volume = pow(grid_size,3);

  for (size_t i = 0; i < types_to_tally.size(); i++){
    volumes[types_to_tally[i]] = type_tally[types_to_tally[i]] * unit_volume;
  }
  // start vectors with 0 volume element for compatibility with non unit cell mode that has 0 volume for id 0
  std::array<double,3> min_arr = {0,0,0};
  std::array<double,3> max_arr = {0,0,0};
  std::array<size_t,3> min_index_arr = {0,0,0};
  std::array<size_t,3> max_index_arr = {0,0,0};
  cavities_core.emplace_back(0);
  cavities_shell.emplace_back(0);
  cav_min.emplace_back(min_arr);
  cav_max.emplace_back(max_arr);
  cav_min_index.emplace_back(min_index_arr);
  cav_max_index.emplace_back(max_index_arr);
  // convert from units of bottom level voxels to units of volume
  // convert from index to spatial coordinates
  // copy values from map to vector for more efficient storage and access
  for (auto& [id,tally] : id_core_tally) {
    cavities_core.emplace_back(tally * unit_volume);
    cavities_shell.emplace_back(id_shell_tally[id] * unit_volume);
    for (char i = 0; i < 3; ++i){
      min_arr[i] = getOrigin()[i] + getVxlSize()*id_min[id][i];
      max_arr[i] = getOrigin()[i] + getVxlSize()*(id_max[id][i]+1);
      min_index_arr[i] = id_min[id][i];
      max_index_arr[i] = id_max[id][i];
    }
    cav_min.emplace_back(min_arr);
    cav_max.emplace_back(max_arr);
    cav_min_index.emplace_back(min_index_arr);
    cav_max_index.emplace_back(max_index_arr);
  }
}

void Space::tallyVoxelsUnitCell(std::array<unsigned int,3> bot_lvl_index,
                                double voxel_fraction,
                                std::map<char, double>& type_tally,
                                std::map<unsigned char, double>& id_core_tally,
                                std::map<unsigned char, double>& id_shell_tally,
                                std::map<unsigned char, std::array<unsigned,3>>& id_min,
                                std::map<unsigned char, std::array<unsigned,3>>& id_max){
  // add voxel by type
  type_tally[getVxlFromGrid(bot_lvl_index, 0).getType()] += voxel_fraction;
  // add voxel by id and type
  if(getVxlFromGrid(bot_lvl_index, 0).getType() == 0b00001001){
    id_core_tally[getVxlFromGrid(bot_lvl_index, 0).getID()] += voxel_fraction;
  }
  else if(getVxlFromGrid(bot_lvl_index, 0).getType() == 0b00010001){
    id_shell_tally[getVxlFromGrid(bot_lvl_index, 0).getID()] += voxel_fraction;
  }
  // set cavity min/max indexes
  if (id_min.count(getVxlFromGrid(bot_lvl_index, 0).getID()) == 0){
    id_min[getVxlFromGrid(bot_lvl_index, 0).getID()] = bot_lvl_index;
  }
  if (id_max.count(getVxlFromGrid(bot_lvl_index, 0).getID()) == 0){
    id_max[getVxlFromGrid(bot_lvl_index, 0).getID()] = bot_lvl_index;
  }
  for (char i = 0; i < 3; i++){
    if (id_min[getVxlFromGrid(bot_lvl_index, 0).getID()][i] > bot_lvl_index[i]){
      id_min[getVxlFromGrid(bot_lvl_index, 0).getID()][i] = bot_lvl_index[i];
    }
    if (id_max[getVxlFromGrid(bot_lvl_index, 0).getID()][i] < bot_lvl_index[i]){
      id_max[getVxlFromGrid(bot_lvl_index, 0).getID()][i] = bot_lvl_index[i];
    }
  }
}

//////////////////////
// ACCESS FUNCTIONS //
//////////////////////

std::array<double,3> Space::getMin(){
  return cart_min;
}

std::array<double,3> Space::getOrigin(){
  return getMin();
}

std::array<double,3> Space::getMax(){
  return cart_max;
}

std::array<double,3> Space::getSize(){
  std::array<double,3> size;
  for(int dim = 0; dim < 3; dim++){
    size[dim] = cart_max[dim] - cart_min[dim];
  }
  return size;
}

double Space::getVxlSize() const {
  return grid_size;
}

Container3D<Voxel>& Space::getGrid(const unsigned lvl){
  return _grid[lvl];
}

/////////////////
// GET ELEMENT //
/////////////////

Voxel& Space::getVxlFromGrid(const unsigned int i, unsigned lvl){
  return _grid[lvl].getElement(i);
}

Voxel& Space::getVxlFromGrid(const unsigned int x, const unsigned int y, const unsigned int z, unsigned lvl){
  return _grid[lvl].getElement(x,y,z);
}

Voxel& Space::getVxlFromGrid(const std::array<unsigned int,3> arr, unsigned lvl){
  return _grid[lvl].getElement(arr);
}

Voxel& Space::getVxlFromGrid(const std::array<int,3> arr, unsigned lvl){
  return _grid[lvl].getElement(arr);
}

Voxel& Space::getTopVxl(const unsigned int i){
  return getVxlFromGrid(i, max_depth);
}

Voxel& Space::getTopVxl(const unsigned int x, const unsigned int y, const unsigned int z){
  return getVxlFromGrid(x, y, z, max_depth);
}

Voxel& Space::getTopVxl(const std::array<unsigned int,3> arr){
  return getVxlFromGrid(arr, max_depth);
}

Voxel& Space::getTopVxl(const std::array<int,3> arr){
  return getVxlFromGrid(arr, max_depth);
}

/////////////////

// check whether coord is inside grid bounds
bool Space::isInBounds(const std::array<int,3>& coord, const unsigned lvl){
  for (char i = 0; i < 3; i++){
    if(coord[i] < 0 || coord[i] >= n_gridsteps[i] * std::pow(2,max_depth-lvl)){return false;}
  }
  return true;
}
bool Space::isInBounds(const std::array<unsigned,3>& coord, const unsigned lvl){
  for (char i = 0; i < 3; i++){
    if(coord[i] < 0 || coord[i] >= n_gridsteps[i] * std::pow(2,max_depth-lvl)){return false;}
  }
  return true;
}


std::array<unsigned int,3> Space::getGridsteps(){
  return n_gridsteps;
}

unsigned long int Space::totalVxlOnLvl(const int lvl) const{
  unsigned long int total = 1;
  const std::array<unsigned long int,3> gridsteps = gridstepsOnLvl(lvl);
  for (char i = 0; i < 3; i++){
    total *= gridsteps[i];
  }
  return total;
}

const std::array<unsigned long int,3> Space::gridstepsOnLvl(const int level) const {
  if (level > max_depth){throw ExceptIllegalFunctionCall();}
  std::array<unsigned long int,3> n_voxels;
  for (char i = 0; i < 3; i++){
    n_voxels[i] = n_gridsteps[i] * pow(2,max_depth-level);
  }
  return n_voxels;
}

////////////////
// PRINT GRID //
////////////////

std::array<unsigned int,3> makeIndices(std::array<unsigned int,3> indices, int depth){
  for (char i = 0; i < 3; i++){
    indices[i] *= pow(2,depth);
  }
  return indices;
}

// displays voxel grid types as matrix in the terminal. useful for debugging
void Space::printGrid(){

  bool disp_id = false;
  int depth = 0;
  std::array<unsigned int,3> indices = makeIndices(n_gridsteps, depth);

  unsigned int x_min = 0;
  unsigned int y_min = 0;

  unsigned int x_max = ((indices[0] >= 50)? 50: indices[0]);
  unsigned int y_max = ((indices[1] >= 25)? 25: indices[1]);

  unsigned int z = 0;
  std::cout << "Enter 'q' to quit; 'w', 'a', 's', 'd' for directional input; 'c' to continue in z direction; 'r' to go back in z direction. Enter '+' or '-' to change the octree depth." << std::endl;
  char usr_inp = '\0';
  while (usr_inp != 'q'){

    if (usr_inp == '#'){
      disp_id = !disp_id;
    }

    // if depth is changed, reset view
    if (usr_inp == '+' || usr_inp == '-'){
      if (usr_inp == '+' && depth < max_depth){depth++;}
      else if (usr_inp == '-' && depth > 0){depth--;}
      indices = makeIndices(n_gridsteps,depth);
      x_min = 0;
      y_min = 0;
      x_max = ((indices[0] >= 50)? 50: indices[0]);
      y_max = ((indices[1] >= 25)? 25: indices[1]);
      z = 0;
    }

    // x and y coordinates
    if (usr_inp == 'a'){x_min--; x_max--;}
    else if (usr_inp == 'd'){x_min++; x_max++;}
    else if (usr_inp == 's'){y_min++; y_max++;}
    else if (usr_inp == 'w'){y_min--; y_max--;}

    if (x_min < 0){x_min++; x_max++;}
    if (y_min < 0){y_min++; y_max++;}
    if (x_max > indices[0]){x_min--; x_max--;}
    if (y_max > indices[1]){y_min--; y_max--;}

    // z coordinate
    if (usr_inp == 'r'){
      if (z==0){
        std::cout << "z position at min. " << std::endl;
      }
      else{z--;}
    }
    else if (usr_inp == 'c'){
      z++;
      if (z >= indices[2]){
        z--;
        std::cout << "z position at max. " << std::endl;
      }
    }

    // print matrix
    for(unsigned int y = y_min; y < y_max; y++){
      for(unsigned int x = x_min; x < x_max; x++){
        char to_print;
        if (disp_id){
          to_print = ('0' + getVxlFromGrid(x,y,z, max_depth-depth).getID());
        }
        else{
          to_print = (getVxlFromGrid(x,y,z,max_depth-depth).getType() == 0b00000011)? 'A' : 'O';
          if (!readBit(getVxlFromGrid(x,y,z,max_depth-depth).getType(),0)){to_print = '?';}
          if (readBit(getVxlFromGrid(x,y,z,max_depth-depth).getType(),7)){to_print = 'M';}
          if (getVxlFromGrid(x,y,z,max_depth-depth).getType() == 0b00000101){to_print = 'X';}
          if (getVxlFromGrid(x,y,z,max_depth-depth).getType() == 0b00001001){to_print = 'P';}
          if (getVxlFromGrid(x,y,z,max_depth-depth).getType() == 0b00010001){to_print = 'S';}
          if (getVxlFromGrid(x,y,z,max_depth-depth).getType() == 0b00100001){to_print = 'p';}
          if (getVxlFromGrid(x,y,z,max_depth-depth).getType() == 0b01000001){to_print = 's';}
        }

        std::cout << to_print << " ";
      }
      std::cout << std::endl;
    }

    // get user input
    std::cout << std::endl;
    std::cout << "INPUT: ";
    std::cin >> usr_inp;
  }
}

