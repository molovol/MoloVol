
#include "model.h"
#include "controller.h"
#include "atom.h"
#include <array>
#include <string>

void Model::defineCell(const double& grid_step, const int& max_depth){
  cell = Space(atoms, grid_step, max_depth);
  return;
}

void Model::storeAtomsInTree(){
  atomtree = AtomTree(atoms);
}

void Model::findCloseAtoms(const double& r_probe){
  //TODO
  return;
}

void Model::calcVolume(){
  std::wstring angstrom = L" \u212B\u00B3";
  cell.placeAtomsInGrid(atoms, atomtree);
  double volume = cell.getVolume();
  Ctrl::getInstance()->notifyUser("Van der Waals Volume: " + std::to_string(volume));
  Ctrl::getInstance()->notifyUserUnicode(angstrom);
  return;
}

void Model::debug(){
  std::array<double,3> cell_min = cell.getMin();
  std::array<double,3> cell_max = cell.getMax();

  for(int dim = 0; dim < 3; dim++){
    std::cout << "Cell Limit in Dim " << dim << ":" << cell_min[dim] << " and " << cell_max[dim] << std::endl;
  }
  return;
}
