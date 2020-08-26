#ifndef MISC_H

#define MISC_H

#include <array>
#include <cmath>

inline double distance(const std::array<double,3> &start, const std::array<double,3> &end){
  return std::pow( (pow(end[0]-start[0],2) + pow(end[1]-start[1],2) + pow(end[2]-start[2],2)) , 0.5);
}

inline double distance(const std::array<double,3> &start, const std::array<double,3> &end, const int dim){
  return end[dim]-start[dim];
}

inline std::string fileExtension(const std::string& path){
  // will cause an issue, if there is a dot in the middle of the file AND no file extension
  std::string after_dot = "";
  int dot_pos = path.find_last_of(".");
  if (dot_pos != std::string::npos){
    return path.substr(dot_pos+1);
  }
  else{
    return "invalid";
  }
}

#endif
