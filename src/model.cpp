
#include "model.h"
#include "controller.h"
#include "atom.h"
#include "special_chars.h"
#include <array>
#include <string>

void Model::defineCell(const double& grid_step, const int& max_depth){
  cell = Space(atoms, grid_step, max_depth);
  return;
}

void Model::storeAtomsInTree(){
  atomtree = AtomTree(atoms);
}

void Model::updateAtomRadii(){
  for (Atom& at : atoms){
    at.rad = radius_map[at.symbol];
  }
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

std::deque<bool> Model:: getMatrix(){
	int dim = 100;
	auto tmp =std::deque<bool>(dim*dim*dim,0);
	
	int i=0;
	for (auto iter = tmp.begin();iter != tmp.end();++iter){
		int x = i % dim-50;
		int y = (i % (dim*dim))/dim-50;
		int z = i / (dim*dim)-50;
		if (x*x + (y-20)*(y-20) + (z-30)*(z-30) < 50 || x*x + y*y + z*z < 320 || (x<40 && x>20 && y<40 && y>20 && z<40 && z>20)){
			*iter = 1;
		}
		++i;
	}
	return tmp;
}
