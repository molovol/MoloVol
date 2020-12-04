#include "voxel.h"
#include "misc.h"
#include "atom.h"
//#include "atomtree.h"
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

void Voxel::setType(char input){
  type = input;
}

//////////////
// SET TYPE //
//////////////

void Voxel::storeUniversal(AtomTree atomtree, double grid_size, double r_probe1){
  _atomtree = atomtree;
  _grid_size = grid_size;
  _r_probe1 = r_probe1;
}

char Voxel::determineType(std::array<double,3> vxl_pos, const double max_depth)
{
  double r_at = _atomtree.getMaxRad();
  double r_vxl = calcSphereOfInfluence(_grid_size, max_depth); // calculated every time, since max_depth may change
  
  bool accessibility_checked = false; // keeps track of whether probe accessibility has been determined
  traverseTree(_atomtree.getRoot(), 0, r_at, r_vxl, vxl_pos, max_depth, accessibility_checked); 
  
  if(type == 'm'){
    splitVoxel(vxl_pos, max_depth);
  }
	return type;
}

void Voxel::splitVoxel(const std::array<double,3>& vxl_pos, const double& max_depth){
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
      new_pos[dim] = vxl_pos[dim] + factors[dim] * _grid_size * std::pow(2,max_depth-2);//why -2?
    }
    resultcount += data[i].determineType(new_pos, max_depth-1);
  }
	//determine if all children have the same type
  // TODO: delete data vector in this case
	if (resultcount == 'a'*8){
	  type = 'a';
	} else if (resultcount == 'e'*8) {
	  type = 'e';
	}
  //
}

// use the properties of the binary tree to recursively traverse the tree and 
// only check the voxel type with respect to relevant atoms. at the end of this 
// method, the type of the voxel will have been set.
void Voxel::traverseTree
  (const AtomNode* node, 
   int dim, const double& at_rad, 
   const double& vxl_rad, 
   const std::array<double,3> vxl_pos, 
   const double& max_depth,
   bool& accessibility_checked){

  if (node == NULL){return;}
  // distance between atom and voxel along one dimension
  double dist1D = distance(node->atom->getPos(), vxl_pos, dim);
 
  // this condition is not optimised. _r_probe1 might be unneccessary here and slow down the algo
  if (abs(dist1D) > (vxl_rad + at_rad + _r_probe1)){ // then atom is too far to matter for voxel type
      traverseTree(dist1D < 0 ? node->left_child : node->right_child,
				   (dim+1)%3, at_rad, vxl_rad, vxl_pos, max_depth, accessibility_checked);
  } else{ // then atom is close enough to influence voxel type
    // evaluate voxel type with respect to the found atom
    determineTypeSingleAtom(*(node->atom), vxl_pos, max_depth, accessibility_checked);
    // continue with both children
    for (AtomNode* child : {node->left_child, node->right_child}){
      traverseTree(child, (dim+1)%3, at_rad, vxl_rad, vxl_pos, max_depth, accessibility_checked);
    }
  }
}

void Voxel::determineTypeSingleAtom(const Atom& atom, 
                                    std::array<double,3> vxl_pos, // voxel centre
                                    const double max_depth,
                                    bool& accessibility_checked){


  // return if voxel is already in atom
  if(type == 'a' || type == 'm'){return;}

  double dist_vxl_at = distance(vxl_pos, atom.getPos());

  // if bottom level voxel: radius of influence = 0, i.e., treat like point
  // if higher level voxel: radius of influence > 0
  double radius_of_influence = 
    max_depth != 0 ? 1.73205080757 * _grid_size * (pow(2,max_depth) - 1) : 0;
  // I believe the use of a conditional makes the code slightly faster -JM
  
  // is voxel inside atom? 
  if (isAtom(atom, dist_vxl_at, radius_of_influence)){return;}

  // is voxel partially inside atom?
  //else if(isAtAtomEdge(atom, dist_vxl_at, radius_of_influence)){return;}
  
  // is voxel inaccessible by probe?
  // pass _r_probe1 as proper argument, so that this routine may be reused for two probe mode
  else if (!accessibility_checked && isProbeExcluded(atom, vxl_pos, _r_probe1, radius_of_influence, accessibility_checked)){
    return;
  }
  // else type remains unchanged
}

////////////////////
// CHECK FOR TYPE //
////////////////////

bool Voxel::isAtom(const Atom& atom, const double& dist_vxl_at, const double& radius_of_influence){
  if(atom.rad > (dist_vxl_at + radius_of_influence)){ 
    setType('a'); // in atom
    return true;
  }
  else if(atom.rad >= (dist_vxl_at - radius_of_influence)){
    setType('m'); // mixed
    return true;
  }
  return false;
}

bool Voxel::isProbeExcluded(const Atom& atom, const std::array<double,3>& vxl_pos, const double& r_probe, const double& radius_of_influence, bool& accessibility_checked){ 
  accessibility_checked = true;

  // for simplicity all vectors are shifted by -vec_offset, so that the atom is in the origin
  Vector vec_offset = Vector(atom.getPos());
  double rad_atom = atom.getRad(); 
  
  Vector vec_vxl = Vector(vxl_pos) - vec_offset;
  
  for (const Atom* adj : atom.adjacent_atoms){
    Vector vec_adj = Vector(adj->getPos()) - vec_offset; // vector pointing from atom to neighbour atom
    double rad_adj = adj->getRad();
    
    if (vec_adj < 2*r_probe + rad_atom + rad_adj){ // then atoms are close enough
      if (isExcludedByPair(vec_vxl, vec_adj, rad_atom, rad_adj, r_probe, radius_of_influence)){
        return true;
      } 
    }
  }
  return false;
}

void breakpoint(){return;}

bool Voxel::isExcludedByPair(const Vector& vec_vxl, const Vector& vec_atat, const double& rad_atom1, const double& rad_atom2, const double& rad_probe, const double& rad_vxl){

  Vector unitvec_parallel = vec_atat.normalise();
  double vxl_parallel = vec_vxl * unitvec_parallel; 
  if (vxl_parallel > 0 && vec_atat > vxl_parallel){ // then voxel is between atoms
    Vector unitvec_orthogonal = (vec_vxl-unitvec_parallel*vxl_parallel).normalise();
    double vxl_orthogonal = vec_vxl * unitvec_orthogonal; 
    
    double dist_atom1_probe = rad_atom1 + rad_probe;
    double dist_atom2_probe = rad_atom2 + rad_probe;
    double dist_atom1_atom2 = vec_atat.length();

    double angle_atom1 = acos((pow(dist_atom1_probe,2) + pow(dist_atom1_atom2,2) - pow(dist_atom2_probe,2))/(2*dist_atom1_probe*dist_atom1_atom2));
    double angle_vxl1 = atan(vxl_orthogonal/vxl_parallel);

    if (angle_atom1 > angle_vxl1){
      double angle_atom2 = acos((pow(dist_atom2_probe,2) + pow(dist_atom1_atom2,2) - pow(dist_atom1_probe,2))/(2*dist_atom2_probe*dist_atom1_atom2));
      double angle_vxl2 = atan(vxl_orthogonal/(dist_atom1_atom2-vxl_parallel));
      if (angle_atom2 > angle_vxl2){ // then voxel is in triangle spanned by atoms and probe
        double probe_parallel = ((pow(dist_atom1_probe,2) + pow(dist_atom1_atom2,2) - pow(dist_atom2_probe,2))/(2*dist_atom1_atom2));
        double probe_orthogonal = pow(pow(dist_atom1_probe,2)-pow(probe_parallel,2),0.5);

        Vector vec_probe = probe_parallel * unitvec_parallel + probe_orthogonal * unitvec_orthogonal;
        
        if (vec_probe-vec_vxl > rad_probe+rad_vxl){ // then all subvoxels are inaccessible
          setType('x');
          return true;
        }
        else if (vec_probe-vec_vxl <= rad_probe-rad_vxl){ // then all subvoxels are accessible
          return false;
        }
        else { // then each subvoxel has to be evaluated
          breakpoint();
          setType('m');
          return false;
        }
      }
    }
  }
  return false;
}

///////////
// TALLY //
///////////

// TODO: Optimise. Allow for tallying multiples types at once
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

