
#include "space.h"
#include "atom.h"
#include <cmath>
#include <cassert>

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

Voxel Space::getElement(const size_t &i){
  assert(i < n_gridsteps[0] * n_gridsteps[1] * n_gridsteps[2]); // consider removing this for better performance
  return grid[i];
}

Voxel Space::getElement(const size_t &x, const size_t &y, const size_t &z){
  // check if element is out of bounds
  assert(x < n_gridsteps[0]);
  assert(y < n_gridsteps[1]);
  assert(z < n_gridsteps[2]);
  return grid[z * n_gridsteps[0] * n_gridsteps[1] + y * n_gridsteps[0] + x];
}

/////////////////
// CONSTRUCTOR //
/////////////////

Space::Space(std::vector<Atom> &atoms, const double& grid_step, const int& depth){
  setBoundaries(atoms);
  setGrid(grid_step, depth);
}

Space::Space(std::vector<Atom> &atoms){//, const double &grid_step){
  setBoundaries(atoms);
//  setGrid(grid_step);
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

void Space::setGrid(const double &grid_step, const int &depth){
  size_t n_voxels = 1;
  
  for(int dim = 0; dim < 3; dim++){
    n_gridsteps[dim] = std::ceil (std::ceil( (getSize())[dim] / grid_step ) / std::pow(2,depth) );
    n_voxels *= n_gridsteps[dim];
  }
  
  for(size_t i = 0; i < n_voxels; i++){
    grid.push_back(Voxel());
  }
  
  return;
}

