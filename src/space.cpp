#include "space.h"
#include "atom.h"
#include "voxel.h"
#include "atomtree.h"
#include <cmath>
#include <cassert>

/////////////////
// VOLUME COMP //
/////////////////

// sets all voxel's types, determined by the input atoms
void Space::placeAtomsInGrid(const AtomTree& atomtree, const double& r_probe){
  // calculate position of first voxel
  const std::array<double,3> vxl_origin = getOrigin();
  
  // calculate side length of top level voxel
  const double vxl_dist = grid_size * pow(2,max_depth);
  
  std::array<double,3> vxl_pos;

  // save variable that all voxels need access to for their type determination
  //Voxel::storeUniversalVariables();

  for(size_t x = 0; x < n_gridsteps[0]; x++){
    vxl_pos[0] = vxl_origin[0] + vxl_dist * (0.5 + x);
    for(size_t y = 0; y < n_gridsteps[1]; y++){
      vxl_pos[1] = vxl_origin[1] + vxl_dist * (0.5 + y);
      for(size_t z = 0; z < n_gridsteps[2]; z++){
        vxl_pos[2] = vxl_origin[2] + vxl_dist * (0.5 + z);
        // voxel position is deliberately not stored in voxel object to reduce memory cost
        getElement(x,y,z).determineType(vxl_pos, grid_size, r_probe, max_depth, atomtree);
      }
    }
  }      
}

double Space::getVolume(){
  //printGrid();
  // calc Volume
  size_t total = 0;
  size_t total_excluded = 0;
  for(size_t i = 0; i < n_gridsteps[0] * n_gridsteps[1] * n_gridsteps[2]; i++){ // loop through all top level voxels
    // tally bottom level voxels
    total += getElement(i).tallyVoxelsOfType('a',max_depth);
    total_excluded += getElement(i).tallyVoxelsOfType('x',max_depth);
  }
  double unit_volume = pow(grid_size,3);
  std::cout << "Probe inaccessible volume: " << unit_volume*total_excluded << std::endl;
  return unit_volume * total;
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

Voxel& Space::getElement(const size_t &i){
  assert(i < n_gridsteps[0] * n_gridsteps[1] * n_gridsteps[2]); 
  return grid[i];
}

Voxel& Space::getElement(const size_t &x, const size_t &y, const size_t &z){
  // check if element is out of bounds
  assert(x < n_gridsteps[0]);
  assert(y < n_gridsteps[1]);
  assert(z < n_gridsteps[2]);
  return grid[z * n_gridsteps[0] * n_gridsteps[1] + y * n_gridsteps[0] + x];
}

// displays voxel grid types as matrix in the terminal. useful for debugging
void Space::printGrid(){
  char usr_inp = '\0';
  int x_min = 0;
  int y_min = 0;
  
  int x_max = ((n_gridsteps[0] >= 50)? 50: n_gridsteps[0]);
  int y_max = ((n_gridsteps[1] >= 25)? 25: n_gridsteps[1]);

  size_t z = 0;
  std::cout << "Enter 'q' to quit; 'w', 'a', 's', 'd' for directional input; 'c' to continue in z direction; 'r' to go back in z direction" << std::endl;
  while (usr_inp != 'q'){

    // x and y coordinates
    if (usr_inp == 'a'){x_min--; x_max--;}
    else if (usr_inp == 'd'){x_min++; x_max++;}
    else if (usr_inp == 's'){y_min++; y_max++;}
    else if (usr_inp == 'w'){y_min--; y_max--;}
    
    if (x_min < 0){x_min++; x_max++;}
    if (y_min < 0){y_min++; y_max++;}
    if (x_max > n_gridsteps[0]){x_min--; x_max--;}
    if (y_max > n_gridsteps[1]){y_min--; y_max--;}
    
    // z coordinate
    if (usr_inp == 'r'){
      if (z==0){
        std::cout << "z position at min. ";
      }
      else{z--;}
    }
    else if (usr_inp == 'c'){
      z++;
      if (z >= n_gridsteps[2]){
        z--;
        std::cout << "z position at max. ";
      }
    }
    
    // print matrix
    for(size_t y = y_min; y < y_max; y++){
      for(size_t x = x_min; x < x_max; x++){
        std::cout << getElement(x,y,z).getType();
      }
      std::cout << std::endl;
    }
    
    // get user input
    std::cout << std::endl;
    std::cout << "INPUT: ";
    std::cin >> usr_inp;
  }
  return;
}


/////////////////
// CONSTRUCTOR //
/////////////////

Space::Space(std::vector<Atom> &atoms, const double& bottom_level_voxel_dist, const int& depth)
  :grid_size(bottom_level_voxel_dist), max_depth(depth){
  setBoundaries(atoms);
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
void Space::setBoundaries(const std::vector<Atom> &atoms){
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
    cart_min[dim] -= (1.1 * max_radius);
    cart_max[dim] += (1.1 * max_radius);    
  }
  return;
}

// based on the grid step and the octree max_depth, this function produces a
// 3D grid (in form of a 1D vector) that contains all top level voxels.
void Space::setGrid(){
  size_t n_voxels = 1;
  
  for(int dim = 0; dim < 3; dim++){
    n_gridsteps[dim] = std::ceil (std::ceil( (getSize())[dim] / grid_size ) / std::pow(2,max_depth) );
    n_voxels *= n_gridsteps[dim];
  }
  
  for(size_t i = 0; i < n_voxels; i++){
    grid.push_back(Voxel());
  }
  
  return;
}

