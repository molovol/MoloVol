#ifndef SPACE_H

#define SPACE_H

#include <vector>
#include <array>

struct Atom;
class Space{
  public:
    Space(std::vector<Atom> &atoms);
    std::array <double,3> getMin();
    std::array <double,3> getOrigin(); // same as getMin();
    std::array <double,3> getMax();
    std::array <double,3> getSize();
  private:
    std::array <double,3> cart_min; // this is also the "origin" of the space
    std::array <double,3> cart_max;
    // member function for constructor
    void setBoundaries(std::vector<Atom> &atoms);
};

#endif
