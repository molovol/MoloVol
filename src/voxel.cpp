#include "voxel.h"
#include "space.h"
#include "misc.h"
#include "atom.h"
#include <cmath> // abs, pow
#include <algorithm> // max_element
#include <cassert>
#include <unordered_map>

///////////////////
// AUX FUNCTIONS //
///////////////////

inline double Voxel::calcRadiusOfInfluence(const double& max_depth){
  return max_depth != 0 ? 0.86602540378 * s_grid_size * (pow(2,max_depth) - 1) : 0;
}
char mergeTypes(std::vector<Voxel>&);

// DEPRECIATED
bool allAtomsClose(const double&, const std::array<double,4>&, const std::array<Vector,4>&, char);

bool isInsideTetrahedron(const Vector&, const std::array<Vector,4>&, const std::array<Vector,4>&, bool&);
bool isInsideTetrahedron(const Vector&, const std::array<Vector,4>&, const std::array<Vector,4>&);
bool isInsideTetrahedron(const Vector&, const std::array<Vector,4>&, bool&);
bool isInsideTetrahedron(const Vector&, const std::array<Vector,4>&);
std::array<Vector,4> makeNormalsForTetrahedron(const std::array<Vector,4>&);

Vector calcProbeComponentAlong(const Vector&, const double&, const double&, const double&);
Vector calcProbeVectorInPlane(const std::array<Vector,4>, const std::array<double,4>&, const double&);
Vector calcProbeVectorNormal(const std::array<Vector,4>, const std::array<double,4>&, const double&, const Vector&);

bool isPointBetween(const Vector& vec_point, const Vector& vec_bounds);
// DEPRECIATED

/////////////////////////
// SEARCH INDEX STRUCT //
/////////////////////////
// this struct is used to store the relative indices of neighbour voxels in increasing distance,
// as well as the upper search distance limits for every level. this only needs to be created once
// before the calculation

// function declarations for functions used in SearchIndex initialisation
std::vector<std::array<unsigned int,3>> sumOfThreeSquares(unsigned long int);
std::vector<std::array<int,3>> logPermutations(const std::array<unsigned int,3>);
void signCombinations(std::vector<std::array<int,3>>&, std::array<int,3>);
void signCombinations(std::vector<std::array<int,3>>&, std::array<unsigned int,3>);

// constructors
SearchIndex::SearchIndex(){}

SearchIndex::SearchIndex(const double r_probe, const double grid_size, const unsigned int max_depth){
  _safe_lim = std::vector<unsigned int>(max_depth+1);
  _upp_lim = std::vector<unsigned int>(max_depth+1);
  for (unsigned int lvl = 0; lvl <= max_depth; ++lvl){
    // maximum distance between neighbour voxels that need to be assessed (in units of voxel side length at lvl)
    double max_dist = r_probe/(grid_size*std::pow(2,lvl)) + std::sqrt(2)/4;
    // voxel radius in units of voxel side length at current lvl
    double vxl_radius = std::sqrt(3) * (1-1/std::pow(2,lvl));
    // squared max distance between neighbours, where voxels do not have to be split
    _safe_lim[lvl] = (0 > max_dist - 2*vxl_radius)? 0 : std::pow( max_dist - (2*vxl_radius) , 2);
    // squared max distance between neighbours, that need to be assessed
    _upp_lim[lvl] = std::pow( max_dist + 2*vxl_radius , 2);
  }
  unsigned int max_element = *std::max_element(_upp_lim.begin(), _upp_lim.end());
  _index_list = computeIndices(max_element);
}

// access
const std::vector<std::array<int,3>>& SearchIndex::operator[](unsigned int i){
  return _index_list[i];
}

unsigned int SearchIndex::getUppLim(unsigned int lvl){
  return _upp_lim[lvl];
}

unsigned int SearchIndex::getSafeLim(unsigned int lvl){
  return _safe_lim[lvl];
}

// initialisation of indices
std::vector<std::vector<std::array<int,3>>> SearchIndex::computeIndices(unsigned int upp_lim){
  std::vector<std::vector<std::array<int,3>>> search_indices = std::vector<std::vector<std::array<int,3>>>(upp_lim+1);
  for (unsigned int n = 0; n <= upp_lim; n++){
    // get all combinations of three integers whose sum equals n
    std::vector<std::array<unsigned int,3>> list_of_roots = sumOfThreeSquares(n);
    for (std::array<unsigned int,3> roots : list_of_roots){
      // get all sign combinations for each integer combination
      std::vector<std::array<int,3>> temp = logPermutations(roots);
      search_indices[n].insert(std::end(search_indices[n]), std::begin(temp), std::end(temp));
    }
  }
  return search_indices;
}

// list all unique combinations of three integers, whose sum added results in n
std::vector<std::array<unsigned int,3>> sumOfThreeSquares(unsigned long int n){
  std::vector<std::array<unsigned int,3>> list_of_roots;
  // TODO roots is unused => remove ?
  std::array<unsigned int,3> roots;
  for (unsigned int x = 0; x <= std::sqrt(n); x++){
    unsigned int diff = n - std::pow(x,2);
    for (unsigned int y = x; y <= std::sqrt(diff); y++){
      unsigned int z_sqr = diff - pow(y,2);
      unsigned int z = std::sqrt(z_sqr);
      if (z_sqr == std::pow(z,2) && z >= y){
        list_of_roots.push_back({x,y,z});
      }
    }
  }
  return list_of_roots;
}

// given an array of size three, return all permutations of those three numbers
std::vector<std::array<int,3>> logPermutations(const std::array<unsigned int,3> inp_arr){
  std::vector<std::array<int,3>> list;
  signCombinations(list, inp_arr); // initial order
  for (int i = 0; i < 2; i++){
    for (int j = i+1; j < ((inp_arr[1]==inp_arr[2])? 2 : 3); j++){
      std::array<unsigned int,3> arr = inp_arr;
      if (arr[i] != arr[j]){ // if i and j are different, swap and store
        swap(arr[i], arr[j]);
        signCombinations(list, arr);
        if (i!=1 && arr[1]!=arr[2]){ // if 1 and 2 are different, swap and store
          swap(arr[1], arr[2]);
          signCombinations(list, arr);
        }
      }
    }
  }
  return list;
}

// to a vector, append all sign combinations of the number in an array of size 3
void signCombinations(std::vector<std::array<int,3>>& list, std::array<int,3> arr){
  // following two lines check whether any element is zero
  for (int sign = 0; sign < ((arr[2]==0)? 4 : 8); sign += ((arr[0]==0)? 2 : 1)){ // loop over sign combinations
    if (arr[1]==0 && sign%4 > 1){continue;} // skip if middle element is 0
    list.push_back({
      static_cast<int>((pow(-1, sign    %2))*arr[0]),
      static_cast<int>((pow(-1,(sign/2) %2))*arr[1]),
      static_cast<int>((pow(-1,(sign/4) %2))*arr[2])
    });
  }
}

void signCombinations(std::vector<std::array<int,3>>& list, std::array<unsigned int,3> inp_arr){
  std::array<int,3> arr;
  for (char i = 0; i < 3; i++){arr[i] = int(inp_arr[i]);}
  signCombinations(list, arr);
}

/////////////////
// CONSTRUCTOR //
/////////////////

Voxel::Voxel(){_type = 0;}

////////////
// ACCESS //
////////////

// data
Voxel& Voxel::getSubvoxel(const short& x, const short& y, const short& z){
  return _data[4 * z + 2 * y + x];
}
Voxel& Voxel::getSubvoxel(const short& i){
  return _data[i];
}
bool Voxel::hasSubvoxel(){return readBit(_type,7);}

// type
char Voxel::getType(){return _type;}
void Voxel::setType(char input){_type = input;}

/////////////////////////////////
// TYPE ASSIGNMENT PREPARATION //
/////////////////////////////////

// function to call before beginning the type assignment routine in order to prepare static variables
void Voxel::prepareTypeAssignment(Space* cell, AtomTree atomtree, double grid_size, double r_probe1, int max_depth){
  s_cell = cell;
  s_atomtree = atomtree;
  s_grid_size = grid_size;
  s_r_probe1 = r_probe1;
  s_search_indices = SearchIndex(r_probe1, grid_size, cell->getMaxDepth());
}

///////////////////////////////
// TYPE ASSIGNMENT 1ST ROUND //
///////////////////////////////
// part of the type assigment routine. first evaluation is only concerned with the relation between
// voxels and atoms
char Voxel::evalRelationToAtoms(const std::array<unsigned,3>& index_vxl, Vector pos_vxl, const int max_depth){
  double rad_vxl = calcRadiusOfInfluence(max_depth); // calculated every time, since max_depth may change (not expensive)

  traverseTree(s_atomtree.getRoot(), s_atomtree.getMaxRad(), pos_vxl, rad_vxl, s_r_probe1, max_depth);

  if (_type == 0){_type = 0b00001001;}
  if (readBit(_type,7)){splitVoxel(index_vxl, pos_vxl, max_depth);} // split if type mixed

  return _type;
}

// adds an array of size 8 to the voxel that contains 8 subvoxels and evaluates each subvoxel's type
void Voxel::splitVoxel(const std::array<unsigned,3>& vxl_index, const Vector& vxl_pos, const double& max_depth){
  // split into 8 subvoxels
  Vector factors;
  std::array<unsigned,3> sub_index;
  for (char z = 0; z < 2; ++z){
    sub_index[2] = vxl_index[2]*2 + z;
    factors[2] = z ? 1 : -1;
    for (char y = 0; y < 2; ++y){
      sub_index[1] = vxl_index[1]*2 + y;
      factors[1] = y ? 1 : -1;
      for (char x = 0; x < 2; ++x){
        sub_index[0] = vxl_index[0]*2 + x;
        factors[0] = x ? 1 : -1;
        // modify position
        Vector new_pos = vxl_pos + factors * s_grid_size * std::pow(2,max_depth-2);
        
        Voxel subvxl = Voxel();
        subvxl.evalRelationToAtoms(sub_index, new_pos, max_depth-1);
        
        _data.push_back(subvxl);
      }
    }
  }
  setType(mergeTypes(_data));
}

// goes through all close atoms to determine a voxel's type
void Voxel::traverseTree
  (const AtomNode* node,
   const double& rad_max,
   const Vector& pos_vxl,
   const double& rad_vxl,
   const double& rad_probe,
   const int& max_depth,
   const char exit_type,
   const char dim){

  if (node == NULL || readBit(_type,0)){return;}
  const Atom& atom = node->getAtom();

  // distance between atom and voxel along one dimension
  double dist1D = distance(atom.getPosVec(), pos_vxl, dim);

  if (abs(dist1D) > (rad_vxl + rad_max + rad_probe)){ // then atom is too far to matter for voxel type
      traverseTree(dist1D < 0 ? node->left_child : node->right_child,
          rad_max, pos_vxl, rad_vxl, rad_probe, max_depth, exit_type, (dim+1)%3);
  }
  else{ // then atom is close enough to influence voxel type
    if(isAtom(atom, pos_vxl, rad_vxl, rad_probe)){return;}

    // continue with both children
    for (AtomNode* child : {node->left_child, node->right_child}){
      traverseTree(child, rad_max, pos_vxl, rad_vxl, rad_probe, max_depth, exit_type, (dim+1)%3);
    }
  }
}

// assign a type based on the distance between a voxel and an atom
bool Voxel::isAtom(const Atom& atom, const Vector& pos_vxl, const double rad_vxl, const double rad_probe){
  Vector dist = pos_vxl - atom.getPosVec();

  if((dist < atom.getRad() - rad_vxl) && (0 < atom.getRad() - rad_vxl)){
    _type = 0b00000011;
    return true;
  }
  else if (dist < atom.getRad() + rad_vxl){
    if (readBit(_type,1)){return false;} // if inside atom
    _type = 0b10000010;
  }
  else if ((dist < atom.getRad() + rad_probe - rad_vxl) && (0 < atom.getRad() + rad_probe - rad_vxl)){
    if (readBit(_type,1)){return false;} // if mixed or inside atom
    _type = 0b00010000;
  }
  else if (dist < atom.getRad() + rad_probe + rad_vxl){
    if (readBit(_type,4) || readBit(_type,1)){return false;} // if mixed, inside atom, or potential shell
    _type = 0b10010000;
  }
  return false;
}

// not currently in use
// go through a tree, starting from node. return a list of atoms that are a specified max distance
// from a point with radius rad_point.
void Voxel::listFromTree(
  std::vector<int>& atom_id_list,
  const AtomNode* node,
  const Vector& pos_point,
  const double& rad_point,
  const double& rad_max, // ideally this value should be retrieved from node
  const double& max_dist,
  const char dim)
{
  if (node == NULL){return;}

  // distance between atom and point along one dimension
  double dist1D = distance(node->getAtom().getPosVec(), pos_point, dim);
  double rad_atom = node->getAtom().getRad();

  if (abs(dist1D) > rad_point + rad_max + max_dist) { // then atom is too far
      listFromTree(atom_id_list, dist1D<0? node->left_child : node->right_child, pos_point, rad_point, rad_max, max_dist, (dim+1)%3);
  }
  else { // then atom may be close enough
    if ((pos_point-node->getAtom().getPosVec()) < rad_point + rad_atom + max_dist){
      atom_id_list.push_back(node->getAtomId());
    }

    // continue with both children
    for (AtomNode* child : {node->left_child, node->right_child}){
      listFromTree(atom_id_list, child, pos_point, rad_point, rad_max, max_dist, (dim+1)%3);
    }
  }
}

///////////////////////////////
// TYPE ASSIGNMENT 2ND ROUND //
///////////////////////////////

char Voxel::evalRelationToVoxels(const std::array<unsigned int,3>& index, const unsigned lvl, bool split){
  // if voxel (including all subvoxels) have been assigned, then return immediately
  if (readBit(_type,0)){return _type;}
  else if (!hasSubvoxel()){ // vxl has no children
    searchForCore(index, lvl, split);
  }
  else { // vxl has children
    std::array<unsigned int,3> index_subvxl;
    for (char x = 0; x < 2; x++){
      index_subvxl[0] = index[0]*2 + x;
      for (char y = 0; y < 2; y++){
        index_subvxl[1] = index[1]*2 + y;
        for (char z = 0; z < 2; z++){
          index_subvxl[2] = index[2]*2 + z;
          getSubvoxel(x,y,z).evalRelationToVoxels(index_subvxl, lvl-1, split);
        }
      }
    }
    setType(mergeTypes(_data));
  }
  if (!readBit(_type,0)){ // if voxel not assigned
    splitVoxel(index, lvl);
  }
  return _type;
}

void Voxel::searchForCore(const std::array<unsigned int,3>& index, const unsigned lvl, bool split){
  _type = 0b00000101; // type excluded
  for (unsigned int n = (split? Voxel::s_search_indices.getSafeLim(lvl+1)*4 : 1); n <= Voxel::s_search_indices.getUppLim(lvl); ++n){
    for (std::array<int,3> coord : Voxel::s_search_indices[n]){
      coord = add(coord, index);
      if (readBit((s_cell->getVxlFromGrid(coord,lvl)).getType(),3)){
        _type = (n <= Voxel::s_search_indices.getSafeLim(lvl))? 0b00010001 : 0b10000000;
        return;
      }
    }
  }
}

// adds an array of size 8 to the voxel that contains 8 subvoxels
void Voxel::splitVoxel(const std::array<unsigned int,3>& vxl_ind, const unsigned lvl){
  _data = std::vector<Voxel>(8);
  evalRelationToVoxels(vxl_ind, lvl, true);
  setType(mergeTypes(_data));
}

///////////
// TALLY //
///////////

// TODO: Optimise. Allow for tallying multiples types at once
unsigned int Voxel::tallyVoxelsOfType(const char volume_type, const int max_depth)
{
  // if voxel is of type "mixed" (i.e. data vector is not empty)
  if(hasSubvoxel()){
    // then total number of voxels is given by tallying all subvoxels
    unsigned int total = 0;
    for(int i = 0; i < 8; i++){
      total += _data[i].tallyVoxelsOfType(volume_type, max_depth-1);
    }
    return total;
  }
  // if voxel is of the type of interest, tally the voxel in units of bottom level voxels
  else if(_type == volume_type){
    return pow(pow(2,max_depth),3); // return number of bottom level voxels
  }
  // if neither empty nor of the type of interest, then the voxel doesn't count towards the total
  return 0;
}

//////////////////////////////
// AUX FUNCTION DEFINITIONS //
//////////////////////////////

bool allAtomsClose(
    const double& r_probe,
    const std::array<double,4>& atom_radii,
    const std::array<Vector,4>& vectors,
    char n_atoms)
{
  // TODO all_atoms_close is unused => remove ?
  bool all_atoms_close = true;

  Vector last_added = vectors[n_atoms-1];
  double rad_last_added = atom_radii[n_atoms-1];
  for (char i = 0; i < n_atoms-1; i++){
    if (vectors[i]-last_added > 2*r_probe + atom_radii[i] + rad_last_added){
      return false;
    }
  }
  return true; // if all atoms close
}

bool isInsideTetrahedron(
    const Vector& vec_point,
    const std::array<Vector,4>& vec_vertices,
    const std::array<Vector,4>& norm_planes,
    bool& sign)
{
  // when multiplying the voxel vector with a normal belonging to a plane, the
  // sign of the resulting vector holds information about which side of the plane
  // the voxel is on. The order of normals in norm_planes is integral to the
  // success of this method!
  // if all signs are the same, then the voxel is inside the tetrahedron
  for (int i = 0; i < 4; i++){
    if (!i){
      sign = std::signbit((vec_point-vec_vertices[i])*norm_planes[i]);
    }
    else if (sign != std::signbit((vec_point-vec_vertices[i])*norm_planes[i])){
      return false;
    }
  }
  return true;
}

bool isInsideTetrahedron(
    const Vector& vec_point,
    const std::array<Vector,4>& vec_vertices,
    const std::array<Vector,4>& norm_planes)
{
  bool dummy = false;
  return isInsideTetrahedron(vec_point, vec_vertices, norm_planes, dummy);
}

bool isInsideTetrahedron(
    const Vector& vec_point,
    const std::array<Vector,4>& vec_vertices,
  bool& sign)
{
  return isInsideTetrahedron(vec_point, vec_vertices, makeNormalsForTetrahedron(vec_vertices), sign);
}

bool isInsideTetrahedron(
    const Vector& vec_point,
    const std::array<Vector,4>& vec_vertices)
{
  return isInsideTetrahedron(vec_point, vec_vertices, makeNormalsForTetrahedron(vec_vertices));
}

std::array<Vector,4> makeNormalsForTetrahedron(const std::array<Vector,4>& vec_vertices)
{
  std::array<Vector,4> norm_planes;
  char i = 0;
  for (char j = 0; j < 4; j++){
    for (char k = j+1; k < 4; k++){
      for (char l = k+1; l < 4; l++){
        norm_planes[i] = pow(-1,i) * crossproduct(vec_vertices[k]-vec_vertices[j], vec_vertices[l]-vec_vertices[j]);
        i++;
      }
    }
  }
  return norm_planes;
}

bool isPointBetween(const Vector& vec_point, const Vector& vec_bounds){
  Vector unitvec_bounds = vec_bounds.normalise();
  double dist_point_along_bounds = unitvec_bounds * vec_point;
  return (dist_point_along_bounds > 0 && vec_bounds > dist_point_along_bounds);
}

Vector calcProbeComponentAlong(const Vector& vec_atom_atom, const double& rad_atom_origin, const double& rad_atom_destination, const double& rad_probe)
{
  double dist = vec_atom_atom.length();
  double dist_probe = (rad_atom_origin-rad_atom_destination)*(((rad_atom_origin + rad_atom_destination) + 2*rad_probe)/(2*dist)) + (dist/2);
  return dist_probe * vec_atom_atom.normalise(); // the unitvector here may be calculated multiple times
}

Vector calcProbeVectorInPlane(const std::array<Vector,4> vec_atom, const std::array<double,4>& rad_atom, const double& rad_probe){

  Vector vec_probe_12 = calcProbeComponentAlong(vec_atom[1], rad_atom[0], rad_atom[1], rad_probe);
  Vector vec_probe_13 = calcProbeComponentAlong(vec_atom[2], rad_atom[0], rad_atom[2], rad_probe);

  Vector vec_normal_12 = crossproduct(crossproduct(vec_probe_12,vec_probe_13),vec_probe_12);
  Vector vec_normal_13 = crossproduct(crossproduct(vec_probe_12,vec_probe_13),vec_probe_13);

  double c1 = ((vec_probe_13[1]-vec_probe_12[1]) + (vec_normal_13[1]/vec_normal_13[0])*(vec_probe_12[0]-vec_probe_13[0]))/(vec_normal_12[1]-(vec_normal_12[0]*vec_normal_13[1]/vec_normal_13[0])); // system of linear equations

  return vec_probe_12 + (c1 * vec_normal_12);
}

Vector calcProbeVectorNormal(const std::array<Vector,4> vec_atom, const std::array<double,4>& rad_atom, const double& rad_probe, const Vector& vec_probe_plane){

  Vector unitvec_normal = crossproduct(vec_atom[1],vec_atom[2]).normalise();

  return unitvec_normal * pow((pow(rad_atom[0]+rad_probe,2) - vec_probe_plane*vec_probe_plane),0.5);
}

// combines the types of subvoxels into a new type for the parent. performs bitwise OR for bits 1-6
// and a bitwise AND for bit 0. sets bit 7 to true
char mergeTypes(std::vector<Voxel>& sub_vxls){
  char parent_type = 0;
  bool confirmed = true;
  for (Voxel sub_vxl : sub_vxls){
    parent_type = parent_type | sub_vxl.getType();
    confirmed &= readBit(sub_vxl.getType(),0);
  }
  setBit(parent_type,0,confirmed);
  setBitOn(parent_type,7);
  return parent_type;
}

/////////////////
// OUTPUT TYPE //
/////////////////

// function for generating a 3D tensor filled with the types of the voxel
void Voxel::fillTypeTensor(
    Container3D<char>& type_tensor,
    const std::array<unsigned long int,3> block_start,
    const int remaining_depth){
  if (remaining_depth == 0){
    type_tensor.getElement(block_start) = _type;
  }
  else if (_data.empty()){
    for (unsigned int i = block_start[0]; i < block_start[0]+pow(2,remaining_depth); i++){
      for (unsigned int j = block_start[1]; j < block_start[1]+pow(2,remaining_depth); j++){
        for (unsigned int k = block_start[2]; k < block_start[2]+pow(2,remaining_depth); k++){
          type_tensor.getElement(i,j,k) = _type;
        }
      }
    }
  }
  else {
    std::array<unsigned long int,3> new_start;
    for (char i = 0; i < 2; i++){
      new_start[0] = block_start[0] + i*pow(2,remaining_depth-1);
      for (char j = 0; j < 2; j++){
        new_start[1] = block_start[1] + j*pow(2,remaining_depth-1);
        for (char k = 0; k < 2; k++){
          new_start[2] = block_start[2] + k*pow(2,remaining_depth-1);

          getSubvoxel(i,j,k).fillTypeTensor(type_tensor, new_start, remaining_depth-1);
        }
      }
    }
  }
}
