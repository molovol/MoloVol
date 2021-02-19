#ifndef SPACE_H

#define SPACE_H

#include "voxel.h"
#include "container3d.h"
#include <vector>
#include <array>
#include <map>

class AtomTree;
struct Atom;
class Voxel;
class Space{
  public:
    // constructors
    Space() = default;
    Space(std::vector<Atom>&, const double&, const int&);

    // access
    std::array <double,3> getMin();
    std::array <double,3> getOrigin(); // same as getMin();
    std::array <double,3> getMax();
    std::array <double,3> getSize();

    Voxel& getElement(const size_t &x, const size_t &y, const size_t &z);
    Voxel& getElement(const size_t &i);
    std::array<size_t,3> getGridsteps();
    const std::array<unsigned int,3> gridstepsOnLevel(const int) const;

    // output
    Container3D<char> generateTypeTensor(); 
    void printGrid();

    // type evaluation
    void placeAtomsInGrid(const AtomTree&, const double&);
    std::map<char,double> getVolume();
  private:
    std::array <double,3> cart_min; // this is also the "origin" of the space
    std::array <double,3> cart_max;
    // member function for constructor
    void setBoundaries(const std::vector<Atom>&);
  
    std::vector<Voxel> grid;
    std::array<size_t,3> n_gridsteps; // number of top level voxels in x,y,z direction 
    void setGrid();

    double grid_size;
    int max_depth; // for voxels
    
};

#endif
