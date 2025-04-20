#ifndef ATOM_H

#define ATOM_H

#include "vector.h"
#include <array>
#include <string>

struct Atom {
  
  typedef std::string symbol_type;
  typedef double num_type;
  typedef unsigned atomic_num_type;
  typedef signed charge_type;

  typedef std::array<num_type,3> pos_type;
  typedef std::pair<symbol_type,pos_type> symbol_pos_pair;

  num_type pos_x, pos_y, pos_z, rad;
  atomic_num_type number;
  charge_type charge;
  symbol_type symbol;

  Atom();
  Atom(const symbol_pos_pair&);
  Atom(const symbol_pos_pair&, charge_type);
  Atom(num_type, num_type, num_type, const symbol_type&, num_type, atomic_num_type);
  Atom(num_type, num_type, num_type, const symbol_type&, num_type, atomic_num_type, charge_type);

  const pos_type getPos() const;
  const Vector getPosVec() const;
  const num_type getRad() const;
  const num_type getCoordinate(const char) const;
  void print() const;
  bool isValid() const;

  bool operator==(const Atom&) const;

};

#endif
