#ifndef SPACE_H

#define SPACE_H

#include "voxel.h"
#include <vector>
#include <array>

class AtomTree;
struct Atom;
class Voxel;
class Space{
  public:
    Space() = default;
    Space(std::vector<Atom>&, const double&, const int&);
    std::array <double,3> getMin();
    std::array <double,3> getOrigin(); // same as getMin();
    std::array <double,3> getMax();
    std::array <double,3> getSize();

    Voxel& getElement(const size_t &x, const size_t &y, const size_t &z);
    Voxel& getElement(const size_t &i);
    void printGrid();

    void placeAtomsInGrid(const AtomTree&, const double&);
    double getVolume();
  
  private:
    std::array <double,3> cart_min; // this is also the "origin" of the space
    std::array <double,3> cart_max;
    // member function for constructor
    void setBoundaries(const std::vector<Atom>&);
  
    std::vector<Voxel> grid;
    std::array<size_t,3> n_gridsteps; // i think top level voxel?
    void setGrid();
    //void setGrid(const double &grid_step, const int &max_depth);

    double grid_size;
    int max_depth; // for voxels
    
};

#endif
