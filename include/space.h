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
<<<<<<< HEAD
    bool coordInBounds(const std::array<int,3>&, const unsigned); // not in use
    double getResolution() const; // misnamed

    // get voxel
    Voxel& getVxl(int, int, int, int);
    Voxel& getVxl(const std::array<int,3>&, int);
    Voxel& getVxlFromGrid(const unsigned int, unsigned);
    Voxel& getVxlFromGrid(const unsigned int, const unsigned int, const unsigned int, unsigned);
    Voxel& getVxlFromGrid(const std::array<unsigned int,3>, unsigned);
    Voxel& getVxlFromGrid(const std::array<int,3>, unsigned);
    Voxel& getTopVxl(const unsigned int);
    Voxel& getTopVxl(const unsigned int, const unsigned int, const unsigned int);
    Voxel& getTopVxl(const std::array<unsigned int,3>);
    Voxel& getTopVxl(const std::array<int,3>);
    std::array<unsigned int,3> getGridsteps();
    unsigned long int totalVxlOnLvl(const int) const;
=======
>>>>>>> master

    Voxel& getElement(const size_t &x, const size_t &y, const size_t &z);
    Voxel& getElement(const size_t &i);
    void printGrid();

<<<<<<< HEAD
    // type evaluation
    void placeAtomsInGrid(const AtomTree&, const double&);
    std::map<char,double> getVolume();

  private:
    std::array <double,3> cart_min; // this is also the "origin" of the space
    std::array <double,3> cart_max;
    std::vector<Container3D<Voxel>> _grid;
    std::array<unsigned int,3> n_gridsteps; // number of top level voxels in x,y,z direction 
    double grid_size;
    int max_depth; // for voxels
    
    void setBoundaries(const std::vector<Atom>&, const double);

    void initGrid();
    void updateGrid();
    void fillGrid(Voxel& vxl, int x, int y, int z, unsigned lvl);

    const std::array<unsigned long int,3> gridstepsOnLvl(const int) const;
    void assignAtomVsCore();
    void assignShellVsVoid();
    
=======
    void placeAtomsInGrid(const AtomTree&);
    double getVolume();
  
  private:
    std::array <double,3> cart_min; // this is also the "origin" of the space
    std::array <double,3> cart_max;
    // member function for constructor
    void setBoundaries(std::vector<Atom> &atoms);
  
    std::vector<Voxel> grid;
    std::array<size_t,3> n_gridsteps; // i think top level voxel?
    void setGrid();
    //void setGrid(const double &grid_step, const int &max_depth);

    double grid_size;
    int max_depth; // for voxels
    
>>>>>>> master
};

#endif
