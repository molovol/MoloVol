#include "voxel.h"
#include "misc.h"
#include "atom.h"
#include "atomtree.h"
#include <cmath> // abs, pow
#include <cassert>

///////////////////
// AUX FUNCTIONS //
///////////////////

inline double calcSphereOfInfluence(const double& grid_size, const double& max_depth){
  return 0.70710678118 * grid_size * (pow(2,max_depth)-1);
}

/////////////////
// CONSTRUCTOR //
/////////////////

Voxel::Voxel(){
  type = 'e';
}

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

char Voxel::determineType(
    std::array<double,3> vxl_pos, // voxel centre
    const double& grid_size,
    const double max_depth,
    const AtomTree& atomtree)
{
  double r_at = atomtree.getMaxRad();
  double r_vxl = calcSphereOfInfluence(grid_size, max_depth); // calculated every time, since max_depth may change
    
  traverseTree(atomtree.getRoot(), 0, r_at, r_vxl, vxl_pos, grid_size, max_depth); 

  if(type == 'm'){
    splitVoxel(vxl_pos, grid_size, max_depth, atomtree);
  }
	return type;
}

void Voxel::splitVoxel(const std::array<double,3>& vxl_pos, const double& grid_size, const double& max_depth, const AtomTree& atomtree){
  // split into 8 subvoxels
	short resultcount = 0;
  for(int i = 0; i < 8; i++){
    data.push_back(Voxel());
    // modify position
    std::array<int,3> factors = {
      (i%2) >= 1 ? 1 : -1,
      (i%4) >= 2 ? 1 : -1,
       i    >= 4 ? 1 : -1};
   
    std::array<double,3> new_pos;
    for(int dim = 0; dim < 3; dim++){
      new_pos[dim] = vxl_pos[dim] + factors[dim] * grid_size * std::pow(2,max_depth-2);//why -2?
    }
    resultcount += data[i].determineType(new_pos, grid_size, max_depth-1, atomtree);
  }
	//determine if all children have the same type
	if (resultcount == 'a'*8){
	  type = 'a';
	} else if (resultcount == 'e'*8) {
	  type = 'e';
	}
}

// use the properties of the binary tree to recursively traverse the tree and 
// only check the voxel type with respect to relevant atoms. at the end of this 
// method, the type of the voxel will have been set.
void Voxel::traverseTree
  (const AtomNode* node, 
   int dim, const double& at_rad, 
   const double& vxl_rad, 
   const std::array<double,3> vxl_pos, 
   const double& grid_size, 
   const double& max_depth){

  if (node == NULL){return;}
  // distance between atom and voxel along one dimension
  double dist1D = distance(node->atom->getPos(), vxl_pos, dim);

  // is voxel close enough to atom?
  if (abs(dist1D) > (vxl_rad + at_rad)){ // if not, continue to next child
      traverseTree(dist1D < 0 ? node->left_child : node->right_child,
				   (dim+1)%3, at_rad, vxl_rad, vxl_pos, grid_size, max_depth);
  } else{ // if voxel is close enough, check distance to the node's atom. if needed,
    // continue with both children
    determineTypeSingleAtom(*(node->atom), vxl_pos, grid_size, max_depth);
    for (AtomNode* child : {node->left_child, node->right_child}){
      traverseTree(child, (dim+1)%3, at_rad, vxl_rad, vxl_pos, grid_size, max_depth);
    }
  }
}

void Voxel::determineTypeSingleAtom
  (const Atom& atom, 
   std::array<double,3> vxl_pos, // voxel centre
   const double& grid_size,
   const double max_depth)
{
  // return if voxel is already in atom 
  if(type == 'a'){return;}

  double dist_vxl_at = distance(vxl_pos, atom.getPos());

  // if bottom level voxel: radius of influence = 0, i.e., treat like point
  // if higher level voxel: radius of influence > 0
  double radius_of_influence = 
    max_depth != 0 ? pow(3,0.5)*(grid_size * pow(2,max_depth))/2 : 0; // TODO: avoid expensive pow function
  
  // is voxel inside atom? 
  if(atom.rad > (dist_vxl_at + radius_of_influence)){ 
    type = 'a'; // in atom
    return;
  }
  // is voxel partially inside atom?
  else if(atom.rad > (dist_vxl_at - radius_of_influence)){
    type = 'm'; // mixed
    return;
  // else type remains unchanged
  }
}

///////////
// TALLY //
///////////

size_t Voxel::tallyVoxelsOfType(const char volume_type, const int max_depth){
  // if voxel is of type "mixed" (i.e. data vector is not empty)
  if(!data.empty()){
    // then total number of voxels is given by tallying all subvoxels
    size_t total = 0;
    for(int i = 0; i < 8; i++){
      total += data[i].tallyVoxelsOfType(volume_type, max_depth-1);
    }
    return total;
  }
  // if voxel is of the type of interest, tally the voxel in units of bottom level voxels
  else if(type == volume_type){
    return pow(pow(2,max_depth),3); // return number of bottom level voxels
  }
  // if neither empty nor of the type of interest, then the voxel doesn't count towards the total
  return 0;
}
