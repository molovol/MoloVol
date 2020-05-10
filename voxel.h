#ifndef VOXEL_H

#define VOXEL_H

#include <vector>

struct Atom;
class Voxel{
  public:
    Voxel& access(const short& x, const short& y, const short& z);
    Voxel& access(const short& i);
    char getType();
    
    void determineType
      (const std::vector<Atom>& atoms, 
       std::array<double,3> pos,
       const double& grid_size,
       const double max_depth);
    size_t tallyVoxelsOfType(const char volume_type, const int max_depth);

  private:
    std::vector<Voxel> data; // empty or exactly 8 elements
    char type;
    // types
    // 'a' : inside of atom
    // 'e' : empty
    // 'm' : mixed
};

#endif
