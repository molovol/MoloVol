#ifndef VOXEL_H

#define VOXEL_H

#include "vector.h"
#include "atomtree.h"
#include "container3d.h"
#include <vector>
#include <array>
#include <unordered_map>

struct PairBundle {
  PairBundle(){}
  PairBundle(Vector unitvec_parallel, double probe_parallel, double probe_orthogonal)
    : unitvec_parallel(unitvec_parallel), probe_parallel(probe_parallel), probe_orthogonal(probe_orthogonal){}

  Vector unitvec_parallel;
  double probe_parallel;
  double probe_orthogonal;
};

struct TripletBundle {
  TripletBundle(){}
  TripletBundle(Vector vec_probe_plane, Vector vec_probe_normal)
    : vec_probe_plane(vec_probe_plane), vec_probe_normal(vec_probe_normal){}

  Vector vec_probe_plane;
  Vector vec_probe_normal;
};

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

    static void storeUniversal(AtomTree, double, double, int);
    char evalRelationToAtoms(Vector, const int);
    void traverseTree(const AtomNode*, const double&, const Vector&, const double&, const double&, const int&, 
        const char = 0b00000011, const char = 0); 
    void splitVoxel(const Vector&, const double&); 
    
    static void listFromTree(
        std::vector<int>&,
        const AtomNode*,
        const Vector&,
        const double&,
        const double&,
        const double&,
        const char=0);
    
    size_t tallyVoxelsOfType(const char volume_type, const int max_depth);

    void fillTypeMatrix(Container3D<char>&, std::array<unsigned int,3>, int);
  private:

    static inline AtomTree s_atomtree;
    static inline double s_grid_size;
    static inline double s_r_probe1;
    // DEPRECIATED
    static inline std::unordered_map<unsigned long long int,TripletBundle> s_triplet_data;
    static inline std::unordered_map<int,PairBundle> s_pair_data;
    // DEPRECIATED

    static inline double calcRadiusOfInfluence(const double& max_depth);

    bool isAtom(const Atom&, const Vector&, const double, const double);
    // DEPRECIATED
    bool isProbeExcluded(const Vector& vxl_pos, const double& r_probe, const double&, const std::vector<int>&);
    bool isExcludedByPair(const Vector&, const Vector&, const double&, const double&, const double&, const double&, int);
    bool isExcludedByTriplet(const Vector&, const double&, const std::array<Vector,4>&, const std::array<double,4>&, const double&, const unsigned long long int, const bool = false);
    bool isExcludedByQuadruplet(const Vector&, const double&, const std::array<Vector,4>&, const std::array<double,4>&, const double&, const std::vector<int>&);
    bool isExcludedSetType(const Vector&, const double&, const Vector&, const double&);
    // DEPRECIATED

    std::vector<Voxel> data; // empty or exactly 8 elements
    char type;
};

#endif
