
#include "space.h"
#include "atom.h"

Space::Space(std::vector<Atom> &atoms){
  findMinMaxFromAtoms(atoms, cart_min, cart_max);
//  for(int dim = 0; dim < 3; dim++){
//* space needs to be made larger to fit whole atomic volume
}

std::array<double,3> Space::getMin(){
  return cart_min;
}

std::array<double,3> Space::getMax(){
  return cart_max;
}

// member function loops through all atoms (previously extracted
// from file) and saves the minimum and maximum positions in all
// three cartesian directions out of all atoms.
inline void Space::findMinMaxFromAtoms
  (std::vector<Atom> &atoms,
   std::array<double,3> &cart_min, 
   std::array<double,3> &cart_max)
{
  for(int at = 0; at < atoms.size(); at++){
    std::array<double,3> atom_pos = {atoms[at].pos_x, atoms[at].pos_y, atoms[at].pos_z}; // atom positions are correct
    
    // if we are at the first atom, then the atom position is min/max by default
    if(at == 0){
      cart_min = atom_pos;
      cart_max = atom_pos;
    }
    
    // after the first atom, begin comparing the atom positions to min/max
    else{
      for(int dim = 0; dim < 3; dim++){ 
        if(atom_pos[dim] > cart_max[dim]){ 
          cart_max[dim] = atom_pos[dim];
        }
        if(atom_pos[dim] < cart_min[dim]){
          cart_min[dim] = atom_pos[dim];
        }
      }
    }
  }
  return;
}
    
