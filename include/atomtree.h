#ifndef ATOMTREE_H

#define ATOMTREE_H

#include <vector>

struct Atom;
class AtomTree;
struct AtomNode{
  AtomNode(Atom* at, AtomNode* left_node, AtomNode* right_node); 
    
  Atom* atom; // using pointer so that original atom list can be kept and objects do not have to be copied
  // using pointers, so that children can be assigned to null pointers
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
    
    const AtomNode* getRoot() const;

    const double getMaxRad() const;
    void print() const;

    std::vector<Atom*> findAdjacent(const Atom&, const double&);
  private:
    AtomNode* root;
    double max_rad;

    AtomNode* buildTree(
        std::vector<Atom>& list_of_atoms, 
        int vec_first, // index of first vector element (default 0)
        int vec_end, // vector size (index of last element + 1)
        char dim);
  
    void quicksort(std::vector<Atom>& list_of_atoms, const int& vec_first, const int& vec_end, const char& dim);
    void swap(Atom& a, Atom& b);
};


#endif
