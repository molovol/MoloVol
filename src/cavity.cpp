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

std::string Cavity::cavTypeDescriptor() const {
  std::string cav_type = "Tunnel";
  switch(n_entrances){
    case 0 :
      cav_type = "Isolated";
      break;
    case 1 :
      cav_type = "Pocket";
      break;
  }
  return cav_type;
}


bool compareVolume(const Cavity& a, const Cavity& b){
  return a.getVolume() < b.getVolume();
}

void inverseSort(std::vector<Cavity>& cavities){
  std::sort(cavities.rbegin(), cavities.rend(), compareVolume);
}

// only valid for isolated structures, not for unit cell analysis
std::array<double, 3> getTotalVolPerType(const std::vector<Cavity>& cavities, const bool probe_mode){
  std::array<double, 3> volume_per_type;
  if(probe_mode){
    for(unsigned int i = 0; i < cavities.size(); i++){
      unsigned int type;
      if(cavities[i].getNumberEntrances() == 0){
        type = 0;
      }
      else if(cavities[i].getNumberEntrances() == 1){
        type = 1;
      }
      else{
        type = 2;
      }
      volume_per_type[type] += cavities[i].getVolume();
    }
  }
  else{
  // in single probe mode, the first cavity with id 1 is the outside space plus pockets and tunnels
  // thus there is no physical meaning
    for(unsigned int i = 0; i < cavities.size(); i++){
      if(cavities[i].id > 1){
        volume_per_type[0] += cavities[i].getVolume();
      }
    }
  }
  return volume_per_type;
}
