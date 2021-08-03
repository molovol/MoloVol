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

std::string Cavity::cavTypeDescriptor(const bool probe_mode) const {
  if (probe_mode){
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
  else {
    return (id == 1)? "Outside" : "Isolated";
  }
}


bool compareVolume(const Cavity& a, const Cavity& b){
  return a.getVolume() < b.getVolume();
}

void inverseSort(std::vector<Cavity>& cavities){
  std::sort(cavities.rbegin(), cavities.rend(), compareVolume);
}

// only valid for isolated structures, not for unit cell analysis
std::array<double, 3> getTotalVolPerType(const std::vector<Cavity>& cavities, const bool probe_mode){
  std::array<double, 3> volume_per_type = {0,0,0};
  if(probe_mode){
    for(const auto& cav : cavities){
      int type = cav.getNumberEntrances();
      if (type > 2) {type = 2;}
      volume_per_type[type] += cav.getVolume();
    }
  }
  else{
  // in single probe mode, the first cavity with id 1 is the outside space plus pockets and tunnels
  // thus there is no physical meaning
    for(const auto& cav : cavities){
      if(cav.id == 1){continue;}
      volume_per_type[0] += cav.getVolume();
    }
  }
  return volume_per_type;
}
