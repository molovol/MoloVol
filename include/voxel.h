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
class Voxel{
  public:
    Voxel();

    Voxel& getSubvoxel(std::array<unsigned,3>, const unsigned, const std::array<char,3>&);
    Voxel& getSubvoxel(std::array<unsigned,3>, const unsigned, const char);
    Voxel& getSubvoxel(std::array<unsigned,3>, const unsigned);
    bool hasSubvoxel();
    bool isAssigned();
    void setType(char);
    char getType();

    static void prepareTypeAssignment(Space*, AtomTree);
    static void storeProbe(const double);
    static void computeIndices();
    static void computeIndices(unsigned int);

    char evalRelationToAtoms(const std::array<unsigned,3>&, Vector, const int);
    void traverseTree(const AtomNode*, const double&, const Vector&, const double&, const double&, const int&, 
        const char = 0b00000011, const char = 0); 
    void passTypeToChildren(const std::array<unsigned,3>&, const int);
    void splitVoxel(const std::array<unsigned,3>&, const Vector&, const double); 
    void splitVoxel(const std::array<unsigned int,3>&, const unsigned);

    char evalRelationToVoxels(const std::array<unsigned int,3>&, const unsigned, bool=false);
    
    static void listFromTree(
        std::vector<int>&,
        const AtomNode*,
        const Vector&,
        const double&,
        const double&,
        const double&,
        const char=0);
    
    unsigned int tallyVoxelsOfType(const std::array<unsigned,3>&, const char volume_type, const int max_depth);

  private:
    static inline Space* s_cell;
    static inline AtomTree s_atomtree;
    static inline double s_grid_size;
    static inline double s_r_probe;
    static inline SearchIndex s_search_indices;

    static inline double calcRadiusOfInfluence(const double& max_depth);

    bool isAtom(const Atom&, const Vector&, const double, const double);
    void searchForCore(const std::array<unsigned int,3>&, const unsigned, bool=false);

    char _type;
};

#endif
