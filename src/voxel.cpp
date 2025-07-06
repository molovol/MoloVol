#include "voxel.h"
#include "space.h"
#include "misc.h"
#include "atom.h"
#include "controller.h"
#include <cmath> // abs, pow
#include <algorithm> // max_element, swap
#include <cassert>
#include <unordered_map>

///////////////////
// AUX FUNCTIONS //
///////////////////

inline double Voxel::calcVxlRadius(const double& max_depth){
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

std::vector<std::vector<std::array<int,3>>> SearchIndex::computeIndices(unsigned int upp_lim, const bool include_zero){
  std::vector<std::vector<std::array<int,3>>> indices = computeIndices(upp_lim);
  if (!include_zero){
    indices.erase(indices.begin());
  }
  return indices;
}

// list all unique combinations of three integers, whose sum added results in n
std::vector<std::array<unsigned int,3>> sumOfThreeSquares(unsigned long int n){
  std::vector<std::array<unsigned int,3>> list_of_roots;
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
        std::swap(arr[i], arr[j]);
        signCombinations(list, arr);
        if (i!=1 && arr[1]!=arr[2]){ // if 1 and 2 are different, swap and store
          std::swap(arr[1], arr[2]);
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

Voxel::Voxel(){
  _type = 0;
  _identity = 0;
}

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
bool Voxel::isCore(){return readBit(_type,3);}
bool Voxel::isAssigned(){return readBit(_type,0);}

// type
char Voxel::getType() const {return _type;}
void Voxel::setType(char input){_type = input;}

// ID
void Voxel::setID(unsigned char id){_identity = id;}
unsigned char Voxel::getID() const {return _identity;}

/////////////////////////////////
// TYPE ASSIGNMENT PREPARATION //
/////////////////////////////////

// function to call before beginning the type assignment routine in order to prepare static variables
void Voxel::prepareTypeAssignment(Space* cell, std::vector<Atom>& atoms){
  s_cell = cell;
  delete s_atomtree;
  s_atomtree = new AtomTree(atoms);
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
  if(Ctrl::getInstance()->getAbortFlag()){return 0;}
  if (isAssigned()) {return _type;}
  if (!hasSubvoxel()) {
    double rad_vxl = calcVxlRadius(lvl); // calculated every time, since max_depth may change (not expensive)
    traverseTree(s_atomtree->getRoot(), s_atomtree->getMaxRad(), pos_vxl, rad_vxl, s_r_probe, lvl);
    if (_type == 0){_type = s_masking_mode? 0b00100001 : 0b00001001;}
  }
  if (hasSubvoxel()) {
    splitVoxel(index_vxl, pos_vxl, lvl);
  }
  else {
    // voxel has been processed
    passTypeToChildren(index_vxl, lvl);
  }
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

void Voxel::passIDtoChildren(const std::array<unsigned,3>& index, const int lvl){
  if (lvl == 0){return;}
  std::array<unsigned,3> sub_index;
  for (char x = 0; x < 2; ++x){
    sub_index[0] = index[0]*2 + x;
    for (char y = 0; y < 2; ++y){
      sub_index[1] = index[1]*2 + y;
      for (char z = 0; z < 2; ++z){
        sub_index[2] = index[2]*2 + z;

        getSubvoxel(sub_index, lvl).setID(_identity);
        getSubvoxel(sub_index, lvl).passIDtoChildren(sub_index, lvl-1);
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
   const double rad_max,
   const Vector& pos_vxl,
   const double rad_vxl,
   const double rad_probe,
   const int max_depth,
   const char exit_type,
   const char dim){

  if (node == NULL){return;}
  const Atom& atom = node->getAtom();

  // distance between atom and voxel along one dimension
  double dist1D = distance(atom.getPosVec(), pos_vxl, dim);

  if (abs(dist1D) > (rad_vxl + rad_max + rad_probe)){ // then atom is too far to matter for voxel type
      traverseTree(dist1D < 0 ? node->getLeftChild() : node->getRightChild(),
          rad_max, pos_vxl, rad_vxl, rad_probe, max_depth, exit_type, (dim+1)%3);
  }
  else{ // then atom is close enough to influence voxel type
    if(isAtom(atom, pos_vxl, rad_vxl, rad_probe)){return;}

    // continue with both children
    for (const AtomNode* child : {node->getLeftChild(), node->getRightChild()}){
      traverseTree(child, rad_max, pos_vxl, rad_vxl, rad_probe, max_depth, exit_type, (dim+1)%3);
    }
  }
}

// assign a type based on the distance between a voxel and an atom
bool Voxel::isAtom(const Atom& atom, const Vector& pos_vxl, const double rad_vxl, const double rad_probe){
  Vector dist = pos_vxl - atom.getPosVec();

  if((dist < atom.getRad() - rad_vxl) && (0 < atom.getRad() - rad_vxl)){ // if completely inside atom
    _type = 0b00000011;
    return true;
  }
  else if (dist < atom.getRad() + rad_vxl){ // if partially inside atom
    if (readBit(_type,1)){return false;} // if inside atom // TODO: this line may be unnecessary
    _type = 0b10000010;
  }
  else if ((dist < atom.getRad() + rad_probe - rad_vxl) && (0 < atom.getRad() + rad_probe - rad_vxl)){ // if outside atom but not touching potential probe core
    if (readBit(_type,1)){return false;} // if mixed or inside atom
    _type = s_masking_mode? 0b01000000 : 0b00010000;
  }
  else if (dist < atom.getRad() + rad_probe + rad_vxl){ // if outside atom but touching potential probe core
    if (readBit(_type,4) || readBit(_type,1)){return false;} // if mixed, inside atom, or potential shell
    _type = s_masking_mode? 0b11000000 : 0b10010000;
  }
  return false;
}

// TODO: Remove, instead use AtomTree::listAllWithin()
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
      listFromTree(atom_id_list, dist1D<0? node->getLeftChild() : node->getRightChild(), pos_point, rad_point, rad_max, max_dist, (dim+1)%3);
  }
  else { // then atom may be close enough
    if ((pos_point-node->getAtom().getPosVec()) < rad_point + rad_atom + max_dist){
      atom_id_list.push_back(node->getAtomId());
    }

    // continue with both children
    for (const AtomNode* child : {node->getLeftChild(), node->getRightChild()}){
      listFromTree(atom_id_list, child, pos_point, rad_point, rad_max, max_dist, (dim+1)%3);
    }
  }
}

///////////////
// CAVITY ID //
///////////////

struct VoxelLoc{
  VoxelLoc() = default;
  VoxelLoc(const std::array<unsigned,3>& index, const int lvl) : index(index), lvl(lvl), interface_vxl(false) {}
  VoxelLoc(const std::array<unsigned,3>& index, const int lvl, const bool interface_vxl)
    : index(index), lvl(lvl), interface_vxl(interface_vxl) {}
  std::array<unsigned,3> index;
  int lvl;
  bool interface_vxl;
};

class FloodStack{
  private:
    std::vector<VoxelLoc> economy_lane;
    std::vector<VoxelLoc> priority_lane;
  public:
    FloodStack() = default;
    VoxelLoc popOut(){
      VoxelLoc vxl;
      if (priority_lane.empty()){
        vxl = economy_lane.back();
        vxl.interface_vxl = true;
        economy_lane.pop_back();
      }
      else {
        vxl = priority_lane.back();
        priority_lane.pop_back();
      }
      return vxl;
    }
    size_t size() const {
      return economy_lane.size() + priority_lane.size();
    }
    size_t sizePriority() const {
      return priority_lane.size();
    }
    void pushBack(const VoxelLoc& vxl, const bool priority=false){
      (priority? priority_lane : economy_lane).push_back(vxl);
    }
};

// this function returns false when accessing an existing cavity and returns
// true every time a new cavity has been processed
bool Voxel::floodFill(std::vector<Cavity>& cavities, const unsigned char id, const std::array<unsigned,3>& start_index, const int start_lvl, const bool cavity_type){
  if(getID() != 0){return false;}
  // initialise flood fill stack
  FloodStack stack;

  // set the ID of the start voxel and all its children
  setID(id);
  passIDtoChildren(start_index, start_lvl);
  // add first voxel to stack
  stack.pushBack(VoxelLoc(start_index, start_lvl), 
      cavity_type? isInterfaceVxl(VoxelLoc(start_index, start_lvl)) : false);
  
  int n_interface = 0;
  bool at_interface = false;
  while (stack.size() > 0){
    if (Ctrl::getInstance()->getAbortFlag()){return false;}
    Ctrl::getInstance()->updateCalculationStatus(); // checks for abort button click

    // get the next voxel from the stack
    VoxelLoc vxl = stack.popOut();

    // get all pure small probe core neighbours of the current voxel. pure means the voxel is the highest
    // level voxel that does not have mixed type
    std::vector<VoxelLoc> all_core_nbs = findPureNeighbours(vxl, mvTYPE_SP_CORE, false);
   
    // go through all neighbours
    for (const VoxelLoc& nb_loc : all_core_nbs){
      // get a reference to the neighbour voxel
      Voxel& nb_vxl = s_cell->getVxlFromGrid(nb_loc.index,nb_loc.lvl);
      if (nb_vxl.getID()){continue;} // skip processed voxels
      
      nb_vxl.setID(id);
      nb_vxl.passIDtoChildren(nb_loc.index, nb_loc.lvl);
      stack.pushBack(nb_loc, cavity_type? isInterfaceVxl(nb_loc) : false);
    }
    
    // increment interface count if we are entering interface mode
    if (!at_interface && stack.sizePriority()){
      n_interface++;
    }
    
    // determines whether we are at a core-outside interface
    at_interface = stack.sizePriority();
  }
  cavities.push_back(Cavity(id, n_interface));
  return true;
}

bool Voxel::isInterfaceVxl(const VoxelLoc& vxl){
  std::vector<VoxelLoc> outside_nbs = s_cell->getVxlFromGrid(vxl.index, vxl.lvl).findPureNeighbours(vxl, mvTYPE_LP_SHELL);
  // if (nbs.size()){return true;}
  for (const VoxelLoc& nb : outside_nbs) {
    if (readBit(s_cell->getVxlFromGrid(nb.index, nb.lvl).getType(),6)){
      return true;
    }
  }
  return false;
}

// returns a vector of all pure neighbours 
// contains duplicates due to ascend
std::vector<VoxelLoc> Voxel::findPureNeighbors(const VoxelLoc& central_vxl, const unsigned char type_flag, const bool any_id){
  return findPureNeighbours(central_vxl, type_flag, any_id);
}
std::vector<VoxelLoc> Voxel::findPureNeighbours(const VoxelLoc& central_vxl, const unsigned char type_flag, const bool any_id){
  // reusing SearchIndex to get a vector of all direct neighbour voxel indices, i.e.
  // (1,0,0); (1,0,1); (1,1,0), (1,1,1), etc.
  static const std::vector<std::vector<std::array<int,3>>> s_nb_indices = SearchIndex().computeIndices(3,false);
  
  std::vector<VoxelLoc> all_pure_nbs;
  std::array<unsigned,3> nb_index;
  for (const auto& shell : s_nb_indices){
    for (const auto& rel_index : shell){

      nb_index = add(central_vxl.index, rel_index);
      if (!s_cell->isInBounds(nb_index,central_vxl.lvl)){continue;}

      Voxel& nb_vxl = s_cell->getVxlFromGrid(nb_index,central_vxl.lvl);
      if (!any_id && nb_vxl.getID()){continue;} // greatly accelerates flood fill
      if (!(nb_vxl.getType() & type_flag)){continue;}

      if (nb_vxl.hasSubvoxel()){
        // descend to all subvoxels that border this voxel and add to vector
        nb_vxl.descend(all_pure_nbs, nb_index, central_vxl.lvl, rel_index, type_flag);
      }
      else {
        // ascend to highest parent of pure type and add to vector
        nb_vxl.ascend(all_pure_nbs, nb_index, central_vxl.lvl, central_vxl.index, rel_index);
      }
    }
  }
  return all_pure_nbs;
}

void Voxel::descend(std::vector<VoxelLoc>& all_pure_nbs, const std::array<unsigned,3>& index, const int lvl, const std::array<int,3>& nb_relation, const unsigned char type_flag){
  if (!hasSubvoxel()){
    if (getType() & type_flag){
      all_pure_nbs.push_back(VoxelLoc(index, lvl));
    }
  }
  else {
    // only loop over those voxels bordering the previous voxel
    std::array<unsigned,3> sub_index;

    // the subvoxels that need to be added to the flood fill stack are determined by the relation of the previous
    // voxel and the current voxel. the following block evaluates the relation to determine, which subvoxels to
    // loop through
    std::array<std::vector<char>,3> loop_i;
    for (char dim = 0; dim < 3; ++ dim){
      if (nb_relation[dim]){
        loop_i[dim] = {static_cast<char>((nb_relation[dim] > 0)? 0 : 1)};
      }
      else {
        loop_i[dim] = {0,1};
      }
    }

    for (char i : loop_i[0]){
      sub_index[0] = index[0] * 2 + i;
      for (char j : loop_i[1]){
        sub_index[1] = index[1] * 2 + j;
        for (char k : loop_i[2]){
          sub_index[2] = index[2] * 2 + k;
          s_cell->getVxlFromGrid(sub_index, lvl-1).descend(all_pure_nbs, sub_index, lvl-1, nb_relation, type_flag);
        }
      }
    }
  }
}

void Voxel::ascend(std::vector<VoxelLoc>& all_pure_nbs, const std::array<unsigned,3> index, const int lvl, std::array<unsigned,3> prev_index, const std::array<int,3>& nb_relation){
  // compare index with index of previous voxel
  // if voxel and previous voxel dont't belong to the same parent and this is not top lvl vxl
  // then compare types of this voxel and parent voxel

  bool same_vxl = false;
  if (s_cell->getMaxDepth() != lvl) {
    for (char dim = 0; dim < 3; ++dim){
      if (!nb_relation[dim]){
        same_vxl &= (index[dim]/2 == prev_index[dim]/2);
      }
    }
  }

  if (!same_vxl && lvl != s_cell->getMaxDepth()){
    std::array<unsigned,3> parent_index;
    for (char i = 0; i < 3; ++i){
      parent_index[i] = index[i]/2;
      prev_index[i] = prev_index[i]/2;
    }
    // access parent
    Voxel& parent = s_cell->getVxlFromGrid(parent_index, lvl+1);
    // if types are the same, then move to parent voxel
    if (parent.getType() == getType()){
      parent.ascend(all_pure_nbs, parent_index, lvl+1, prev_index, nb_relation);
      return;
    }
  }
  all_pure_nbs.push_back(VoxelLoc(index, lvl));
}

///////////////////////////////
// TYPE ASSIGNMENT 2ND ROUND //
///////////////////////////////

char Voxel::evalRelationToVoxels(const std::array<unsigned int,3>& index, const unsigned lvl, bool split){
  // if voxel (including all subvoxels) have been assigned, then return immediately
  if (Ctrl::getInstance()->getAbortFlag()){return 0;}
  if (isAssigned()){return _type;}
  else if (!hasSubvoxel()){ // vxl has no children
    split = !searchForCore(index, lvl, split);
  }
  if (hasSubvoxel()) { // vxl has children
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
  else {passTypeToChildren(index, lvl);}
  return _type;
}

bool Voxel::searchForCore(const std::array<unsigned int,3>& index, const unsigned lvl, bool split){
  // the return value of this function is used to determine, whether after splitting this voxel,
  // the subsequent neighbour search should start from 0 or from the safe limit. The use of this
  // return value allows avoiding calling a function to validate voxel coordinates (Space::isInBounds)
  // which, due to the number of times the function would have to be called, saves a lot of computations
  bool next_search_from_0 = false;
  _type = s_masking_mode? 0 : 0b00000101; // type excluded

  const char shell_type = s_masking_mode? 0b01000001 : 0b00010001;
  const char bit_pos_core = s_masking_mode? 5 : 3;

  for (unsigned int n = (split? Voxel::s_search_indices.getSafeLim(lvl+1)*4 : 1); n <= Voxel::s_search_indices.getUppLim(lvl); ++n){
    // called very often; keep section inexpensive
    for (std::array<int,3> coord : Voxel::s_search_indices[n]){
      coord = add(coord,index);
      // if a neighbour voxel containing a probe core is found
      if (readBit((s_cell->getVxlFromGrid(coord,lvl)).getType(),bit_pos_core)){
        Voxel& nb_vxl = s_cell->getVxlFromGrid(coord,lvl);
        // if the neighbour is within a safe distance
        if (n <= Voxel::s_search_indices.getSafeLim(lvl)){
          next_search_from_0 = true;
          setType(shell_type); // TODO: type is set only to be potentially reset
          if (!s_masking_mode && nb_vxl.getType() != 0b00001001){
            setType(0b10000000);
          }
          else {
            // voxel evaluation successful
            setID(nb_vxl.getID());
            passIDtoChildren(index, lvl);
          }
        }
        // if the neighbour is within a questionable distance
        else {
          next_search_from_0 = false;
          setType(0b10000000);
        }
        return next_search_from_0;
      }
    }
  }
  return next_search_from_0;
}

///////////
// TALLY //
///////////

void Voxel::tallyVoxelsOfType(std::map<char,double>& type_tally,
    std::map<unsigned char,double>& id_core_tally,
    std::map<unsigned char,double>& id_shell_tally,
    std::map<unsigned char,std::array<unsigned,3>>& id_min,
    std::map<unsigned char,std::array<unsigned,3>>& id_max,
    const std::array<unsigned,3>& index,
    const int lvl,
    const double vxl_fraction)
{
  // if voxel is of type "mixed" (i.e. data vector is not empty)
  if(hasSubvoxel()){
    // then total number of voxels is given by tallying all subvoxels
    std::array<unsigned,3> sub_index;
    for(char x = 0; x < 2; ++x){
      sub_index[0] = index[0]*2 + x;
      for(char y = 0; y < 2; ++y){
        sub_index[1] = index[1]*2 + y;
        for(char z = 0; z < 2; ++z){
          sub_index[2] = index[2]*2 + z;
          getSubvoxel(sub_index, lvl).tallyVoxelsOfType(
              type_tally, id_core_tally, id_shell_tally, id_min, id_max, sub_index, lvl-1, vxl_fraction);
        }
      }
    }
  }
  else {
    // tally number of bottom level voxels
    type_tally[getType()] += pow(pow2(lvl),3) * vxl_fraction;
    if(getType() == 0b00001001){
      id_core_tally[getID()] += pow(pow2(lvl),3) * vxl_fraction;
    }
    else if(getType() == 0b00010001){
      id_shell_tally[getID()] += pow(pow2(lvl),3) * vxl_fraction;
    }
    // localise cavities
    std::array<unsigned,3> min;
    std::array<unsigned,3> max;
    for (char i = 0; i < 3; i++){
      min[i] = index[i]*pow2(lvl);
      max[i] = ((index[i]+1)*pow2(lvl))-1;
    }
    if (id_min.count(getID()) == 0) {id_min[getID()] = min;}
    if (id_max.count(getID()) == 0) {id_max[getID()] = max;}
    for (char i = 0; i < 3; i++){
      if (id_min[getID()][i] > min[i]) {id_min[getID()][i] = min[i];}
      if (id_max[getID()][i] < max[i]) {id_max[getID()][i] = max[i];}
    }
  }
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

