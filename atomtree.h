#ifndef ATOMTREE_H

#define ATOMTREE_H

#include <vector>

struct Atom;
class AtomTree;
struct AtomNode{
  AtomNode(Atom* at, AtomNode* left_node, AtomNode* right_node); 
    
  Atom* atom; // using pointer so that original atom list can be kept and objects do not have to be copied
  AtomNode* left_child;
  AtomNode* right_child;

  void print();
};

struct Atom;
struct AtomNode;
class AtomTree{
  public:
    AtomTree();
    AtomTree(std::vector<Atom>& list_of_atoms);

    void print() const;

  private:
    AtomNode* root;
    
    AtomNode* buildTree(
        std::vector<Atom>& list_of_atoms, 
        int vec_first, // index of first vector element (default 0)
        int vec_end, // vector size (index of last element + 1)
        char dim);
  
    void quicksort(std::vector<Atom>& list_of_atoms, const int& vec_first, const int& vec_end, const char& dim);
    void swap(Atom& a, Atom& b);
};


#endif
