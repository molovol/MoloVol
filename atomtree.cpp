
#include "atomtree.h"
#include "atom.h"

//////////////
// ATOMNODE //
//////////////

AtomNode::AtomNode(Atom* at, AtomNode* left_node, AtomNode* right_node) 
    : atom(at), left_child(left_node), right_child(right_node) {}

void AtomNode::print(){
  if(atom!=NULL){
    std::cout << atom->symbol;
  }
  else{
    std::cout << "n/A";
  }

  std::cout << "(-";
  if(left_child!=NULL){
    left_child->print();
  }
  std::cout << " +";
  if(right_child!=NULL){
    right_child->print();
  }
  std::cout << ")";
  return;
}

//////////////
// ATOMTREE //
//////////////

AtomTree::AtomTree(){
  root = NULL;
}

AtomTree::AtomTree(std::vector<Atom>& list_of_atoms){
  root = buildTree(list_of_atoms, 0, list_of_atoms.size(), 0);
}

    // recursive function to generate a 3-d tree from a list of atoms
    // rather than copying the list of atoms in every recursion, or saving the partitioned
    // vectors, the original list of atoms is passed by reference and only the vector limits
    // are passed by value. any sorting during the tree building occurs in the original list
AtomNode* AtomTree::buildTree(
    std::vector<Atom>& list_of_atoms, 
    int vec_first, // index of first vector element (default 0)
    int vec_end, // vector size (index of last element + 1)
    char dim){
  // if list of atoms has no atoms left
  if((vec_end-vec_first)==0){
    return NULL;
  }
  // if list of atoms has exactly one atom left
  else if((vec_end-vec_first)==1){
    return new AtomNode(&list_of_atoms[vec_first],NULL,NULL);
  }

  else{
    sort(list_of_atoms, vec_first, vec_end, dim);
    int median = (vec_end-vec_first)/2; // operation rounds down
    return new AtomNode(
        &list_of_atoms[median], 
        buildTree(list_of_atoms, vec_first, median, (dim%3)+1), 
        buildTree(list_of_atoms, median+1, vec_end, (dim%3)+1)); 
  }
}

void AtomTree::print(){
  std::cout << "Printing Tree" << std::endl;
  if(root == NULL){
    std::cout << "Tree empty" << std::endl;
  }
  else{
    root->print();
  }
  std::cout << std::endl;
  return;
}
    
void AtomTree::sort(std::vector<Atom>& list_of_atoms, const int& vec_first, const int& vec_end, const char& dim){
  // TODO  
  return;
}

