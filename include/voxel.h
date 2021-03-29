#ifndef VOXEL_H

#define VOXEL_H

#include <vector>
#include <array>

/*
// DEPRECIATED
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
// DEPRECIATED
*/

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
    Voxel& access(const short& x, const short& y, const short& z);
    Voxel& access(const short& i);
    char getType();

   /* replaced by other determineType function
    void determineType
      (const std::vector<Atom>& atoms, 
       std::array<double,3> pos,
       const double& grid_size,
       const double max_depth);
    */

    char determineType
       (std::array<double,3> pos,
       const double& grid_size,
       const double max_depth,
       const AtomTree& atomtree);
    
    void determineTypeSingleAtom
      (const Atom& atom, 
       std::array<double,3> pos, // voxel centre
       const double& grid_size,
       const double max_depth);
   
    void traverseTree
      (const AtomNode* node, 
       int dim, 
       const double& at_rad, 
       const double& vxl_rad, 
       const std::array<double,3> vxl_pos,
       const double& grid_size, 
       const double& max_depth);
    size_t tallyVoxelsOfType(const char volume_type, const int max_depth);

  private:
    static inline Space* s_cell;
    static inline AtomTree s_atomtree;
    static inline double s_grid_size;
    static inline double s_r_probe1;
    static inline SearchIndex s_search_indices;
    // DEPRECIATED
//    static inline std::unordered_map<unsigned long long int,TripletBundle> s_triplet_data;
//    static inline std::unordered_map<int,PairBundle> s_pair_data;
    // DEPRECIATED

    static inline double calcRadiusOfInfluence(const double& max_depth);

    bool isAtom(const Atom&, const Vector&, const double, const double);
    void searchForCore(const std::array<unsigned int,3>&, const unsigned, bool=false);
    /*
    // DEPRECIATED
    bool isProbeExcluded(const Vector& vxl_pos, const double& r_probe, const double&, const std::vector<int>&);
    bool isExcludedByPair(const Vector&, const Vector&, const double&, const double&, const double&, const double&, int);
    bool isExcludedByTriplet(const Vector&, const double&, const std::array<Vector,4>&, const std::array<double,4>&, const double&, const unsigned long long int, const bool = false);
    bool isExcludedByQuadruplet(const Vector&, const double&, const std::array<Vector,4>&, const std::array<double,4>&, const double&, const std::vector<int>&);
    bool isExcludedSetType(const Vector&, const double&, const Vector&, const double&);
    // DEPRECIATED
    // */

    std::vector<Voxel> data; // empty or exactly 8 elements
    char type;
    // types
    // 'a' : inside of atom
    // 'e' : empty
    // 'm' : mixed
};

#endif
