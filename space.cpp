
#include "space.h"
#include "atom.h"
#include "voxel.h"
#include <cmath>
#include <cassert>

/////////////////
// VOLUME COMP //
/////////////////

void Space::placeAtomsInGrid(const std::vector<Atom> &atoms){
  // calculate position of first voxel
  std::array<double,3> vxl_origin = getOrigin();
  // origin of the cell has to be offset by half the grid size
  for(int dim = 0; dim < 3; dim++){
    vxl_origin[dim] += grid_size/2;
  }
  
  // calculate side length of top level voxel
  double vxl_dist = grid_size * pow(2,max_depth);
  
  for(size_t x = 0; x < n_gridsteps[0]; x++){
    for(size_t y = 0; y < n_gridsteps[1]; y++){
      for(size_t z = 0; z < n_gridsteps[2]; z++){
        Voxel& vxl = getElement(x,y,z);
        std::array<double,3> vxl_pos = 
          {vxl_origin[0] + vxl_dist * x, vxl_origin[1] + vxl_dist * y, vxl_origin[2] + vxl_dist * z};
        vxl.determineType(atoms, vxl_pos, grid_size, max_depth);
      }
    }
  }      
  return;
}

double Space::getVolume(){
  return 0;
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
  assert(i < n_gridsteps[0] * n_gridsteps[1] * n_gridsteps[2]); // consider removing this for better performance
  return grid[i];
}

Voxel& Space::getElement(const size_t &x, const size_t &y, const size_t &z){
  // check if element is out of bounds
  assert(x < n_gridsteps[0]);
  assert(y < n_gridsteps[1]);
  assert(z < n_gridsteps[2]);
  return grid[z * n_gridsteps[0] * n_gridsteps[1] + y * n_gridsteps[0] + x];
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
// radius of all atoms and sets the space boundries slightly so
// that all atoms fit the space.
//void Space::findMinMaxFromAtoms
void Space::setBoundaries
  (std::vector<Atom> &atoms)
{
  double max_radius = 0;
  for(int at = 0; at < atoms.size(); at++){
    std::array<double,3> atom_pos = {atoms[at].pos_x, atoms[at].pos_y, atoms[at].pos_z}; // atom positions are correct
  
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

