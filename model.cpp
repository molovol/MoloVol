
#include "model.h"
#include "space.h"
#include <array>

void Model::defineCell(){
  cell = new Space(atoms);
}
/*
void Model::calcVolume(const double& grid_step){
  //cell
  //atoms
  //grid_step
  
  // determine grid steps in all directions
  std::array<int,3> = n_steps;
  for(int dim = 0; dim < 3; dim++){
    n_steps[dim] = (cell->getOrigin())[dim]
  } 
  

  return;
}
*/

void Model::debug(){
  std::array<double,3> cell_min = cell->getMin();
  std::array<double,3> cell_max = cell->getMax();

  for(int dim = 0; dim < 3; dim++){
    std::cout << "Cell Limit in Dim " << dim << ":" << cell_min[dim] << " and " << cell_max[dim] << std::endl; 
  }
  return;
}
