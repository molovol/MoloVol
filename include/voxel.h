#ifndef VOXEL_H

#define VOXEL_H

#include "vector.h"
#include "atomtree.h"
#include <vector>
#include <array>

class AtomTree;
struct Atom;
struct AtomNode;
class Voxel{
  public:
    Voxel();
    Voxel& access(const short& x, const short& y, const short& z);
    Voxel& access(const short& i);
    void setType(char);
    char getType();
    
    static void storeUniversal(AtomTree, double, double);

    char determineType(
        std::array<double,3> pos,
        const double max_depth);
    
    void determineTypeSingleAtom(
        const Atom& atom, 
        std::array<double,3> pos, // voxel centre
        const double max_depth,
        bool&);
  /* 
    void traverseTree(
        const AtomNode* node, 
        int dim, 
        const double& at_rad, 
        const double& vxl_rad, 
        const std::array<double,3> vxl_pos,
        const double& max_depth,
        bool&);
    */
    std::vector<Atom> listFromTree(
        const AtomNode*, 
        const std::array<double,3>&, 
        const double&,
        const double&,
        const double&,
        const char);

    size_t tallyVoxelsOfType(const char volume_type, const int max_depth);

    void splitVoxel(
        const std::array<double,3>& vxl_pos, 
        const double& max_depth); 
  private:

    static inline AtomTree _atomtree;
    static inline double _grid_size;
    static inline double _r_probe1;

    inline double calcRadiusOfInfluence(const double& max_depth);

    bool isAtom(const Atom& atom, const double& dist_vxl_at, const double& radius_of_influence); // inline not faster
    bool isProbeExcluded(const std::array<double,3>& vxl_pos, const double& r_probe, const double&, const std::vector<Atom>&);
    bool isExcludedByPair(const Vector&, const Vector&, const double&, const double&, const double&, const double&);
    bool isExcludedByTriplet(const Vector&, const double&, const std::array<Vector,4>&, const std::array<double,4>&, const double&);
    bool isExcludedByQuadruplet(const Vector&, const double&, const std::array<Vector,4>&, const std::array<double,4>&, const double&);
    bool isExcludedSetType(const Vector&, const double&, const Vector&, const double&);

    std::vector<Voxel> data; // empty or exactly 8 elements
    char type;
    // types
    // 'a' : inside of atom
    // 'e' : empty
    // 'm' : mixed
};

#endif
