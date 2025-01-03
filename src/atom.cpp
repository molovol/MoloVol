#include "atom.h"
#include <exception>
#include <iostream>

// Constructor
Atom::Atom() : pos_x(0), pos_y(0), pos_z(0), rad(-1), number(0), charge(0), symbol("") {}
Atom::Atom(const std::pair<std::string,std::array<double,3>>& symbol_position)
    : pos_x(symbol_position.second[0]), pos_y(symbol_position.second[1]), pos_z(symbol_position.second[2]),
      rad(-1), number(0), charge(0), symbol(symbol_position.first) {}
Atom::Atom(const std::pair<std::string,std::array<double,3>>& symbol_position, signed charge)
    : pos_x(symbol_position.second[0]), pos_y(symbol_position.second[1]), pos_z(symbol_position.second[2]),
      rad(-1), number(0), charge(charge), symbol(symbol_position.first) {}
Atom::Atom(double x, double y, double z, const std::string& symbol, double rad, unsigned atomic_num)
    : pos_x(x), pos_y(y), pos_z(z), rad(rad), number(atomic_num), charge(0), symbol(symbol) {}
Atom::Atom(double x, double y, double z, const std::string& symbol, double rad, unsigned atomic_num, signed charge)
    : pos_x(x), pos_y(y), pos_z(z), rad(rad), number(atomic_num), charge(charge), symbol(symbol) {}

// Methods
const std::array<double,3> Atom::getPos() const {
  return {pos_x, pos_y, pos_z};
}

const Vector Atom::getPosVec() const {
  return Vector(pos_x, pos_y, pos_z);
}

// Not needed for struct member access
const double Atom::getRad() const {
  return rad;
}

const double Atom::getCoordinate(const char dim) const {
  switch(dim){
    case 0: return pos_x;
    case 1: return pos_y;
    case 2: return pos_z;
  }
  throw std::invalid_argument("");
}

void Atom::print() const {
  printf("Atom {%s, (%1.3f, %1.3f, %1.3f)}", symbol.c_str(), pos_x, pos_y, pos_z);
}

bool Atom::isValid() const {return (rad >= 0);}

bool Atom::operator==(const Atom& at) const {
  return 
    pos_x  == at.pos_x  &&
    pos_y  == at.pos_y  &&
    pos_z  == at.pos_z  &&
    rad    == at.rad    &&
    number == at.number &&
    charge == at.charge &&
    symbol == at.symbol;
}

