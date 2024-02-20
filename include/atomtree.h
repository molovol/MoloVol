#ifndef ATOMTREE_H

#define ATOMTREE_H

#include "atom.h"
#include <vector>
#include <string>

class AtomTree;
class AtomNode{
  public:
    AtomNode(AtomTree*, size_t, AtomNode* left_node, AtomNode* right_node);
    ~AtomNode();
 
    const AtomNode* getLeftChild() const;
    const AtomNode* getRightChild() const;
    AtomNode* getLeftChild();
    AtomNode* getRightChild();
    Atom& getAtom() const;
    size_t getAtomId() const;

    void print();
  private:
    // TODO: Reevaluate whether each node needs to know their parent tree. This
    // is only used for accessing atoms from nodes directly which is only used
    // by the voxel class. Atom access could go through atom tree directly
    AtomTree* _parent_tree;
    AtomNode* _left_child;
    AtomNode* _right_child;
    size_t _atom_id;
};

class AtomTree{
  public:
    friend AtomNode;
      
    AtomTree();
    AtomTree(const std::vector<Atom>& list_of_atoms);
    ~AtomTree();
    // TODO: Move and copy operators
    
    const AtomNode* getRoot() const;
    const AtomNode* getNode(const std::string) const;

    const double getMaxRad() const;
    const std::vector<Atom>& getAtomList() const;

    void print() const;
  private:
    AtomNode* _root;
    double _max_rad;
    std::vector<Atom> _atom_list;
    
    AtomNode* buildTree(size_t, size_t, char);
  
    std::vector<Atom>& getAtomList();
    void quicksort(const size_t, const size_t, const char);
    void swap(Atom&, Atom&);
};

#endif
