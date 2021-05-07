#include "cavity.h"

double Cavity::getVolume() const {return core_vol + shell_vol;}

double Cavity::getSurfShell() const {return surf_shell;}
double Cavity::getSurfCore() const {return surf_core;}

bool compareVolume(const Cavity& a, const Cavity& b){
  return a.getVolume() < b.getVolume();
}

void inverseSort(std::vector<Cavity>& cavities){
  std::sort(cavities.rbegin(), cavities.rend(), compareVolume);
}
