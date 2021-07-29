#include "cavity.h"
#include <sstream>

int Cavity::getNumberEntrances() const {return n_entrances;}
double Cavity::getVolume() const {return core_vol + shell_vol;}
double Cavity::getSurfShell() const {return surf_shell;}
double Cavity::getSurfCore() const {return surf_core;}

std::string Cavity::getPosition() const {
  std::string str = "(";
  for (int i = 0; i<3; ++i){
    if(i){ str += ", ";}
  
    std::ostringstream ss;
    ss.precision(2);
    ss << ((min_bound[i] + max_bound[i]) * 0.5);
    str += ss.str();
  }
  return str + ")";
}

bool compareVolume(const Cavity& a, const Cavity& b){
  return a.getVolume() < b.getVolume();
}

void inverseSort(std::vector<Cavity>& cavities){
  std::sort(cavities.rbegin(), cavities.rend(), compareVolume);
}

