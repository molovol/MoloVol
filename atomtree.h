#ifndef ATOM_H

#define ATOM_H

#include "atom.h"
#include <vector>
#include <cmath> 

class AtomTree{
  public:
    AtomTree(Atom* at, AtomTree left_node, AtomTree right_node) 
      : atom(at), left_child(left_node), right_child(right_node) {}
    
    Atom* atom = NULL;
    
    //TODO add exception for empty node
    AtomTree getLeftChild(){
      return left_child;
    }
    AtomTree getRightChild(){
      return right_child;
    }
    
    // recursive function to generate a 3-d tree from a list of atoms
    // rather than copying the list of atoms in every recursion, or saving the partitioned
    // vectors, the original list of atoms is passed by reference and only the vector limits
    // are passed by value. any sorting during the tree building occurs in the original list
    AtomTree buildTree(
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
        return AtomTree(&list_of_atoms[vec_first],NULL,NULL);
      }

      else{
        sort(list_of_atoms, vec_first, vec_end, dim);
        int median = (vec_end-vec_first)/2; // operation rounds down
        return AtomTree(
            &list_of_atoms[median], 
            buildTree(list_of_atoms, vec_first, median, (dim%3)+1), 
            buildTree(list_of_atoms, median+1, vec_end, (dim%3)+1)); 
      }
    }
  private:
    AtomTree left_child = NULL;
    AtomTree right_child = NULL;
    
    void sort(std::vector<Atom>& list_of_atoms, const int& vec_first, const int& vec_end, const char& dim){
      // TODO  
      return;
    }
};


#endif
