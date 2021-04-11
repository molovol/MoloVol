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
  return max_depth != 0 ? 0.86602540378 * s_cell->getVxlSize() * (pow(2,max_depth) - 1) : 0;
}
char mergeTypes(std::vector<Voxel*>&);
char mergeTypes(const std::array<char,8>&);

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

// subvoxels (through s_cell)
Voxel& Voxel::getSubvoxel(std::array<unsigned,3> p_index, const unsigned p_lvl, const std::array<char,3>& sub_index){
  for (char i = 0; i < 3; ++i){
    p_index[i] *= 2;
    p_index[i] += sub_index[i];
  }
  return s_cell->getVxlFromGrid(p_index,p_lvl-1);
}

Voxel& Voxel::getSubvoxel(std::array<unsigned,3> p_index, const unsigned p_lvl, const char j){
  for (char i = 0; i < 3; ++i){
    p_index[i] *= 2;
    p_index[i] += (j/pow2(i))%2;
  }
  return s_cell->getVxlFromGrid(p_index,p_lvl-1);
}

Voxel& Voxel::getSubvoxel(std::array<unsigned,3> sub_index, const unsigned p_lvl){
  return s_cell->getVxlFromGrid(sub_index,p_lvl-1);
}

bool Voxel::hasSubvoxel(){return readBit(_type,7);}
bool Voxel::isAssigned(){return readBit(_type,0);}

// type
char Voxel::getType(){return _type;}
void Voxel::setType(char input){_type = input;}

/////////////////////////////////
// TYPE ASSIGNMENT PREPARATION //
/////////////////////////////////

// function to call before beginning the type assignment routine in order to prepare static variables
void Voxel::prepareTypeAssignment(Space* cell, AtomTree atomtree){
  s_cell = cell;
  s_atomtree = atomtree;
}

void Voxel::storeProbe(const double r_probe, const bool masking_mode){
  s_r_probe = r_probe;
  s_masking_mode = masking_mode;
  s_search_indices = SearchIndex(r_probe, s_cell->getVxlSize(), s_cell->getMaxDepth());
}
///////////////////////////////
// TYPE ASSIGNMENT 1ST ROUND //
///////////////////////////////
// part of the type assigment routine. first evaluation is only concerned with the relation between
// voxels and atoms
char Voxel::evalRelationToAtoms(const std::array<unsigned,3>& index_vxl, Vector pos_vxl, const int lvl){
  if (isAssigned()) {return _type;}
  double rad_vxl = calcRadiusOfInfluence(lvl); // calculated every time, since max_depth may change (not expensive)

  traverseTree(s_atomtree.getRoot(), s_atomtree.getMaxRad(), pos_vxl, rad_vxl, s_r_probe, lvl);

  if (_type == 0){_type = s_masking_mode? 0b00100001 : 0b00001001;}
  if (readBit(_type,7)){splitVoxel(index_vxl, pos_vxl, lvl);} // split if type mixed
  else {passTypeToChildren(index_vxl, lvl);}

  return _type;
}

// passes parent type to all children
void Voxel::passTypeToChildren(const std::array<unsigned,3>& index, const int lvl){
  if (lvl == 0){return;}
  std::array<unsigned,3> sub_index;
  for (char x = 0; x < 2; ++x){
    sub_index[0] = index[0]*2 + x;
    for (char y = 0; y < 2; ++y){
      sub_index[1] = index[1]*2 + y;
      for (char z = 0; z < 2; ++z){
        sub_index[2] = index[2]*2 + z;

        getSubvoxel(sub_index, lvl).setType(_type);
        getSubvoxel(sub_index, lvl).passTypeToChildren(sub_index, lvl-1);
      }
    }
  }
}

// adds an array of size 8 to the voxel that contains 8 subvoxels and evaluates each subvoxel's type
void Voxel::splitVoxel(const std::array<unsigned,3>& vxl_index, const Vector& vxl_pos, const double lvl){
  // split into 8 subvoxels
  std::array<char,8> subtypes;
  std::array<unsigned,3> sub_index;
  Vector factors;
  char i = 0;
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
        Vector new_pos = vxl_pos + factors * s_cell->getVxlSize() * std::pow(2,lvl-2);
        
        subtypes[i] = getSubvoxel(sub_index, lvl).evalRelationToAtoms(sub_index, new_pos, lvl-1);
        ++i;
        
      }
    }
  }
  setType(mergeTypes(subtypes));
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

  if (node == NULL || isAssigned()){return;}
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
    _type = s_masking_mode? 0b01000000 : 0b00010000;
  }
  else if (dist < atom.getRad() + rad_probe + rad_vxl){
    if (readBit(_type,4) || readBit(_type,1)){return false;} // if mixed, inside atom, or potential shell
    _type = s_masking_mode? 0b11000000 : 0b10010000;
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
  if (isAssigned()){return _type;}
  else if (!hasSubvoxel()){ // vxl has no children
    searchForCore(index, lvl, split);
  }
  else { // vxl has children
    std::array<unsigned int,3> index_subvxl;
    std::array<char,8> subtypes;
    char i = 0;
    for (char x = 0; x < 2; x++){
      index_subvxl[0] = index[0]*2 + x;
      for (char y = 0; y < 2; y++){
        index_subvxl[1] = index[1]*2 + y;
        for (char z = 0; z < 2; z++){
          index_subvxl[2] = index[2]*2 + z;
          subtypes[i] = getSubvoxel(index_subvxl, lvl).evalRelationToVoxels(index_subvxl, lvl-1, split);
          ++i;
        }
      }
    }
    setType(mergeTypes(subtypes));
  }
  
  if (hasSubvoxel()){ splitVoxel(index, lvl);} // if voxel has been marked to be split
  else {passTypeToChildren(index, lvl);}
  assert(_type != 0b00000100);
  return _type;
}

void Voxel::searchForCore(const std::array<unsigned int,3>& index, const unsigned lvl, bool split){
  _type = s_masking_mode? 0b00000100 : 0b00000101; // type excluded
  //_type = 0b00000101; // type excluded
  // TODO: make it so that after masking mode, type is unassigned

  const char shell = s_masking_mode? 0b01000001 : 0b00010001;
  const char bit_pos_core = s_masking_mode? 5 : 3;

  for (unsigned int n = (split? Voxel::s_search_indices.getSafeLim(lvl+1)*4 : 1); n <= Voxel::s_search_indices.getUppLim(lvl); ++n){
    // called very often; keep section inexpensive
    for (std::array<int,3> coord : Voxel::s_search_indices[n]){
      coord = add(coord, index);
      if (readBit((s_cell->getVxlFromGrid(coord,lvl)).getType(),bit_pos_core)){
        _type = (n <= Voxel::s_search_indices.getSafeLim(lvl))? shell : 0b10000000; // mark to split
        return;
      }
    }
  }
}

// adds an array of size 8 to the voxel that contains 8 subvoxels
void Voxel::splitVoxel(const std::array<unsigned int,3>& vxl_ind, const unsigned lvl){
  evalRelationToVoxels(vxl_ind, lvl, true);
}

///////////
// TALLY //
///////////

// TODO: Optimise. Allow for tallying multiples types at once
unsigned int Voxel::tallyVoxelsOfType(const std::array<unsigned,3>& index, const char volume_type, const int lvl)
{
  // if voxel is of type "mixed" (i.e. data vector is not empty)
  if(hasSubvoxel()){
    // then total number of voxels is given by tallying all subvoxels
    unsigned int total = 0;
    std::array<unsigned,3> sub_index;
    for(char x = 0; x < 2; ++x){
      sub_index[0] = index[0]*2 + x;
      for(char y = 0; y < 2; ++y){
        sub_index[1] = index[1]*2 + y;
        for(char z = 0; z < 2; ++z){
          sub_index[2] = index[2]*2 + z;
          total += getSubvoxel(sub_index, lvl).tallyVoxelsOfType(sub_index, volume_type, lvl-1);
        }
      }
    }
    return total;
  }
  // if voxel is of the type of interest, tally the voxel in units of bottom level voxels
  else if(_type == volume_type){
    return pow(pow(2,lvl),3); // return number of bottom level voxels
  }
  // if neither empty nor of the type of interest, then the voxel doesn't count towards the total
  return 0;
}

//////////////////////////////
// AUX FUNCTION DEFINITIONS //
//////////////////////////////

// combines the types of subvoxels into a new type for the parent. performs bitwise OR for bits 1-6
// and a bitwise AND for bit 0. sets bit 7 to true
char mergeTypes(const std::array<char,8>& subtypes){
  char parent_type = 0;
  bool confirmed = true;
  for (char sub_type : subtypes){
    parent_type = parent_type | sub_type;
    confirmed &= readBit(sub_type,0);
  }
  setBit(parent_type,0,confirmed);
  setBitOn(parent_type,7);
  return parent_type;
}

char mergeTypes(std::vector<Voxel*>& sub_vxls){
  char parent_type = 0;
  bool confirmed = true;
  for (Voxel* sub_vxl : sub_vxls){
    parent_type = parent_type | sub_vxl->getType();
    confirmed &= readBit(sub_vxl->getType(),0);
  }
  setBit(parent_type,0,confirmed);
  setBitOn(parent_type,7);
  return parent_type;
}

