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
    Space(std::vector<Atom>&, const double, const int, const double);

    // access
    std::array <double,3> getMin();
    std::array <double,3> getOrigin(); // same as getMin();
    std::array <double,3> getMax();
    std::array <double,3> getSize();
    Voxel& getVoxel(unsigned int, unsigned int, unsigned int, int);
    Voxel& getVoxel(std::array<int,3>, int);
    bool coordInBounds(const std::array<int,3>&, const unsigned);
    double getResolution() const;

    Voxel& getElement(const unsigned int);
    Voxel& getElement(const unsigned int, const unsigned int, const unsigned int);
    Voxel& getElement(const std::array<unsigned int,3>);
    std::array<unsigned int,3> getGridsteps();
    unsigned long int totalVxlOnLvl(const int) const;

    int getMaxDepth(){return max_depth;}
    // output
    Container3D<char> generateTypeTensor(); 
    void printGrid();

    // type evaluation
    void placeAtomsInGrid(const AtomTree&, const double&);
    std::map<char,double> getVolume();
  private:
    std::array <double,3> cart_min; // this is also the "origin" of the space
    std::array <double,3> cart_max;
    std::vector<Voxel> grid;
    std::array<unsigned int,3> n_gridsteps; // number of top level voxels in x,y,z direction 
    double grid_size;
    int max_depth; // for voxels
    
    void setBoundaries(const std::vector<Atom>&, const double);
    void setGrid();
    const std::array<unsigned long int,3> gridstepsOnLvl(const int) const;
    void assignAtomVsCore();
    void assignShellVsVoid();
    
};

#endif
