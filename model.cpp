
#include "model.h"
#include "space.h"
#include <array>

void Model::defineCell(const double& grid_step, const int& max_depth){
  cell = new Space(atoms, grid_step, max_depth);
  return;
}

void Model::calcVolume(){
  cell->placeAtomsInGrid(atoms);

  double volume = cell->getVolume();
  std::cout << volume << std::endl;
  return;
}

void Model::debug(){
  std::array<double,3> cell_min = cell->getMin();
  std::array<double,3> cell_max = cell->getMax();

  for(int dim = 0; dim < 3; dim++){
    std::cout << "Cell Limit in Dim " << dim << ":" << cell_min[dim] << " and " << cell_max[dim] << std::endl; 
  }
  return;
}
