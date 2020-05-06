#ifndef SPACE_H

#define SPACE_H

#include <vector>
#include <array>

struct Atom;
class Space{
  public:
    Space(std::vector<Atom> &atoms);
    std::array <double,3> getMin();
    std::array <double,3> getMax();
  private:
    std::array <double,3> cart_min;
    std::array <double,3> cart_max;
    // member function for constructor
    void findMinMaxFromAtoms(std::vector<Atom> &atoms,std::array<double,3> &cart_min, std::array<double,3> &cart_max);
};

#endif
