#ifndef ATOM_H

#define ATOM_H

#include "exception.h"
#include <array>
#include <iostream>

static inline unsigned int symbolToNumber(const std::string& symbol);

struct Atom{
  Atom(const double& x_inp, const double& y_inp, const double& z_inp, const std::string& symbol_inp, const double& rad_inp)
    : pos_x(x_inp), 
      pos_y(y_inp), 
      pos_z(z_inp), 
      rad(rad_inp),
      number(symbolToNumber(symbol_inp)), 
      symbol(symbol_inp) {}

  double pos_x, pos_y, pos_z, rad;
  unsigned int number;
  std::string symbol;
  //std::vector<Atom*> close_atoms;

  const std::array<double,3> getPos() const {
    return {pos_x, pos_y, pos_z};
  }
  const double getCoordinate(const char& dim){
    switch(dim){
      case 0: return pos_x;
      case 1: return pos_y;
      case 2: return pos_z;
    }
    throw ExceptIllegalFunctionCall();
  }
};


static inline unsigned int symbolToNumber(const std::string& symbol){
  const std::array<std::string,30> element_symbols = {
    "H" , "He",
    "Li", "Be", "B" , "C" , "N" , "O" , "F" , "Ne",
    "Na", "Mg", "Al", "Si", "P" , "S" , "Cl", "Ar",
    "K" , "Ca", "Sc", "Ti", "V" , "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn"
  };

  for(int i = 0; i < element_symbols.size(); i++){
    if (symbol == element_symbols[i]){
      return i+1;
    }
  }
  std::cout << "There has been an error in atom.h" << std::endl;
  return 0;
}
  
#endif
