
#include "model.h"
#include "space.h"

void Model::defineCell(){
  cell = new Space(atoms);
}

void Model::debug(){
  std::array<double,3> cell_min = cell->getMin();
  std::array<double,3> cell_max = cell->getMax();

  for(int dim = 0; dim < 3; dim++){
    std::cout << "Cell Limit in Dim " << dim << ":" << cell_min[dim] << " and " << cell_max[dim] << std::endl; 
  }
  return;
}
