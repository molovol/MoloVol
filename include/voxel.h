#ifndef VOXEL_H

#define VOXEL_H

#include "vector.h"
#include "atomtree.h"
#include "container3d.h"
#include <vector>
#include <array>
#include <unordered_map>

struct SearchIndex{
  public:
    SearchIndex();
    SearchIndex(const double, const double, const unsigned int);
    const std::vector<std::array<int,3>>& operator[](unsigned int);
    unsigned int getUppLim(unsigned int);
    unsigned int getSafeLim(unsigned int);
    std::vector<std::vector<std::array<int,3>>> computeIndices(unsigned int);
  private:
    std::vector<std::vector<std::array<int,3>>> _index_list;
    std::vector<unsigned> _upp_lim;
    std::vector<unsigned> _safe_lim;
};

class Space;
class AtomTree;
struct Atom;
struct AtomNode;
struct VoxelLoc;
class Voxel{
  public:
    Voxel();

    Voxel& getSubvoxel(std::array<unsigned,3>, const unsigned, const std::array<char,3>&);
    Voxel& getSubvoxel(std::array<unsigned,3>, const unsigned, const char);
    Voxel& getSubvoxel(std::array<unsigned,3>, const unsigned);
    void setType(char);
    char getType();
    void setID(unsigned char);
    unsigned char getID();
    
    // bitwise operations on _type
    bool hasSubvoxel(); // state of bit 7
    bool isCore(); // state of bit 3
    bool isAssigned(); // state of bit 0

    // calc preparation
    static void prepareTypeAssignment(Space*, AtomTree);
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
    bool floodFill(const unsigned char, const std::array<unsigned,3>&, const int);

    // shell vs void
    char evalRelationToVoxels(const std::array<unsigned int,3>&, const unsigned, bool=false);
    
    // unused
    static void listFromTree(
        std::vector<int>&,
        const AtomNode*,
        const Vector&,
        const double&,
        const double&,
        const double&,
        const char=0);
    
    // volume
    unsigned int tallyVoxelsOfType(const std::array<unsigned,3>&, const char volume_type, const int max_depth);

  private:
    char _type;
    unsigned char _identity;

    static inline Space* s_cell;
    // atom vs core
    static inline AtomTree s_atomtree;
    // shell vs void
    static inline double s_r_probe;
    static inline bool s_masking_mode;
    static inline SearchIndex s_search_indices;

    static inline double calcRadiusOfInfluence(const double& max_depth);

    // atom vs core
    bool isAtom(const Atom&, const Vector&, const double, const double);
    // cavity id
    void descend(std::vector<VoxelLoc>&, const unsigned char, const std::array<unsigned,3>&, const int, const signed char, const bool);
    void ascend(std::vector<VoxelLoc>&, const unsigned char, const std::array<unsigned,3>, const int, std::array<unsigned,3>, const signed char);
    void passIDtoChildren(const std::array<unsigned,3>&, const int);
    // shell vs void
    void searchForCore(const std::array<unsigned int,3>&, const unsigned, bool=false);
};

#endif
