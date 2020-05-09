
#include "model.h"
#include "space.h"
#include <array>
//#include <cmath>

void Model::defineCell(const double& grid_step, const int& depth){
  cell = new Space(atoms, grid_step, depth);
  return;
}

void Model::defineCell(){
  cell = new Space(atoms);
}
/*
void Model::calcVolume(const double& grid_step){
  //cell
  //atoms
  //grid_step
  //octree_depth
  octree_depth = 4; // top_level_voxel.side_length = 2^4 * bottom_level_voxel.side_length

  // determine grid steps in all directions
  std::array<size_t,3> = n_steps;
  for(int dim = 0; dim < 3; dim++){
    n_steps[dim] = std::ceil( (cell->getSize())[dim] / grid_step );
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
