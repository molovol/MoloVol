#ifndef CRYSTALLOGRAPHER_H

#define CRYSTALLOGRAPHER_H

#include <array>

// The crystallographer compartmentalises code that is used to process unit cells
namespace Cryst {
  typedef std::array<std::array<double,3>,3> MatR3;

  MatR3 orthogonalizeUnitCell(const std::array<double,6>& cell_param);
}

#endif
