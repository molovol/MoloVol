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

#endif
