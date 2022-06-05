#ifndef ATOM_H

#define ATOM_H

#include "vector.h"
#include <exception>
#include <vector>
#include <array>
#include <iostream>

struct Atom{
  // Constructor
  Atom() : pos_x(0), pos_y(0), pos_z(0), rad(-1), number(0), charge(0), symbol("") {}
  Atom(const std::pair<std::string,std::array<double,3>>& symbol_position)
    : pos_x(symbol_position.second[0]), pos_y(symbol_position.second[1]), pos_z(symbol_position.second[2]),
      rad(-1), number(0), charge(0), symbol(symbol_position.first) {}
  Atom(const std::pair<std::string,std::array<double,3>>& symbol_position, signed charge)
    : pos_x(symbol_position.second[0]), pos_y(symbol_position.second[1]), pos_z(symbol_position.second[2]),
      rad(-1), number(0), charge(charge), symbol(symbol_position.first) {}
  Atom(double x, double y, double z, const std::string& symbol, double rad, unsigned atomic_num)
    : pos_x(x), pos_y(y), pos_z(z), rad(rad), number(atomic_num), charge(0), symbol(symbol) {}
  Atom(double x, double y, double z, const std::string& symbol, double rad, unsigned atomic_num, signed charge)
    : pos_x(x), pos_y(y), pos_z(z), rad(rad), number(atomic_num), charge(charge), symbol(symbol) {}

  // Member variables
  double pos_x, pos_y, pos_z, rad;
  unsigned number;
  signed charge;
  std::string symbol; // May be redundant?

  // Methods
  const std::array<double,3> getPos() const {
    return {pos_x, pos_y, pos_z};
  }
  
  const Vector getPosVec() const {
    return Vector(pos_x, pos_y, pos_z);
  }

  // Not needed for struct member access
  const double getRad() const {
    return rad;
  }

  const double getCoordinate(const char dim){
    switch(dim){
      case 0: return pos_x;
      case 1: return pos_y;
      case 2: return pos_z;
    }
    throw std::invalid_argument("");
  }

  void print(){
    printf("Atom {%s, (%1.3f, %1.3f, %1.3f)}", symbol.c_str(), pos_x, pos_y, pos_z);
  }

  bool isValid() const {return (rad >= 0);}
};

#endif
