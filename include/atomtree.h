#ifndef ATOMTREE_H

#define ATOMTREE_H

#include <vector>

struct Atom;
class AtomTree;
class AtomNode{
  public:
    AtomNode(int, AtomNode* left_node, AtomNode* right_node);
  
    // TODO: move children to private
    AtomNode* left_child;
    AtomNode* right_child;
  
    Atom& getAtom() const;
    static Atom& getAtom(const int);
    int getAtomId() const;
    void print(); // used for testing
    static void setAtomList(const std::vector<Atom>&);
    static std::vector<Atom>& getAtomList();
  private:
    int _atom_id;
    static inline std::vector<Atom> _atom_list;
};

struct Atom;
class AtomNode;
class AtomTree{
  public:
    AtomTree();
    AtomTree(std::vector<Atom>& list_of_atoms);
    
    const AtomNode* getRoot() const;
    const double getMaxRad() const;
    void print() const;

    std::vector<Atom*> findAdjacent(const Atom&, const double&); // may not be needed
  private:
    AtomNode* _root;
    double _max_rad;
    
    AtomNode* buildTree(int, int, char);
  
    void quicksort(std::vector<Atom>& list_of_atoms, const int& vec_first, const int& vec_end, const char& dim);
    void swap(Atom& a, Atom& b);
};

#endif
