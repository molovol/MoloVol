#include "cavity.h"
#include <sstream>

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

void Cavity::setVolumes(const double c_vol, const double s_vol, const std::array<double,3>& min_b, const std::array<double,3>& max_b, const std::array<unsigned int,3>& min_i, const std::array<unsigned int,3>& max_i){
  core_vol = c_vol;
  shell_vol = s_vol;
  min_bound = min_b;
  max_bound = max_b;
  min_index = min_i;
  max_index = max_i;
}
