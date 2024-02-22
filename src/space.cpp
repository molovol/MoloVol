#include "space.h"
#include "atom.h"
#include "voxel.h"
#include "atomtree.h"
#include "misc.h"
#include "exception.h"
#include "controller.h"
#include <cmath>
#include <cassert>
#include <stdexcept>
#include <algorithm> // find
#include <numeric> // accumulate

/////////////////
// CONSTRUCTOR //
/////////////////

Space::Space(std::vector<Atom> &atoms, const double bot_lvl_vxl_dist, const int depth, const double r_probe, const bool unit_cell_option, const std::array<double,3> unit_cell_axes)
  :_grid_size(bot_lvl_vxl_dist), _max_depth(depth), _unit_cell_limits(unit_cell_axes), _unit_cell(unit_cell_option){
  setBoundaries(atoms,r_probe+2*bot_lvl_vxl_dist);
  initGrid();
}

///////////////////////////////
// FUNCTIONS FOR CONSTRUCTOR //
///////////////////////////////

// member function loops through all atoms (previously extracted
// from file) and saves the minimum and maximum positions in all
// three cartesian directions out of all atoms. Also finds max
// radius of all atoms and sets the space boundaries slightly so
// that all atoms fit the space.
void Space::setBoundaries(const std::vector<Atom> &atoms, const double add_space){
  double max_radius = 0;
  for(size_t at = 0; at < atoms.size(); at++){
    std::array<double,3> atom_pos = atoms[at].getPos();

    // if we are at the first atom, then the atom position is min/max by default
    if(at == 0){
      _cart_min = atom_pos;
      _cart_max = atom_pos;
      max_radius = atoms[at].rad;
    }

    // after the first atom, begin comparing the atom positions to min/max and save if exceeds previously saved values
    else{
      for(int dim = 0; dim < 3; dim++){
        if(atom_pos[dim] > _cart_max[dim]){
          _cart_max[dim] = atom_pos[dim];
        }
        if(atom_pos[dim] < _cart_min[dim]){
          _cart_min[dim] = atom_pos[dim];
        }
        if(atoms[at].rad > max_radius){
          max_radius = atoms[at].rad;
        }
      }
    }
  }
  // expand boundaries by a little more than the largest atom found
  for(int dim = 0; dim < 3; dim++){
    _cart_min[dim] -= (add_space + max_radius);
    _cart_max[dim] += (add_space + max_radius);
  }
  // in case the unit cell has wide empty gaps on the sides, the grid would be smaller than the unit cell which would cause problems for volume and surface calculation
  // the grid boundaries need to be set further than the unit cell boundaries
  if(_unit_cell){
    for(int dim = 0; dim < 3; dim++){
      if(_cart_min[dim] >= -2 * _grid_size){
        _cart_min[dim] = -2 * _grid_size;
      }
      if(_cart_max[dim] <= _unit_cell_limits[dim] + 2 * _grid_size){
        _cart_max[dim] = _unit_cell_limits[dim] + 2 * _grid_size;
      }
    }
  }
  // align the grid with origin (0,0,0) of atomic Cartesian coordinates, useful for unit cell analysis
  for(int dim = 0; dim < 3; dim++){
    if(custom_fmod(_cart_min[dim],_grid_size) != 0){
      _cart_min[dim] -= custom_fmod(_cart_min[dim],_grid_size);
    }
  }
  return;
}

// based on the grid step and the octree _max_depth, this function produces a
// 3D grid (in form of a 1D vector) that contains all top level voxels.
void Space::initGrid(){
  _grid.clear();
  // determine how many top lvl voxels in each direction are needed
  std::array<unsigned long,3> n_top_lvl_vxl;
  for (int dim = 0; dim < 3; dim++){
    n_top_lvl_vxl[dim] = std::ceil (std::ceil( (getSize())[dim] / _grid_size ) / std::pow(2,_max_depth) );
  }
  // initialise 3d tensors for each octree level
  for (int lvl = 0; lvl <= _max_depth; ++lvl){
    _grid.push_back(Container3D<Voxel>( n_top_lvl_vxl[0]*pow(2,_max_depth-lvl),
                                        n_top_lvl_vxl[1]*pow(2,_max_depth-lvl),
                                        n_top_lvl_vxl[2]*pow(2,_max_depth-lvl)));
  }
}

/////////////////////
// TYPE ASSIGNMENT //
/////////////////////

// sets all voxel's types, determined by the input atoms
void Space::assignTypeInGrid(std::vector<Atom>& atomlist, std::vector<Cavity>& cavities, const double r_probe1, const double r_probe2, bool probe_mode, bool& cavities_exceeded){
  // save variable that all voxels need access to for their type determination as static members of Voxel class
  Voxel::prepareTypeAssignment(this, atomlist);
  if (probe_mode){
    // first run algorithm with the larger probe to exclude most voxels - "masking mode"
    Voxel::storeProbe(r_probe2, true);
    Ctrl::getInstance()->updateStatus("Blocking off cavities with large probe...");
    assignAtomVsCore();
    assignShellVsVoid();
  }

  Ctrl::getInstance()->updateStatus(std::string("Probing space") + (probe_mode? " with small probe..." : "..."));
  Voxel::storeProbe(r_probe1, false);
  assignAtomVsCore();

  Ctrl::getInstance()->updateStatus("Identifying cavities...");
  try{identifyCavities(cavities, probe_mode);}
  catch (const std::overflow_error& e){cavities_exceeded = true;}

  Ctrl::getInstance()->updateStatus("Searching inaccessible areas...");
  assignShellVsVoid();
}

void Space::assignAtomVsCore(){
  if (Ctrl::getInstance()->getAbortFlag()){return;}
  // calculate position of first voxel
  const std::array<double,3> vxl_origin = getOrigin();
  // calculate side length of top level voxel
  const double vxl_dist = _grid_size * pow(2,_max_depth);
  std::array<double,3> vxl_pos;
  std::array<unsigned,3> top_lvl_index;
  for(top_lvl_index[0] = 0; top_lvl_index[0] < getGridsteps()[0]; top_lvl_index[0]++){
    Ctrl::getInstance()->updateCalculationStatus();
    vxl_pos[0] = vxl_origin[0] + vxl_dist * (0.5 + top_lvl_index[0]);
    for(top_lvl_index[1] = 0; top_lvl_index[1] < getGridsteps()[1]; top_lvl_index[1]++){
      vxl_pos[1] = vxl_origin[1] + vxl_dist * (0.5 + top_lvl_index[1]);
      for(top_lvl_index[2] = 0; top_lvl_index[2] < getGridsteps()[2]; top_lvl_index[2]++){
        vxl_pos[2] = vxl_origin[2] + vxl_dist * (0.5 + top_lvl_index[2]);
        // voxel position is deliberately not stored in voxel object to reduce memory cost
        if (Ctrl::getInstance()->getAbortFlag()){return;}
        getTopVxl(top_lvl_index).evalRelationToAtoms(top_lvl_index, vxl_pos, _max_depth);
      }
    }
    Ctrl::getInstance()->updateProgressBar(int(100*(double(top_lvl_index[0])+1)/double(getGridsteps()[0])));
  }
}

void Space::identifyCavities(std::vector<Cavity>& cavities, const bool cavity_types){
  if (Ctrl::getInstance()->getAbortFlag()){return;}
  std::array<unsigned int,3> vxl_index;
  unsigned char id = 1;
  for(vxl_index[0] = 0; vxl_index[0] < getGridsteps()[0]; vxl_index[0]++){
    for(vxl_index[1] = 0; vxl_index[1] < getGridsteps()[1]; vxl_index[1]++){
      for(vxl_index[2] = 0; vxl_index[2] < getGridsteps()[2]; vxl_index[2]++){
        if (Ctrl::getInstance()->getAbortFlag()){return;}
        try{
          descendToCore(cavities, id,vxl_index,getMaxDepth(),cavity_types); // id gets iterated inside this function
        }
        catch (const std::overflow_error& e){
          throw;
        }
      }
    }
    Ctrl::getInstance()->updateProgressBar(int(100*(double(vxl_index[0])+1)/double(getGridsteps()[0])));
  }
}

// this function finds the lowest level core voxel. this voxel becomes the entry point
// for the flood fill
void Space::descendToCore(std::vector<Cavity>& cavities, unsigned char& id, const std::array<unsigned,3> index, int lvl, const bool cavity_types){
  Voxel& vxl = getVxlFromGrid(index,lvl);
  if (!vxl.isCore()){return;}
  if (!vxl.hasSubvoxel()){
    if(vxl.floodFill(cavities, id, index, lvl, cavity_types)){
      if (id == 0b11111111){
        throw std::overflow_error("Too many isolated cavities detected!");
      }
      id++;
    }
  }
  else {
    std::array<unsigned,3> subindex;
    for (char i = 0; i < 2; ++i){
      subindex[0] = index[0]*2 + i;
      for (char j = 0; j < 2; ++j){
        subindex[1] = index[1]*2 + j;
        for (char k = 0; k < 2; ++k){
          subindex[2] = index[2]*2 + k;
          if (Ctrl::getInstance()->getAbortFlag()){return;}
          try {descendToCore(cavities, id, subindex, lvl-1, cavity_types);}
          catch (const std::overflow_error& e){throw;}
        }
      }
    }
  }
}

void Space::assignShellVsVoid(){
  if (Ctrl::getInstance()->getAbortFlag()){return;}
  std::array<unsigned int,3> vxl_index;
  for(vxl_index[0] = 0; vxl_index[0] < getGridsteps()[0]; vxl_index[0]++){
    Ctrl::getInstance()->updateCalculationStatus();
    for(vxl_index[1] = 0; vxl_index[1] < getGridsteps()[1]; vxl_index[1]++){
      for(vxl_index[2] = 0; vxl_index[2] < getGridsteps()[2]; vxl_index[2]++){
        if (Ctrl::getInstance()->getAbortFlag()){return;}
        getTopVxl(vxl_index).evalRelationToVoxels(vxl_index, _max_depth);
      }
    }
    Ctrl::getInstance()->updateProgressBar(int(100*(double(vxl_index[0])+1)/double(getGridsteps()[0])));
  }
}

void Space::sumVolume(std::map<char,double>& volumes, std::vector<Cavity>& cavities, const bool unit_cell){
  // clear all output variables
  volumes.clear();
  // create maps used for tallying voxels
  std::map<char, double> type_tally;
  std::map<unsigned char, double> id_core_tally;
  std::map<unsigned char, double> id_shell_tally;
  // contain the boundaries in which all voxels of a given ID are contained
  std::map<unsigned char, std::array<unsigned,3>> id_min;
  std::map<unsigned char, std::array<unsigned,3>> id_max;

  if(unit_cell){
    setUnitCellIndexes();
  }

  int tally_lvl = unit_cell? 0 : _max_depth;
  std::array<unsigned,3> start_index = unit_cell? _unit_cell_start_index : std::array<unsigned,3>();
  std::array<unsigned,3> end_index = unit_cell? _unit_cell_end_index : getGridstepsOnLvl<unsigned>(tally_lvl);

  // count bottom level voxels per type
  std::array<unsigned int,3> vxl_index;
  for (vxl_index[0] = start_index[0]; vxl_index[0] < end_index[0]; vxl_index[0]++){
    for (vxl_index[1] = start_index[1]; vxl_index[1] < end_index[1]; vxl_index[1]++){
      for (vxl_index[2] = start_index[2]; vxl_index[2] < end_index[2]; vxl_index[2]++){
        getVxlFromGrid(vxl_index, tally_lvl).tallyVoxelsOfType(type_tally, id_core_tally, id_shell_tally, id_min, id_max, vxl_index, tally_lvl);
      }
    }
  }

  if(unit_cell){
    std::array<unsigned int,3> bot_lvl_index;
    double voxel_fraction;
    // for unit cells that are not proportional to _grid_size, it is necessary to only consider a partial voxel volume for the voxels partially overlapping with the unit cell
    for(int n = 0; n < 3; n++){
      int i = n;
      int j = (n+1)%3;
      int k = (n+2)%3;

      // count voxels in the three end faces of the unit cell
      bot_lvl_index[i] = _unit_cell_end_index[i];
      for (bot_lvl_index[j] = _unit_cell_start_index[j]; bot_lvl_index[j] < _unit_cell_end_index[j]; bot_lvl_index[j]++){
        for (bot_lvl_index[k] = _unit_cell_start_index[k]; bot_lvl_index[k] < _unit_cell_end_index[k]; bot_lvl_index[k]++){
          voxel_fraction = _unit_cell_mod_index[i];
          getVxlFromGrid(bot_lvl_index, tally_lvl).tallyVoxelsOfType(type_tally, id_core_tally, id_shell_tally, id_min, id_max, bot_lvl_index, tally_lvl, voxel_fraction);
        }
      }

      // count voxels in the three edges connecting end faces of the unit cell
      bot_lvl_index[j] = _unit_cell_end_index[j];
      for (bot_lvl_index[k] = _unit_cell_start_index[k]; bot_lvl_index[k] < _unit_cell_end_index[k]; bot_lvl_index[k]++){
        voxel_fraction = _unit_cell_mod_index[i]*_unit_cell_mod_index[j];
        getVxlFromGrid(bot_lvl_index, tally_lvl).tallyVoxelsOfType(type_tally, id_core_tally, id_shell_tally, id_min, id_max, bot_lvl_index, tally_lvl, voxel_fraction);
      }
    }

    // add last end vertex voxel
    for(int i = 0; i < 3; i++){
      bot_lvl_index[i] = _unit_cell_end_index[i];
    }
    voxel_fraction = _unit_cell_mod_index[0]*_unit_cell_mod_index[1]*_unit_cell_mod_index[2];
    getVxlFromGrid(bot_lvl_index, tally_lvl).tallyVoxelsOfType(type_tally, id_core_tally, id_shell_tally, id_min, id_max, bot_lvl_index, tally_lvl, voxel_fraction);
  }

  // calculate the volume of a single bottom level voxel
  double unit_volume = pow(getVxlSize(),3);
  // convert from units of bottom level voxels to units of volume
  for (auto& [type,tally] : type_tally) {
    volumes[type] = tally * unit_volume;
  }
  // convert from units of bottom level voxels to units of volume
  // convert from index to spatial coordinates
  // copy values from map to vector for more efficient storage and access
  // ignore id == 0; this is not a cavity but all voxels that are neither core nor shell
  for (auto& [id,tally] : id_core_tally) {
    if (id == 0){continue;}
    cavities[id-1].core_vol = tally * unit_volume;
    cavities[id-1].shell_vol = id_shell_tally[id] * unit_volume;
    cavities[id-1].id = id;
    for (char i = 0; i < 3; ++i){
      cavities[id-1].min_bound[i] = getOrigin()[i] + getVxlSize()*id_min[id][i];
      cavities[id-1].max_bound[i] = getOrigin()[i] + getVxlSize()*(id_max[id][i]+1);
      cavities[id-1].min_index[i] = id_min[id][i];
      cavities[id-1].max_index[i] = id_max[id][i];
    }
  }
  // remove cavities with volume equal to zero (artefacts from unit cell mode)
  for (auto it = cavities.begin(); it != cavities.end(); it++)
  {
    if ((*it).getVolume() == 0){
      cavities.erase(it--);
    }
  }
}

void Space::setUnitCellIndexes(){
  for(int i = 0; i < 3; i++){
    // +0.5 to avoid rounding errors
    _unit_cell_start_index[i] = int(0.5 - _cart_min[i]/_grid_size);
    // no rounding because _unit_cell_mod_index will correct any rounding error in _unit_cell_end_index
    _unit_cell_end_index[i] = _unit_cell_start_index[i] + int(_unit_cell_limits[i]/_grid_size);
    // warning: std::fmod() provided a wrong value in some cases so the fmod calculation is done manually
    _unit_cell_mod_index[i] = custom_fmod(_unit_cell_limits[i],_grid_size) / _grid_size;
   }
}

//////////////////
// SURFACE AREA //
//////////////////

// overloaded for full molecule surfaces
// sets the appropriate start and end indices
double Space::calcSurfArea(const std::vector<char>& types){
  std::array<unsigned,3> start_index = _unit_cell? _unit_cell_start_index : std::array<unsigned,3>({0,0,0});
  std::array<unsigned,3> end_index   = _unit_cell? _unit_cell_end_index   : getGrid(0).getNumElements<unsigned>();
  double surface = tallySurface(types, start_index, end_index);
  // scale the surface area in squared gridstep units
  return (surface * (_grid_size*_grid_size));
}

// overload for cavity surfaces
// solid types MUST also have appropriate ID!
double Space::calcSurfArea(const std::vector<char>& types, const unsigned char id, std::array<unsigned int,3> start_index, std::array<unsigned int,3> end_index){
  if(Ctrl::getInstance()->getAbortFlag()){return 0;}
  // the surface area is counted between voxels, thus we need to check voxels around the limits of the cavity
  if(!_unit_cell){
    for(char i = 0; i < 3; i++){
      std::array<unsigned long,3> n_elements = getGrid(0).getNumElements();
      if(start_index[i] > 0){start_index[i]--;}
      // increase end_index twice because it should be above the range of indexes checked like vector and array sizes in C++
      if(end_index[i] < n_elements[i]){end_index[i]++;}
      if(end_index[i] < n_elements[i]){end_index[i]++;}
    }
  }
  // for unit cell analysis, surfaces outside the unit cell should not be included
  else{
    for(char i = 0; i < 3; i++){
      if(start_index[i] > _unit_cell_start_index[i]){start_index[i]--;}
      // increase end_index twice because it should be above the range of indexes checked like vector and array sizes in C++
      if(end_index[i] < _unit_cell_end_index[i]){end_index[i]++;}
      if(end_index[i] < _unit_cell_end_index[i]){end_index[i]++;}
    }
  }
  double surface = tallySurface(types, start_index, end_index, id, true);
  // scale the surface area in squared gridstep units
  return (surface * (_grid_size*_grid_size));
}

double Space::tallySurface(const std::vector<char>& types, std::array<unsigned int,3>& start_index, std::array<unsigned int,3>& end_index, const unsigned char id, const bool cavity){
  double surface = 0;

  // loop over all voxels within range minus one in each direction because the +1 neighbors will be checked at the same time
  std::array<unsigned int,3> index;
  Ctrl::getInstance()->updateCalculationStatus();
  for(index[2] = start_index[2]; index[2] < end_index[2]-1; index[2]++){
    for(index[1] = start_index[1]; index[1] < end_index[1]-1; index[1]++){
      if(Ctrl::getInstance()->getAbortFlag()){return 0;}
      for(index[0] = start_index[0]; index[0] < end_index[0]-1; index[0]++){
        surface += SurfaceLUT::configToArea(evalMarchingCubeConfig(index, types, id, cavity));
      }
    }
  }
  if(_unit_cell){
    /* the surface area is counted between voxels, thus the borders of the unit cell should include partial surface area by configuration
    since the surface area is not homogeneous over the voxel, the surface area from the borders will be an approximation
    -1,-1,-1 *1/8 on 1 vertex
    -1,-1,i  *1/4 on 3 edges
    -1,i,i   *1/2 on 3 faces
    -1,n,i   *((1/4)+(mod/2)) on 6 edges
    -1,-1,n  *((1/8)+(mod/4)) on 3 vertices
    -1,n,n   *((1/8)+(modA/4)+(modB/4)+(modA*modB/2)) on 3 vertices
    n,i,i    *((1/2)+mod) on 3 faces
    n,n,i    *((1/4)+(modA/2)+(modB/2)+(modA*modB)) on 3 edges
    n,n,n    *((1/8)+(modA/4)+(modB/4)+(modC/4)+(modA*modB/2)+(modA*modC/2)+(modB*modC/2)+(modA*modB*modC)) on 1 vertex
    */

    // add first -1,-1,-1 vertex
    for(int i = 0; i < 3; i++){
      index[i] = _unit_cell_start_index[i]-1;
    }
    surface += (SurfaceLUT::configToArea(evalMarchingCubeConfig(index, types, id, cavity)) * 0.125);

    // add last n,n,n vertex
    for(int i = 0; i < 3; i++){
      index[i] = _unit_cell_end_index[i]-1;
    }
    surface += (SurfaceLUT::configToArea(evalMarchingCubeConfig(index, types, id, cavity)) *
                (0.125 +
                 ((_unit_cell_mod_index[0] + _unit_cell_mod_index[1] + _unit_cell_mod_index[2])/4) +
                 (_unit_cell_mod_index[0] * _unit_cell_mod_index[1]/2) +
                 (_unit_cell_mod_index[0] * _unit_cell_mod_index[2]/2) +
                 (_unit_cell_mod_index[1] * _unit_cell_mod_index[2]/2) +
                 (_unit_cell_mod_index[0] * _unit_cell_mod_index[1] * _unit_cell_mod_index[2])));

    // swap x,y,z
    for(int n = 0; n < 3; n++){
      int i = n;
      int j = (n+1)%3;
      int k = (n+2)%3;

      // add the first three intermediate vertices -1,-1,n
      index[i] = _unit_cell_start_index[i]-1;
      index[j] = _unit_cell_start_index[j]-1;
      index[k] = _unit_cell_end_index[k]-1;
      surface += (SurfaceLUT::configToArea(evalMarchingCubeConfig(index, types, id, cavity)) * (0.125 + (_unit_cell_mod_index[k]/4)));

      // add the last three intermediate vertices -1,n,n
      index[i] = _unit_cell_start_index[i]-1;
      index[j] = _unit_cell_end_index[j]-1;
      index[k] = _unit_cell_end_index[k]-1;
      surface += (SurfaceLUT::configToArea(evalMarchingCubeConfig(index, types, id, cavity)) *
                  (0.125 +
                   ((_unit_cell_mod_index[j] + _unit_cell_mod_index[k])/4) +
                   (_unit_cell_mod_index[j] * _unit_cell_mod_index[k]/2)));

      // add the three start edges -1,-1,k
      index[i] = _unit_cell_start_index[i]-1;
      index[j] = _unit_cell_start_index[j]-1;
      for (index[k] = _unit_cell_start_index[k]; index[k] < _unit_cell_end_index[k]-1; index[k]++){
        surface += (SurfaceLUT::configToArea(evalMarchingCubeConfig(index, types, id, cavity)) * 0.25);
      }

      // add the three end edges n,n,k
      index[i] = _unit_cell_end_index[i]-1;
      index[j] = _unit_cell_end_index[j]-1;
      for (index[k] = _unit_cell_start_index[k]; index[k] < _unit_cell_end_index[k]-1; index[k]++){
        surface += (SurfaceLUT::configToArea(evalMarchingCubeConfig(index, types, id, cavity)) *
                    (0.25 +
                     ((_unit_cell_mod_index[j] + _unit_cell_mod_index[k])/2) +
                     (_unit_cell_mod_index[j] * _unit_cell_mod_index[k])));
      }

      // add the three start faces -1,j,k
      index[i] = _unit_cell_start_index[i]-1;
      for (index[j] = _unit_cell_start_index[j]; index[j] < _unit_cell_end_index[j]-1; index[j]++){
        for (index[k] = _unit_cell_start_index[k]; index[k] < _unit_cell_end_index[k]-1; index[k]++){
          surface += (SurfaceLUT::configToArea(evalMarchingCubeConfig(index, types, id, cavity)) * 0.5);
        }
      }

      // add the three end faces n,j,k
      index[i] = _unit_cell_end_index[i]-1;
      for (index[j] = _unit_cell_start_index[j]; index[j] < _unit_cell_end_index[j]-1; index[j]++){
        for (index[k] = _unit_cell_start_index[k]; index[k] < _unit_cell_end_index[k]-1; index[k]++){
          surface += (SurfaceLUT::configToArea(evalMarchingCubeConfig(index, types, id, cavity)) * (0.5 + _unit_cell_mod_index[i]));
        }
      }

      // add the six intermediate edges -1,n,k and n,-1,k
      index[i] = _unit_cell_start_index[i]-1;
      index[j] = _unit_cell_end_index[j]-1;
      for (index[k] = _unit_cell_start_index[k]; index[k] < _unit_cell_end_index[k]-1; index[k]++){
        surface += (SurfaceLUT::configToArea(evalMarchingCubeConfig(index, types, id, cavity)) * (0.25 + (_unit_cell_mod_index[j]/2)));
      }
      index[i] = _unit_cell_end_index[i]-1;
      index[j] = _unit_cell_start_index[j]-1;
      for (index[k] = _unit_cell_start_index[k]; index[k] < _unit_cell_end_index[k]-1; index[k]++){
        surface += (SurfaceLUT::configToArea(evalMarchingCubeConfig(index, types, id, cavity)) * (0.25 + (_unit_cell_mod_index[i]/2)));
      }
    }
  }
  return surface;
}

bool isSolid(const Voxel&, const std::vector<char>&);

unsigned char Space::evalMarchingCubeConfig(const std::array<unsigned int,3>& index, const std::vector<char>& types, const unsigned char id, const bool cavity){
  unsigned char config = 0; // configuration of the marching cube stored as a byte
  // check the starting voxel and its 7 neighbors to define a marching cube configuration
  std::array<unsigned,3> subindex;
  for(unsigned int x = 0; x < 2; x++){
    subindex[0] = index[0] + x;
    for(unsigned int y = 0; y < 2; y++){
      subindex[1] = index[1] + y;
      for(unsigned int z = 0; z < 2; z++){
        subindex[2] = index[2] + z;
        // condition for a bit to be true in the byte
        bool bit_state = isSolid(getVxlFromGrid(subindex, 0), types);
        if (cavity) {bit_state &= getVxlFromGrid(subindex, 0).getID() == id;}
        setBit(config, z + 2*y + 4*x, bit_state);
      }
    }
  }
  return config;
}

inline bool isSolid(const Voxel& vxl, const std::vector<char>& solid_types){
  return std::find(solid_types.begin(), solid_types.end(), vxl.getType()) != solid_types.end();
}

//////////////////////
// ACCESS FUNCTIONS //
//////////////////////

std::array<double,3> Space::getMin() const {
  return _cart_min;
}

std::array<double,3> Space::getOrigin() const {
  return getMin();
}

std::array<double,3> Space::getMax() const {
  return _cart_max;
}

std::array<double,3> Space::getSize(){
  std::array<double,3> size;
  for(int dim = 0; dim < 3; dim++){
    size[dim] = _cart_max[dim] - _cart_min[dim];
  }
  return size;
}

double Space::getVxlSize() const {
  return _grid_size;
}

const Container3D<Voxel>& Space::getGrid(const unsigned lvl) const{
  return _grid[lvl];
}

/////////////////
// GET ELEMENT //
/////////////////

Voxel& Space::getVxlFromGrid(const unsigned int i, unsigned lvl){
  return _grid[lvl].getElement(i);
}

Voxel& Space::getVxlFromGrid(const unsigned int x, const unsigned int y, const unsigned int z, unsigned lvl){
  return _grid[lvl].getElement(x,y,z);
}

Voxel& Space::getVxlFromGrid(const std::array<unsigned int,3> arr, unsigned lvl){
  return _grid[lvl].getElement(arr);
}

Voxel& Space::getVxlFromGrid(const std::array<int,3> arr, unsigned lvl){
  return _grid[lvl].getElement(arr);
}

Voxel& Space::getTopVxl(const unsigned int i){
  return getVxlFromGrid(i, _max_depth);
}

Voxel& Space::getTopVxl(const unsigned int x, const unsigned int y, const unsigned int z){
  return getVxlFromGrid(x, y, z, _max_depth);
}

Voxel& Space::getTopVxl(const std::array<unsigned int,3> arr){
  return getVxlFromGrid(arr, _max_depth);
}

Voxel& Space::getTopVxl(const std::array<int,3> arr){
  return getVxlFromGrid(arr, _max_depth);
}

/////////////////

// check whether coord is inside grid bounds
bool Space::isInBounds(const std::array<int,3>& coord, const unsigned lvl){
  for (char i = 0; i < 3; i++){
    if(coord[i] < 0 || coord[i] >= getGridsteps()[i] * std::pow(2,_max_depth-lvl)){return false;}
  }
  return true;
}
bool Space::isInBounds(const std::array<unsigned,3>& coord, const unsigned lvl){
  for (char i = 0; i < 3; i++){
    if(coord[i] < 0 || coord[i] >= getGridsteps()[i] * std::pow(2,_max_depth-lvl)){return false;}
  }
  return true;
}


const std::array<unsigned long,3> Space::getGridsteps(){
  return getGridstepsOnLvl(_max_depth);
}

std::array<std::array<unsigned int,3>,2> Space::getUnitCellIndexes(){
  return {_unit_cell_start_index, _unit_cell_end_index};
}

unsigned long Space::totalVxlOnLvl(const int lvl) const{
  unsigned long total = 1;
  const std::array<unsigned long,3> gridsteps = getGridstepsOnLvl(lvl);
  for (char i = 0; i < 3; i++){
    total *= gridsteps[i];
  }
  return total;
}

////////////////
// PRINT GRID //
////////////////

std::array<unsigned long,3> makeIndices(std::array<unsigned long,3> indices, int depth){
  for (char i = 0; i < 3; i++){
    indices[i] *= pow(2,depth);
  }
  return indices;
}

// displays voxel grid types as matrix in the terminal. useful for debugging
void Space::printGrid(){

  bool disp_id = false;
  int depth = 0;
  std::array<unsigned long,3> indices = makeIndices(getGridsteps(), depth);

  unsigned int x_min = 0;
  unsigned int y_min = 0;

  unsigned int x_max = ((indices[0] >= 50)? 50: indices[0]);
  unsigned int y_max = ((indices[1] >= 25)? 25: indices[1]);

  unsigned int z = 0;
  std::cout << "Enter 'q' to quit; 'w', 'a', 's', 'd' for directional input; 'c' to continue in z direction; 'r' to go back in z direction. Enter '+' or '-' to change the octree depth." << std::endl;
  char usr_inp = '\0';
  while (usr_inp != 'q'){

    if (usr_inp == '#'){
      disp_id = !disp_id;
    }

    // if depth is changed, reset view
    if (usr_inp == '+' || usr_inp == '-'){
      if (usr_inp == '+' && depth < _max_depth){depth++;}
      else if (usr_inp == '-' && depth > 0){depth--;}
      indices = makeIndices(getGridsteps(),depth);
      x_min = 0;
      y_min = 0;
      x_max = ((indices[0] >= 50)? 50: indices[0]);
      y_max = ((indices[1] >= 25)? 25: indices[1]);
      z = 0;
    }

    // x and y coordinates
    if (usr_inp == 'a'){x_min--; x_max--;}
    else if (usr_inp == 'd'){x_min++; x_max++;}
    else if (usr_inp == 's'){y_min++; y_max++;}
    else if (usr_inp == 'w'){y_min--; y_max--;}

    if (x_min < 0){x_min++; x_max++;}
    if (y_min < 0){y_min++; y_max++;}
    if (x_max > indices[0]){x_min--; x_max--;}
    if (y_max > indices[1]){y_min--; y_max--;}

    // z coordinate
    if (usr_inp == 'r'){
      if (z==0){
        std::cout << "z position at min. " << std::endl;
      }
      else{z--;}
    }
    else if (usr_inp == 'c'){
      z++;
      if (z >= indices[2]){
        z--;
        std::cout << "z position at max. " << std::endl;
      }
    }

    // print matrix
    for(unsigned int y = y_min; y < y_max; y++){
      for(unsigned int x = x_min; x < x_max; x++){
        char to_print;
        if (disp_id){
          to_print = ('0' + getVxlFromGrid(x,y,z, _max_depth-depth).getID());
        }
        else{
          to_print = (getVxlFromGrid(x,y,z,_max_depth-depth).getType() == 0b00000011)? 'A' : 'O';
          if (!readBit(getVxlFromGrid(x,y,z,_max_depth-depth).getType(),0)){to_print = '?';}
          if (readBit(getVxlFromGrid(x,y,z,_max_depth-depth).getType(),7)){to_print = 'M';}
          if (getVxlFromGrid(x,y,z,_max_depth-depth).getType() == 0b00000101){to_print = 'X';}
          if (getVxlFromGrid(x,y,z,_max_depth-depth).getType() == 0b00001001){to_print = 'P';}
          if (getVxlFromGrid(x,y,z,_max_depth-depth).getType() == 0b00010001){to_print = 'S';}
          if (getVxlFromGrid(x,y,z,_max_depth-depth).getType() == 0b00100001){to_print = 'p';}
          if (getVxlFromGrid(x,y,z,_max_depth-depth).getType() == 0b01000001){to_print = 's';}
        }

        std::cout << to_print << " ";
      }
      std::cout << std::endl;
    }

    // get user input
    std::cout << std::endl;
    std::cout << "INPUT: ";
    std::cin >> usr_inp;
  }
}

///////////////////////////
// SURFACE LOOK UP TABLE //
///////////////////////////

const constexpr std::array<unsigned char,256> SurfaceLUT::types_by_config = {1,2,2,3,2,3,4,6,2,4,3,6,3,6,6,9,2,3,4,6,4,6,8,10,5,7,7,13,7,13,11,6,2,4,3,6,5,7,7,13,4,8,6,10,7,11,13,6,3,6,6,9,7,13,11,6,7,11,13,6,12,7,7,3,2,4,5,7,3,6,7,13,4,8,7,11,6,10,13,6,3,6,7,13,6,9,11,6,7,11,12,7,13,6,7,3,4,8,7,11,7,11,12,7,8,14,11,8,11,8,7,4,6,10,13,6,13,6,7,3,11,8,7,4,7,4,5,2,2,5,4,7,4,7,8,11,3,7,6,13,6,13,10,6,4,7,8,11,8,11,14,8,7,12,11,7,11,7,8,4,3,7,6,13,7,12,11,7,6,11,9,6,13,7,6,3,6,13,10,6,11,7,8,4,13,7,6,3,7,5,4,2,3,7,7,12,6,13,11,7,6,11,13,7,9,6,6,3,6,13,11,7,10,6,8,4,13,7,7,5,6,3,4,2,6,11,13,7,13,7,7,5,10,8,6,4,6,4,3,2,9,6,6,3,6,3,4,2,6,4,3,2,3,2,2,1};
// Theoretical values from https://doi.org/10.1007/978-3-540-39966-7_33
// const constexpr std::array<double, 15> SurfaceLUT::area_by_type = {0,0,0.2118,0.669,0.4236,0.4236,0.9779,0.8808,0.6354,0.927,1.2706,1.1897,1.338,1.5731,0.8472};
// Semi-empirical values from https://doi.org/10.1016/j.imavis.2004.06.012
const constexpr std::array<double, 15> SurfaceLUT::area_by_type = {0,0,0.636,0.669,1.272,1.272,0.5537,1.305,1.908,0.927,0.4222,1.1897,1.338,1.5731,2.544};
unsigned char SurfaceLUT::configToType(unsigned char config) {
  return types_by_config[config];
}
double SurfaceLUT::typeToArea(unsigned char type) {
  return area_by_type[type];
}
double SurfaceLUT::configToArea(unsigned char config) {
  return area_by_type[types_by_config[config]];
}
