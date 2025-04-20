
#include "atomtree.h"
#include "misc.h"
#include <cmath>

///////////////////
// AUX FUNCTIONS //
///////////////////

double findMaxRad(std::vector<Atom>& list_of_atoms);

//////////////
// ATOMNODE //
//////////////

// CONSTRUCTOR

AtomNode::AtomNode(AtomTree* tree, size_t atom_id, AtomNode* left_node, AtomNode* right_node) 
  : _parent_tree(tree), _left_child(left_node), _right_child(right_node), _atom_id(atom_id) {}

// DESTRUCTOR

AtomNode::~AtomNode(){
  delete _left_child;
  delete _right_child;
}

// ACCESS

const AtomNode* AtomNode::getLeftChild() const {
  return _left_child;
}

const AtomNode* AtomNode::getRightChild() const {
  return _right_child;
}

AtomNode* AtomNode::getLeftChild(){
  return _left_child;
}

AtomNode* AtomNode::getRightChild(){
  return _right_child;
}

Atom& AtomNode::getAtom() const {
  return _parent_tree->getAtomList()[_atom_id];
}

size_t AtomNode::getAtomId() const {
  return _atom_id;
}

// OTHER
void AtomNode::print(){
  std::cout << getAtom().symbol << "("
    << getAtom().getCoordinate(0) << ","
    << getAtom().getCoordinate(1) << ","
    << getAtom().getCoordinate(2) << ")";

  std::cout << "(-";
  if(getLeftChild() != NULL){
    getLeftChild()->print();
  }
  std::cout << " +";
  if(getRightChild() != NULL){
    getRightChild()->print();
  }
  std::cout << ")";
  return;
}

//////////////
// ATOMTREE //
//////////////

// CONSTRUCTOR

AtomTree::AtomTree(){
  _root = NULL;
  _max_rad = 0;
}

AtomTree::AtomTree(const std::vector<Atom>& list_of_atoms) : _atom_list(list_of_atoms){
  _root = buildTree(0, getAtomList().size(), 0);
  // ideally, the maximum radius would be the largest radius among all children of a node.
  // this, however, may require running an algorithm for every tree node, increasing the
  // complexity of the operation. it is much simpler to use the maximum radius among all
  // atoms instead, sacrificing optimisation.
  _max_rad = findMaxRad(getAtomList());
}

// DESTRUCTOR

AtomTree::~AtomTree(){
  delete _root;
}

// FUNCTIONS USED BY CONSTRUCTOR

// recursive function to generate a 3-d tree from a list of atoms
// rather than copying the list of atoms in every recursion, or saving the partitioned
// vectors, the original list of atoms is passed by reference and only the vector limits
// are passed by value. any sorting during the tree building occurs in the original list
AtomNode* AtomTree::buildTree(
    size_t vec_first, // index of first vector element (default 0)
    size_t vec_end, // vector size (index of last element + 1)
    char dim){
  // if list of atoms has no atoms left
  if((vec_end-vec_first)==0){
    return NULL;
  }
  // if list of atoms has exactly one atom left
  else if((vec_end-vec_first)==1){
    return new AtomNode(this, vec_first,NULL,NULL);
  }

  else{
    quicksort(vec_first, vec_end, dim);
    size_t median = vec_first + (vec_end-vec_first)/2; // operation rounds down
    return new AtomNode(this, median, 
        buildTree(vec_first, median, (dim+1)%3), 
        buildTree(median+1, vec_end, (dim+1)%3));
  }
}

double findMaxRad(std::vector<Atom>& list_of_atoms){
  double max_rad = 0;
  for (size_t atom = 0; atom < list_of_atoms.size(); atom++){
    if (list_of_atoms[atom].getRad() > max_rad){
      max_rad = list_of_atoms[atom].getRad();
    }
  }
  return max_rad;
}


void AtomTree::quicksort(const size_t vec_first, const size_t vec_end, const char dim){

  if(vec_first+1 >= vec_end){
    return;
  }

  double pivot = _atom_list[vec_end-1].getCoordinate(dim);

  int cntr = vec_first;

  for(int i = vec_first; i < vec_end; i++){
    if(_atom_list[i].getCoordinate(dim) <= pivot){
      swap(_atom_list[cntr], _atom_list[i]);
      cntr++;
    }
  }
  quicksort(vec_first, cntr-1, dim);
  quicksort(cntr, vec_end, dim);

  return;
}

void AtomTree::swap(Atom& a, Atom& b){
  Atom temp = a;
  a = b;
  b = temp;
  return;
}

// for testing
void AtomTree::print() const {
  std::cout << "Printing Tree" << std::endl;
  if(_root == NULL){
    std::cout << "Tree empty" << std::endl;
  }
  else{
    _root->print();
  }
  std::cout << std::endl;
  return;
}

// ACCESS

const std::vector<Atom>& AtomTree::getAtomList() const {
  return _atom_list;
}

std::vector<Atom>& AtomTree::getAtomList() {
  return _atom_list;
}

const double AtomTree::getMaxRad() const {
  return _max_rad;
}

const AtomNode* AtomTree::getRoot() const {
  return _root;
}

// Returns a vector containing atom IDs of all atoms whose distance from the atom's center
// is equal or below a specified maximal distance + the radius of the atom.
// Can be used to find all atoms that are touching or intersecting a sphere.
std::vector<size_t> AtomTree::listAllWithin(const typename AtomTree::pos_type pos, const double max_dist) const {
  std::vector<size_t> id_list;

  std::vector<std::pair<AtomNode*,char>> to_visit;
  to_visit.push_back(std::make_pair(_root,0));
  
  while (!to_visit.empty()) {
    auto node_dim = to_visit.back();
    to_visit.pop_back();
    // Skip if nullptr
    if (node_dim.first == nullptr) continue;
    AtomNode& node = *node_dim.first;
    char dim = node_dim.second;

    num_type at_pos_dim = node.getAtom().getCoordinate(dim);
    num_type dist1D = pos[dim] - at_pos_dim;

    // If distance along current direction is smaller than threshold then check
    // this node's atom for match and visit both children.
    if (abs(dist1D) <= max_dist + _max_rad) {

      if (distance(node.getAtom().getPos(), pos) <= max_dist + node.getAtom().rad) {
        id_list.push_back(node.getAtomId());
      }

      to_visit.push_back(std::make_pair(node.getLeftChild(), (dim+1)%3));
      to_visit.push_back(std::make_pair(node.getRightChild(), (dim+1)%3));
    }
    // If distance is larger then visit either left or right child, depending on
    // relation of the atom and the point.
    else {
      to_visit.push_back(std::make_pair(
            dist1D < 0? node.getLeftChild() : node.getRightChild(),
            (dim+1)%3));
    }
  }

  return id_list;
}

