#include "atom.h"
#include "atomtree.h"
#include <vector>
#include <iostream>
#include <map>

// Using this macro for future compatibility with Catch2
# define REQUIRE(x) if (!x) return -1;

std::vector<Atom> isobutane();
//std::vector<Atom> protein_6s8y();

int main() {

  auto atomlist = isobutane();
  const AtomTree atomtree(atomlist);
 
  // TEST: Check correct radius access
  if(atomtree.getMaxRad() != 1.7) return -1;

  // TEST: Check correct atom list access
  // The atom list inside atom tree does not have to and likely does not
  // store the atoms in the same order as the originally input list.
  {
    auto atomlist_from_tree = atomtree.getAtomList();
    bool all_atoms_found = true;
    for (const Atom& at : atomlist_from_tree) {
      bool atom_found = false;
      for (const Atom& at2 : atomlist) {
        atom_found |= at == at2; // Looking for single match
      }
      all_atoms_found &= atom_found; // All atoms need to be found
    }
    REQUIRE(all_atoms_found);
  }

  // TEST: Verify k-d tree
  // In a k-d tree the nodes are inserted based on their spatial relation.
  // For any given node, all descendents of its left child are located "below"
  // or "left" of the parent node. Likewise, all descendents of its right child
  // are "above" or "right" of the parent node. The axis along which these nodes
  // are arranged depends on the depth and alternates cyclically as the depth
  // increases. In this 3-d tree the first axis is the x-axis and is followed by
  // the y- then z-axis.
  // This test checks the spatial relation between the left and right child nodes.
  {
    std::vector<const AtomNode*> treenodes;
    std::vector<unsigned char> depth;

    treenodes.push_back(atomtree.getRoot());
    depth.push_back(0);

    while (!treenodes.empty()) {
      const AtomNode* node = treenodes[treenodes.size()-1];
      treenodes.pop_back();
      unsigned char d = depth[depth.size()-1];
      depth.pop_back();
      const AtomNode* left = node->getLeftChild();
      const AtomNode* right = node->getRightChild();

      if (left && right) {
        if (!(left->getAtom().getCoordinate(d%3) <= right->getAtom().getCoordinate(d%3))) return -1;

        treenodes.push_back(left);
        treenodes.push_back(right);
        depth.push_back(d+1);
        depth.push_back(d+1);
      }
    }
  }

  std::map<std::string,int> valence = {
    {"C", 4},
    {"H", 1},
    {"N", 3},
    {"O", 2}
  };

  // TEST: Bond detection
  // First search the atom tree for all atoms within their van der Waals radius.
  // The number of matches should be equal to the valence+1 because the atom counts
  // itself as a match.
  {
    auto all_atoms = atomtree.getAtomList();
    for (size_t at_id = 0; at_id < all_atoms.size(); ++at_id) {
      const Atom& at = all_atoms[at_id];
      std::vector<size_t> closest = atomtree.listAllWithin(at.getPos(), 0);
      if (!(closest.size()-1 == valence.at(at.symbol))) return -1;
    }
  }
}

std::vector<Atom> isobutane() {
  // Atom(#x, #y, #z, #symbol, #radius, #atnum, #charge) 

  double radius_c = 1.7;
  double radius_h = 1.2;

  // Define isobutane
  double xpos_c[4] = {-7.945000, -9.269000, -8.208000, -7.114000};
  double ypos_c[4] = {-0.100000, -0.881000, 1.414000, -0.482000};
  double zpos_c[4] = {-0.336000, -0.362000, -0.366000, 0.899000};
  
  double xpos_h[10] = {
    -9.892000,-9.865000,-9.087000,-7.359000,-7.253000,
    -8.784000,-8.790000,-6.139000,-6.893000,-7.654000
  };
  
  double ypos_h[10] = {
    -0.646000,-0.632000,-1.980000,-0.372000, 1.988000,
     1.705000, 1.739000, 0.057000,-1.574000,-0.231000
  };
  
  double zpos_h[10] = {
    0.530000,-1.270000,-0.371000,-1.251000,-0.377000,
    -1.274000, 0.526000, 0.911000, 0.913000, 1.840000
  };

  std::vector<Atom> at_vec;
  for (size_t i = 0; i < 4; ++i) {
    at_vec.push_back(Atom(xpos_c[i], ypos_c[i], zpos_c[i], "C", radius_c, 6, 0));
  }

  for (size_t i = 0; i < 10; ++i) {
    at_vec.push_back(Atom(xpos_h[i], ypos_h[i], zpos_h[i], "H", radius_h, 1, 0));
  }

  return at_vec;
}