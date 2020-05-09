
#include "voxel.h"
#include "misc.h"
#include "atom.h"
#include <cmath>
#include <cassert>

////////////
// ACCESS //
////////////

Voxel& Voxel::access(const short& x, const short& y, const short& z){
  assert(x*y*z < 8);
  return data[4 * z + 2 * y + x];
}

Voxel& Voxel::access(const short& i){
  assert(i <= 8);
  return data[i];
}

char Voxel::getType(){
  return type;
}

//////////////
// SET TYPE //
//////////////

void Voxel::determineType
  (const std::vector<Atom>& atoms, 
   std::array<double,3> pos,
   const double& grid_size,
   const double max_depth)
{      

  type = 'e'; // default type empty
  if(max_depth == 0){ // for bottom level voxels
    
    for(int at = 0; at < atoms.size(); at++){ // check against all atoms
      // check if centre is inside atom
      Atom atom = atoms[at];
      double dist = distance(pos, atom.getPos());
      if(atom.rad > dist){ // voxel centre is inside atom radius
        type = 'a';
        return;
      }
      // else voxel is empty
    }
  }
  else{ // for higher level voxels
    // determine the radius of the sphere that contains the voxel
    // the voxels side length is given by the grid_size (i.e. the side length
    // of a bottom level voxel) multiplied with 2^max_depth. This value multiplied
    // with the sqrt of 3 gives the diagonal of the voxel. The radius is half the
    // diagonal.
    double radius_of_influence = pow(3,0.5)*(grid_size * pow(2,max_depth))/2;

    for(int at = 0; at < atoms.size(); at++){ // check against all atoms
      Atom atom = atoms[at];
      double dist = distance(pos, atom.getPos()); // distance betwee voxel centre and atom centre
      
      if(atom.rad > (dist + radius_of_influence)){ // voxel is entirely inside atom
        type = 'a'; // in atom
        return;
      }
      else if(atom.rad > (dist - radius_of_influence)){ // atom border cuts through voxel
        type = 'm'; // mixed
      }
      //else empty
    }
  }
  return;
}
