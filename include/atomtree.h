#ifndef ATOMTREE_H

#define ATOMTREE_H

#include "atom.h"
#include <vector>
#include <string>

class AtomTree;
class AtomNode{
  public:
    AtomNode(int, AtomNode* left_node, AtomNode* right_node);
    ~AtomNode();
 
    const AtomNode* getLeftChild() const;
    const AtomNode* getRightChild() const;
    AtomNode* getLeftChild();
    AtomNode* getRightChild();
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

class AtomTree{
  public:
    AtomTree();
    AtomTree(const std::vector<Atom>& list_of_atoms);
    ~AtomTree();
    
    const AtomNode* getRoot() const;
    const AtomNode* getNode(const std::string) const;
    const double getMaxRad() const;
    void print() const;
  private:
    AtomNode* _root;
    double _max_rad;
    
    AtomNode* buildTree(int, int, char);
  
    void quicksort(std::vector<Atom>&, const int, const int, const char);
    void swap(Atom&, Atom&);
};

#endif
