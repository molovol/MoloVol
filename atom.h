#ifndef ATOM_H

#define ATOM_H

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

  std::array<double,3> getPos(){
    return {pos_x, pos_y, pos_z};
  }
};


static inline unsigned int symbolToNumber(const std::string& symbol){
  const std::array<std::string,18> element_symbols = {
    "H" , "He",
    "Li", "Be", "B" , "C" , "N" , "O" , "F" , "Ne",
    "Na", "Mg", "Al", "Si", "P" , "S" , "Cl", "Ar"
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
