#ifndef SPACE_H

#define SPACE_H

#include "voxel.h"
#include "container3d.h"
#include "cavity.h"
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
    Space(std::vector<Atom>&, const double, const int, const double, const bool, const std::array<double,3>);

    // access
    std::array <double,3> getMin();
    std::array <double,3> getOrigin(); // same as getMin();
    std::array <double,3> getMax();
    std::array <double,3> getSize();
    bool isInBounds(const std::array<int,3>&, const unsigned);
    bool isInBounds(const std::array<unsigned,3>&, const unsigned);
    double getVxlSize() const;
    Container3D<Voxel>& getGrid(const unsigned);

    // get voxel
    Voxel& getVxlFromGrid(const unsigned int, unsigned);
    Voxel& getVxlFromGrid(const unsigned int, const unsigned int, const unsigned int, unsigned);
    Voxel& getVxlFromGrid(const std::array<unsigned int,3>, unsigned);
    Voxel& getVxlFromGrid(const std::array<int,3>, unsigned);
    Voxel& getTopVxl(const unsigned int);
    Voxel& getTopVxl(const unsigned int, const unsigned int, const unsigned int);
    Voxel& getTopVxl(const std::array<unsigned int,3>);
    Voxel& getTopVxl(const std::array<int,3>);
    std::array<unsigned int,3> getGridsteps();
    std::array<std::array<unsigned int,3>,2> getUnitCellIndexes();
    unsigned long int totalVxlOnLvl(const int) const;

    int getMaxDepth(){return max_depth;}
    // output
    void printGrid();

    // type evaluation
    void assignTypeInGrid(const AtomTree&, const double, const double, bool, bool&);
    void getVolume(std::map<char,double>&, std::vector<Cavity>&);
    void getUnitCellVolume(std::map<char,double>&, std::vector<Cavity>&);
    void setUnitCellIndexes();
    void tallyVoxelsUnitCell(std::array<unsigned int,3>,
                            double,
                            std::map<char, double>&,
                            std::map<unsigned char, double>&,
                            std::map<unsigned char, double>&,
                            std::map<unsigned char, std::array<unsigned,3>>&,
                            std::map<unsigned char, std::array<unsigned,3>>&);

    // surface area
    double calcSurfArea(const std::vector<char>&);
    double calcSurfArea(const std::vector<char>&, const unsigned char, std::array<unsigned int,3>, std::array<unsigned int,3>);

  private:
    std::array <double,3> cart_min; // this is also the "origin" of the space
    std::array <double,3> cart_max;
    std::vector<Container3D<Voxel>> _grid;
    std::array<unsigned int,3> n_gridsteps; // number of top level voxels in x,y,z direction
    bool unit_cell; // option to analyze unit cell
    std::array<double,3> unit_cell_limits; // cartesian coordinates of the unit cell orthogonal axes
    std::array<unsigned int,3> unit_cell_start_index; // bottom level voxels indexes for the start of the unit cell in x,y,z direction
    std::array<unsigned int,3> unit_cell_end_index; // bottom level voxels indexes for the end of the unit cell in x,y,z direction
    std::array<double,3> unit_cell_mod_index; // bottom level voxels fractional indexes for the end of the unit cell in x,y,z direction
    double grid_size;
    int max_depth; // for voxels

    void setBoundaries(const std::vector<Atom>&, const double);

    void initGrid();

    const std::array<unsigned long int,3> gridstepsOnLvl(const int) const;
    void assignAtomVsCore();
    void identifyCavities();
    void descendToCore(unsigned char&, const std::array<unsigned,3>, int);
    void assignShellVsVoid();

    double tallySurface(const std::vector<char>&, std::array<unsigned int,3>&, std::array<unsigned int,3>&, const unsigned char=0, const bool=false);
    unsigned char evalMarchingCubeConfig(const std::array<unsigned int,3>&, const std::vector<char>&, const unsigned char, const bool);

};

class SurfaceLUT {
  private:
    static const std::array<unsigned char,256> types_by_config;
    static const std::array<double, 15> area_by_config;
  public:
    static unsigned char configToType(unsigned char config);
    static double typeToArea(unsigned char type);
    static double configToArea(unsigned char config);
};

#endif
