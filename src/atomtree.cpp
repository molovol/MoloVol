
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

// Access node via a specified path. The path is encoded in a string
// The string contains either the letter 'l' or 'r' in every
// position, specifying to take either the path towards the left or
// right child respectively.
const AtomNode* AtomTree::getNode(const std::string path) const {
  const AtomNode* node = _root;
  for (size_t i = 0; i < path.length(); ++i) {
    if (path[i] == 'l') {
      node = node->getLeftChild();
    } else if (path[i] == 'r') {
      node = node->getRightChild();
    } else {
//      throw std::exception;
    }
  }
  return node;
}

