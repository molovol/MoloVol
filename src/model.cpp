
#include "model.h"
#include "controller.h"
#include "atom.h"
#include "special_chars.h"
#include <array>
#include <string>
#include <vector>
#include <algorithm>

void Model::defineCell(const double& grid_step, const int& max_depth){
  cell = Space(atoms, grid_step, max_depth);
  return;
}

void Model::setAtomListForCalculation(const std::vector<std::string>& included_elements){
  atoms.clear();
  for(int i = 0; i < raw_atom_coordinates.size(); i++){
    if(std::find(included_elements.begin(), included_elements.end(), std::get<0>(raw_atom_coordinates[i])) != included_elements.end()){
      Atom at = Atom(std::get<1>(raw_atom_coordinates[i]),
                     std::get<2>(raw_atom_coordinates[i]),
                     std::get<3>(raw_atom_coordinates[i]),
                     std::get<0>(raw_atom_coordinates[i]),
                     radius_map[std::get<0>(raw_atom_coordinates[i])],
                     elem_Z[std::get<0>(raw_atom_coordinates[i])]);
      atoms.push_back(at);
    }
  }
}

void Model::updateAtomRadii(){
  for (Atom& at : atoms){
    at.rad = radius_map[at.symbol];
  }
void Model::storeAtomsInTree(){
  atomtree = AtomTree(atoms);
}

void Model::findCloseAtoms(const double& r_probe){
  //TODO
  return;
}

void Model::calcVolume(){
  cell.placeAtomsInGrid(atoms, atomtree);
  double volume = cell.getVolume();

  std::string message_to_user
    = "Van der Waals Volume: " + std::to_string(volume) + " " + Symbol::angstrom() + Symbol::cubed();
  Ctrl::getInstance()->notifyUser(message_to_user);
}

std::vector<std::tuple<std::string, int, double>> Model::generateAtomList(){
  std::vector<std::tuple<std::string, int, double>> atoms_for_list;
  for(auto elem : atom_amounts){
    atoms_for_list.emplace_back(elem.first, elem.second, radius_map[elem.first]);
  }
  return atoms_for_list;
}

void Model::setRadiusMap(std::unordered_map<std::string, double> map){
  radius_map = map;
}

void Model::debug(){
  std::array<double,3> cell_min = cell.getMin();
  std::array<double,3> cell_max = cell.getMax();

  for(int dim = 0; dim < 3; dim++){
    std::cout << "Cell Limit in Dim " << dim << ":" << cell_min[dim] << " and " << cell_max[dim] << std::endl;
  }
}

std::vector<char> Model:: getMatrix(){
	return this->cell.getMatrix();
}

std::array<double,3> Model::getResolution(){
	return this->cell.getResolution();
}
