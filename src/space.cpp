#include "space.h"
#include "atom.h"
#include "voxel.h"
#include "atomtree.h"
#include "misc.h"
#include "exception.h"
#include <cmath>
#include <cassert>

/////////////////
// CONSTRUCTOR //
/////////////////

Space::Space(std::vector<Atom> &atoms, const double bot_lvl_vxl_dist, const int depth, const double r_probe1)
  :grid_size(bot_lvl_vxl_dist), max_depth(depth){
  setBoundaries(atoms,r_probe1+2*bot_lvl_vxl_dist);
  setGrid();
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
  for(int at = 0; at < atoms.size(); at++){
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
  return;
}

// based on the grid step and the octree max_depth, this function produces a
// 3D grid (in form of a 1D vector) that contains all top level voxels.
void Space::setGrid(){
  unsigned int n_voxels = 1;
  
  for(int dim = 0; dim < 3; dim++){
    n_gridsteps[dim] = std::ceil (std::ceil( (getSize())[dim] / grid_size ) / std::pow(2,max_depth) );
    n_voxels *= n_gridsteps[dim];
  }
  
  for(unsigned int i = 0; i < n_voxels; i++){
    grid.push_back(Voxel());
  }
  
  return;
}

/////////////////
// VOLUME COMP //
/////////////////

// sets all voxel's types, determined by the input atoms
void Space::placeAtomsInGrid(const AtomTree& atomtree, const double& r_probe){
  // calculate position of first voxel
  const std::array<double,3> vxl_origin = getOrigin();
  
  // calculate side length of top level voxel
  const double vxl_dist = grid_size * pow(2,max_depth);
  
  // save variable that all voxels need access to for their type determination as static members of Voxel class
  Voxel::storeUniversal(this, atomtree, grid_size, r_probe, max_depth);

  // TODO: wrap this in function
  std::array<double,3> vxl_pos;
  for(unsigned int x = 0; x < n_gridsteps[0]; x++){
    vxl_pos[0] = vxl_origin[0] + vxl_dist * (0.5 + x);
    for(unsigned int y = 0; y < n_gridsteps[1]; y++){
      vxl_pos[1] = vxl_origin[1] + vxl_dist * (0.5 + y);
      for(unsigned int z = 0; z < n_gridsteps[2]; z++){
        vxl_pos[2] = vxl_origin[2] + vxl_dist * (0.5 + z);
        // voxel position is deliberately not stored in voxel object to reduce memory cost
        getElement(x,y,z).evalRelationToAtoms(vxl_pos, max_depth);
      }
    }
    printf("%i%% done\n", int(100*(double(x)+1)/double(n_gridsteps[0])));
  }
  // TODO: Wrap this in function
  std::array<unsigned int,3> vxl_index;
  for(vxl_index[0] = 0; vxl_index[0] < n_gridsteps[0]; vxl_index[0]++){
    for(vxl_index[1] = 0; vxl_index[1] < n_gridsteps[1]; vxl_index[1]++){
      for(vxl_index[2] = 0; vxl_index[2] < n_gridsteps[2]; vxl_index[2]++){
        getElement(vxl_index).evalRelationToVoxels(vxl_index, max_depth);
      }
    }
    printf("%i%% done\n", int(100*(double(vxl_index[0])+1)/double(n_gridsteps[0])));
  }
}

std::map<char,double> Space::getVolume(){
  std::vector<char> types_to_tally{0b00000011,0b00000101};
  
  std::vector<unsigned int> tally;
  for (char i = 0; i < types_to_tally.size(); i++){
    tally.push_back(0);
  }

  for(unsigned int i = 0; i < n_gridsteps[0] * n_gridsteps[1] * n_gridsteps[2]; i++){ // loop through all top level voxels
    // tally bottom level voxels
    
    for (char j = 0; j < types_to_tally.size(); j++){
      tally[j] += getElement(i).tallyVoxelsOfType(types_to_tally[j],max_depth);
    }
  }
  double unit_volume = pow(grid_size,3);

  std::map<char,double> volumes;
  for (char i = 0; i < types_to_tally.size(); i++){
    volumes[types_to_tally[i]] = tally[i] * unit_volume;
  }
/*
  std::cout << "Probe inaccessible volume: " << unit_volume*total_excluded << std::endl;
  return unit_volume * total;
  */
  return volumes;
}

////////////
// OUTPUT //
////////////

Container3D<char> Space::generateTypeTensor(){
 
  // reserve memory
  Container3D<char> type_tensor = Container3D<char>(gridstepsOnLvl(0));
  
  std::array<unsigned long int,3> block_start = {0,0,0};
  int n_bot_lvl_vxl = pow(2,max_depth);
  for (unsigned int i = 0; i < n_gridsteps[0]; i++){
    block_start[0] = i*n_bot_lvl_vxl;
    for (unsigned int j = 0; j < n_gridsteps[1]; j++){
      block_start[1] = j*n_bot_lvl_vxl;
      for (unsigned int k = 0; k < n_gridsteps[2]; k++){
        block_start[2] = k*n_bot_lvl_vxl;
        getElement(i,j,k).fillTypeTensor(type_tensor, block_start, max_depth);
      }
    }
  }
  
  return type_tensor; 
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

double Space::getResolution() const {
  return grid_size;
}

/////////////////
// GET ELEMENT //
/////////////////

Voxel& Space::getElement(const unsigned int i){
  assert(i < n_gridsteps[0] * n_gridsteps[1] * n_gridsteps[2]); 
  return grid[i];
}

Voxel& Space::getElement(const unsigned int x, const unsigned int y, const unsigned int z){
  // check if element is out of bounds
  assert(x < n_gridsteps[0]);
  assert(y < n_gridsteps[1]);
  assert(z < n_gridsteps[2]);
  return grid[z * n_gridsteps[0] * n_gridsteps[1] + y * n_gridsteps[0] + x];
}

Voxel& Space::getElement(const std::array<unsigned int,3> arr){
  return grid[arr[2] * n_gridsteps[0] * n_gridsteps[1] + arr[1] * n_gridsteps[0] + arr[0]];
}

/////////////////

Voxel& Space::getVoxel(unsigned int x, unsigned int y, unsigned int z, int lvl){
  assert(max_depth >= lvl);
  // ptr needed in order to reassign variable in loop and return reference in the end
  // top lvl voxel
  Voxel* ptr_sub_vxl = &getElement(x/int(pow(2,max_depth-lvl)), y/int(pow(2,max_depth-lvl)), z/int(pow(2,max_depth-lvl)));

  // descending octree
  for (int current_lvl = max_depth-1; current_lvl>=lvl && ptr_sub_vxl->hasSubvoxel(); current_lvl--){
    ptr_sub_vxl = &(ptr_sub_vxl->getSubvoxel(
        (x/int(pow(2,current_lvl-lvl)))%2, 
        (y/int(pow(2,current_lvl-lvl)))%2, 
        (z/int(pow(2,current_lvl-lvl)))%2));
  }
  return *ptr_sub_vxl;
}

Voxel& Space::getVoxel(std::array<int,3> arr, int lvl){
  return getVoxel(arr[0], arr[1], arr[2], lvl);
}
      
// check whether coord is inside grid bounds
bool Space::coordInBounds(const std::array<int,3>& coord, const unsigned lvl){
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
  
  int depth = 0;
  std::array<unsigned int,3> indices = makeIndices(n_gridsteps, depth);

  int x_min = 0;
  int y_min = 0;
  
  int x_max = ((indices[0] >= 50)? 50: indices[0]);
  int y_max = ((indices[1] >= 25)? 25: indices[1]);

  unsigned int z = 0;
  std::cout << "Enter 'q' to quit; 'w', 'a', 's', 'd' for directional input; 'c' to continue in z direction; 'r' to go back in z direction. Enter '+' or '-' to change the octree depth." << std::endl;
  char usr_inp = '\0';
  while (usr_inp != 'q'){
  
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
        char to_print = (getVoxel(x,y,z,max_depth-depth).getType() == 0b00000011)? 'A' : 'O';
        if (!readBit(getVoxel(x,y,z,max_depth-depth).getType(),0)){to_print = '?';}
        if (getVoxel(x,y,z,max_depth-depth).getType() == 0b00010001){to_print = 'S';}
        if (getVoxel(x,y,z,max_depth-depth).getType() == 0b00000101){to_print = 'X';}
        
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

