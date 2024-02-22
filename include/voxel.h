#ifndef VOXEL_H

#define VOXEL_H

#include "vector.h"
#include "atomtree.h"
#include "container3d.h"
#include "flags.h"
#include "cavity.h"
#include <vector>
#include <array>
#include <unordered_map>
#include <map>

struct SearchIndex{
  public:
    SearchIndex();
    SearchIndex(const double, const double, const unsigned int);
    const std::vector<std::array<int,3>>& operator[](unsigned int);
    unsigned int getUppLim(unsigned int);
    unsigned int getSafeLim(unsigned int);
    std::vector<std::vector<std::array<int,3>>> computeIndices(unsigned int);
    std::vector<std::vector<std::array<int,3>>> computeIndices(unsigned int, const bool);
  private:
    std::vector<std::vector<std::array<int,3>>> _index_list;
    std::vector<unsigned> _upp_lim;
    std::vector<unsigned> _safe_lim;
};

class Space;
class FloodStack;
struct Atom;
struct VoxelLoc;
class Voxel{
  public:
    Voxel();

    Voxel& getSubvoxel(std::array<unsigned,3>, const unsigned, const std::array<char,3>&);
    Voxel& getSubvoxel(std::array<unsigned,3>, const unsigned, const char);
    Voxel& getSubvoxel(std::array<unsigned,3>, const unsigned);
    void setType(char);
    char getType() const;
    void setID(unsigned char);
    unsigned char getID() const;

    // bitwise operations on _type
    bool hasSubvoxel(); // state of bit 7
    bool isCore(); // state of bit 3
    bool isAssigned(); // state of bit 0

    // calc preparation
    static void prepareTypeAssignment(Space*, std::vector<Atom>&);
    static void storeProbe(const double, const bool);
    static void computeIndices();
    static void computeIndices(unsigned int);

    // atom vs probe core
    char evalRelationToAtoms(const std::array<unsigned,3>&, Vector, const int);
    void traverseTree(const AtomNode*, const double, const Vector&, const double, const double, const int,
        const char = 0b00000011, const char = 0);
    void passTypeToChildren(const std::array<unsigned,3>&, const int);
    void splitVoxel(const std::array<unsigned,3>&, const Vector&, const double);

    // cavity id
    bool floodFill(std::vector<Cavity>&, const unsigned char, const std::array<unsigned,3>&, const int, const bool=false);

    // shell vs void
    char evalRelationToVoxels(const std::array<unsigned int,3>&, const unsigned, bool=false);

    // volume
    void tallyVoxelsOfType(std::map<char,double>&,
        std::map<unsigned char,double>&,
        std::map<unsigned char,double>&,
        std::map<unsigned char,std::array<unsigned,3>>&,
        std::map<unsigned char,std::array<unsigned,3>>&,
        const std::array<unsigned,3>&,
        const int,
        const double=1);

    // unused but could become useful
    static void listFromTree(std::vector<int>&, const AtomNode*, const Vector&, 
        const double&, const double&, const double&, const char=0);
    static const AtomTree& getAtomTree() {
      return *s_atomtree;
    }
  private:
    char _type;
    unsigned char _identity;

    static inline Space* s_cell; // gets destroyed by Model
    // atom vs core
    static inline AtomTree* s_atomtree;
    // shell vs void
    static inline double s_r_probe;
    static inline bool s_masking_mode;
    static inline SearchIndex s_search_indices;

    static inline double calcVxlRadius(const double& max_depth);

    // atom vs core
    bool isAtom(const Atom&, const Vector&, const double, const double);
    // cavity id
    bool isInterfaceVxl(const VoxelLoc&);
    std::vector<VoxelLoc> findPureNeighbours(const VoxelLoc&, const unsigned char=mvTYPE_ALL, const bool=false);
    std::vector<VoxelLoc> findPureNeighbors(const VoxelLoc&, const unsigned char=mvTYPE_ALL, const bool=false);
    void descend(std::vector<VoxelLoc>&, const std::array<unsigned,3>&, 
        const int, const std::array<int,3>&, const unsigned char);
    void ascend(std::vector<VoxelLoc>&, const std::array<unsigned,3>, 
        const int, std::array<unsigned,3>, const std::array<int,3>&);
    void passIDtoChildren(const std::array<unsigned,3>&, const int);
    // shell vs void
    bool searchForCore(const std::array<unsigned int,3>&, const unsigned, bool=false);
};

#endif
