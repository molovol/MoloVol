#ifndef ATOMTREE_H

#define ATOMTREE_H

#include <vector>
#include "atom.h"

struct Atom;
class AtomTree;
class AtomNode{
  public:
    AtomNode(int, AtomNode* left_node, AtomNode* right_node);
    ~AtomNode();
 
    AtomNode* getLeftChild() const;
    AtomNode* getRightChild() const;
    Atom& getAtom() const;
    int getAtomId() const;
    void print();

    static Atom& getAtom(const int);
    static void setAtomList(const std::vector<Atom>&);
    static std::vector<Atom>& getAtomList();
  private:
    AtomNode* _left_child;
    AtomNode* _right_child;
    int _atom_id;
    static inline std::vector<Atom> s_atom_list;
};

class AtomNode;
class AtomTree{
  public:
    AtomTree();
    AtomTree(const std::vector<Atom>& list_of_atoms);
    ~AtomTree();
    
    const AtomNode* getRoot() const;
    double getMaxRad() const;
    void print() const;
  private:
    AtomNode* _root;
    double _max_rad;
    
    AtomNode* buildTree(int, int, char);
  
    void quicksort(std::vector<Atom>& list_of_atoms, const int& vec_first, const int& vec_end, const char& dim);
    void swap(Atom& a, Atom& b);
};

#endif
