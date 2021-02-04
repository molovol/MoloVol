#ifndef VOXEL_H

#define VOXEL_H

#include "vector.h"
#include "atomtree.h"
#include <vector>
#include <array>
#include <unordered_map>

struct TripletBundle;
struct PairBundle;
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

    char determineType(
        std::array<double,3> pos,
        const double max_depth);
    
    void determineTypeSingleAtom(
        const Atom& atom, 
        std::array<double,3> pos, // voxel centre
        const double max_depth,
        bool&);
    
    void traverseTree(
        const AtomNode*, 
        const std::array<double,3>&,
        const double&, 
        const double&, 
        const double&,
        const char&,
        const char = 0); 
    
    static std::vector<int> listFromTree(
        const AtomNode*, 
        const std::array<double,3>&, 
        const double&,
        const double&,
        const double&,
        const char=0);

    size_t tallyVoxelsOfType(const char volume_type, const int max_depth);

    void splitVoxel(
        const std::array<double,3>& vxl_pos, 
        const double& max_depth); 
  private:

    static inline AtomTree _atomtree;
    static inline double _grid_size;
    static inline double _r_probe1;
    static inline std::vector<int> _close_atoms;
    static inline int _d = 6;
    static inline std::unordered_map<unsigned long long int,TripletBundle> _triplet_data;
    static inline std::unordered_map<int,PairBundle> _pair_data;

    static inline double calcRadiusOfInfluence(const double& max_depth);

    bool isAtom(const Atom& atom, const double& dist_vxl_at, const double& radius_of_influence); // inline not faster
    bool isProbeExcluded(const std::array<double,3>& vxl_pos, const double& r_probe, const double&, const std::vector<int>&);
    bool isExcludedByPair(const Vector&, const Vector&, const double&, const double&, const double&, const double&, int);
    bool isExcludedByTriplet(const Vector&, const double&, const std::array<Vector,4>&, const std::array<double,4>&, const double&, const unsigned long long int, const bool = false);
    bool isExcludedByQuadruplet(const Vector&, const double&, const std::array<Vector,4>&, const std::array<double,4>&, const double&);
    bool isExcludedSetType(const Vector&, const double&, const Vector&, const double&);

    std::vector<Voxel> data; // empty or exactly 8 elements
    char type;
    // types
    // 'a' : inside of atom
    // 'e' : empty
    // 'm' : mixed
};

struct TripletBundle {
  TripletBundle(){}
  TripletBundle(Vector vec_probe_plane, Vector vec_probe_normal)
    : vec_probe_plane(vec_probe_plane), vec_probe_normal(vec_probe_normal){}

  Vector vec_probe_plane;
  Vector vec_probe_normal;
};

struct PairBundle {
  PairBundle(){}
  PairBundle(Vector unitvec_parallel, double probe_parallel, double probe_orthogonal)
    : unitvec_parallel(unitvec_parallel), probe_parallel(probe_parallel), probe_orthogonal(probe_orthogonal){}

  Vector unitvec_parallel;
  double probe_parallel;
  double probe_orthogonal;
};

#endif
