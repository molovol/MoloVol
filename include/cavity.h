#ifndef CAVITY_H

#define CAVITY_H

#include <array>
#include <algorithm>
#include <vector>
#include <string>

struct Cavity{
  Cavity() = default;
  Cavity(unsigned char id, double core_vol, double shell_vol, std::array<double,3> min_bound, std::array<double,3> max_bound, std::array<unsigned int,3> min_index, std::array<unsigned int,3> max_index) :
    id(id), core_vol(core_vol), shell_vol(shell_vol), min_bound(min_bound), max_bound(max_bound), min_index(min_index), max_index(max_index){};
  unsigned char id; // the number assigned to core and shell voxels belonging to this cavity
  double core_vol;
  double shell_vol;
  std::array<double,3> min_bound;
  std::array<double,3> max_bound;
  std::array<unsigned int,3> min_index;
  std::array<unsigned int,3> max_index;

  double surf_core;
  double surf_shell;

  double getVolume() const;
  double getSurfCore() const;
  double getSurfShell() const;
  std::string getPosition() const;
};

bool compareVolume(const Cavity& a, const Cavity& b);

void inverseSort(std::vector<Cavity>& cavities);

#endif
