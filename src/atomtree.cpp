
#include "atomtree.h"
#include "misc.h"
#include <cmath>

///////////////////
// AUX FUNCTIONS //
///////////////////

double findMaxRad(std::vector<Atom>& list_of_atoms);
void findAdjacentRecursive(std::vector<Atom*>&, const Atom&, const double& shell_to_shell_dist, const double& min_distance, const AtomNode* node, int dim);

//////////////
// ATOMNODE //
//////////////

// CONSTRUCTOR

AtomNode::AtomNode(int atom_id, AtomNode* left_node, AtomNode* right_node) : _left_child(left_node), _right_child(right_node), _atom_id(atom_id) {}

// DESTRUCTOR

AtomNode::~AtomNode(){
  delete _left_child;
  delete _right_child;
}

// ACCESS

AtomNode* AtomNode::getLeftChild() const {
  return _left_child;
}

AtomNode* AtomNode::getRightChild() const {
  return _right_child;
}

void AtomNode::setAtomList(const std::vector<Atom>& atom_list){
  AtomNode::s_atom_list = atom_list;
}

std::vector<Atom>& AtomNode::getAtomList(){
  return s_atom_list;
}

Atom& AtomNode::getAtom() const {
  return AtomNode::getAtom(_atom_id);
}

Atom& AtomNode::getAtom(const int atom_id){
  return s_atom_list[atom_id];
}

int AtomNode::getAtomId() const {
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

AtomTree::AtomTree(const std::vector<Atom>& list_of_atoms){
  AtomNode::setAtomList(list_of_atoms);
  _root = buildTree(0, AtomNode::getAtomList().size(), 0);
  // ideally, the maximum radius would be the largest radius among all children of a node.
  // this, however, may require running an algorithm for every tree node, increasing the
  // complexity of the operation. it is much simpler to use the maximum radius among all
  // atoms instead, sacrificing optimisation.
  _max_rad = findMaxRad(AtomNode::getAtomList());
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
    int vec_first, // index of first vector element (default 0)
    int vec_end, // vector size (index of last element + 1)
    char dim){
  // if list of atoms has no atoms left
  if((vec_end-vec_first)==0){
    return NULL;
  }
  // if list of atoms has exactly one atom left
  else if((vec_end-vec_first)==1){
    return new AtomNode(vec_first,NULL,NULL);
  }

  else{
    quicksort(AtomNode::getAtomList(), vec_first, vec_end, dim);
    int median = vec_first + (vec_end-vec_first)/2; // operation rounds down
    return new AtomNode(median, buildTree(vec_first, median, (dim+1)%3), buildTree(median+1, vec_end, (dim+1)%3));
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


void AtomTree::quicksort(std::vector<Atom>& list_of_atoms, const int vec_first, const int vec_end, const char dim){

  if(vec_first >= vec_end-1){
    return;
  }

  double pivot = list_of_atoms[vec_end-1].getCoordinate(dim);

  int cntr = vec_first;

  for(int i = vec_first; i < vec_end; i++){
    if(list_of_atoms[i].getCoordinate(dim) <= pivot){
      swap(list_of_atoms[cntr], list_of_atoms[i]);
      cntr++;
    }
  }
  quicksort(list_of_atoms, vec_first, cntr-1, dim);
  quicksort(list_of_atoms, cntr, vec_end, dim);

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

const double AtomTree::getMaxRad() const {
  return _max_rad;
}

const AtomNode* AtomTree::getRoot() const {
  return _root;
}
