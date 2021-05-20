#include "model.h"
#include "atom.h"
#include "controller.h"
#include "misc.h"
#include "container3d.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <filesystem>

///////////////////
// RESULT REPORT //
///////////////////

// TODO remove if completely obselete
bool Model::createOutputFolder(std::string file_name){
  // folder name based on current time to avoid overwriting output files with successive calculations
  _time_stamp = timeNow();
  output_folder = "./output/" + file_name + " MoloVol/" + _time_stamp + "/";
  // TODO create directories didn't seem to work on MacOS, need to fix this issue later
  if(std::filesystem::create_directories(output_folder)){
    return true;
  }
  else{
    output_folder = "./";
    return false;
  }
  return true;
}

void Model::createReport(){
  createReport(output_folder+"/MoloVol report " + _time_stamp +".txt");
}

void Model::createReport(std::string path){
  std::ofstream output_report(path);
  output_report << "\n";
  output_report << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n";
  output_report << "   MM           MM            LL           VV           VV           LL   \n";
  output_report << "   MMM         MMM            LL            VV         VV            LL   \n";
  output_report << "   MMMM       MMMM            LL             VV       VV             LL   \n";
  output_report << "   MM MM     MM MM            LL              VV     VV              LL   \n";
  output_report << "   MM  MM   MM  MM    OOOO    LL    OOOO       VV   VV       OOOO    LL   \n";
  output_report << "   MM   MM MM   MM   OO  OO   LL   OO  OO       VV VV       OO  OO   LL   \n";
  output_report << "   MM    MMM    MM   OO  OO   LL   OO  OO        VVV        OO  OO   LL   \n";
  output_report << "   MM     M     MM    OOOO    LL    OOOO          V          OOOO    LL   \n\n";
  output_report << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n";
  output_report << "Source code available at https://github.com/jmaglic/MoloVol under the MIT licence\n";
  output_report << "Copyright © 2020-2021 Jasmin B. Maglic, Roy Lavendomme\n\n";
  output_report << "MoloVol program: calculation results report\n";
  output_report << "version: " + Ctrl::s_version + "\n\n";
  output_report << "Time of the calculation: " << _time_stamp << "\n";
  output_report << "Structure file analyzed: " << _data.atom_file_path << "\n";
  output_report << "Chemical formula: " + _data.chemical_formula << "\n";
  output_report << "Duration of the calculation: " << _data.getTime() << " s\n";

  output_report << "\n\n\t////////////////////////////\n";
  output_report << "\t// Calculation parameters //\n";
  output_report << "\t////////////////////////////\n\n";
  if(fileExtension(_data.atom_file_path) == "pdb"){
    if(_data.analyze_unit_cell){
      output_report << "Analyze crystal structure unit cell\n";
    }
    output_report << std::string((_data.inc_hetatm)? "Include" : "Exclude") + " HETATM from pdb file\n";
  }
  if(_data.probe_mode){
    output_report << "Probe mode: two probes\n";
    output_report << "Probe 1 radius: " << getProbeRad1() << " Å\n";
    output_report << "Probe 2 radius: " << getProbeRad2() << " Å\n";
  }
  else{
    output_report << "Probe mode: one probe\n";
    output_report << "Probe radius: " << getProbeRad1() << " Å\n";
  }
  output_report << "Grid step size (resolution): " << _data.grid_step << " Å\n";
  output_report << "Maximum tree depth (algorithm acceleration): " << _data.max_depth << "\n";
  output_report << "Elements radii:\n";
  for(std::unordered_map<std::string, double>::iterator it = radius_map.begin(); it != radius_map.end(); it++){
    if(isIncluded(it->first, _data.included_elements)){
      output_report << it->first << " : " << it->second << " Å\n";
    }
  }

  // TODO consider crystal unit cell report with density, volumes per gram and volume ratio

  output_report << "\n\n\t//////////////////////////////\n";
  output_report << "\t// Total Volumes calculated //\n";
  output_report << "\t//////////////////////////////\n\n";
  if(_data.analyze_unit_cell){
    output_report << "Orthogonal(ized) unit cell axes: " << _cart_matrix[0][0] << " Å, " << _cart_matrix[1][1] << " Å, " << _cart_matrix[2][2] << " Å\n";
    output_report << "Total unit cell volume: " << _cart_matrix[0][0]*_cart_matrix[1][1]*_cart_matrix[2][2] << " Å^3\n";
  }
  output_report << "Van der Waals volume: " << _data.volumes[0b00000011] << " Å^3\n";
  output_report << "Excluded void volume: " << _data.volumes[0b00000101] << " Å^3\n";
  output_report << "Molecular volume (vdw + excluded void): " << _data.volumes[0b00000011] + _data.volumes[0b00000101] << " Å^3\n";
  output_report << "Probe 1 core volume: " << _data.volumes[0b00001001] << " Å^3\n";
  output_report << "Probe 1 shell volume: " << _data.volumes[0b00010001] << " Å^3\n";
  if(_data.probe_mode){
    output_report << "Internal cavities and pockets volume (probe 1 core + shell): " << _data.volumes[0b00001001] + _data.volumes[0b00010001] << " Å^3\n";
    output_report << "Probe 2 core volume: " << _data.volumes[0b00100001] << " Å^3\n";
    output_report << "Probe 2 shell volume: " << _data.volumes[0b01000001] << " Å^3\n";
  }

  if(_data.calc_surface_areas){
    output_report << "\n\n\t////////////////////////////////////\n";
    output_report << "\t// Total Surface Areas calculated //\n";
    output_report << "\t////////////////////////////////////\n\n";
    output_report << "Van der Waals surface: " << _data.surf_vdw << " Å^2\n";
    if(_data.probe_mode){
      output_report << "Molecular surface: " << _data.surf_molecular << " Å^2 (probes 1 and 2 excluded surface, similar to the Connolly surface)\n";
    }
    output_report << "Probe 1 excluded surface: " << _data.surf_probe_excluded << " Å^2 (similar to the Connolly surface)\n";
    output_report << "Probe 1 accessible surface: " << _data.surf_probe_accessible << " Å^2 (similar to the Lee-Richards surface)\n";
  }

  if(!_data.cavities.empty()){
    output_report << "\n\n\t///////////////////////////////\n";
    output_report << "\t// Cavities and pockets data //\n";
    output_report << "\t///////////////////////////////\n\n";

    if(_data.cavities.size() >= 255){
      output_report << "!!! WARNING !!!\n";
      output_report << "Maximum number of cavities reached. Some cavities might be missing.\n";
      output_report << "To solve this issue, change probe radii (e.g. smaller probe 2 and/or larger probe 1).\n\n";
    }
    output_report << "Note 1:\tPockets and isolated cavities are not differentiated yet.\n";
    output_report << "\tThis feature might be added in future versions if requested by the community.\n";
    output_report << "Note 2:\tSeparate cavities are defined by space accessible to the core of probe 1.\n";
    output_report << "\tTwo cavities can be in contact but if a probe cannot pass from one to the other, they are considered separated.\n";
    output_report << "Note 3:\tIn single probe mode, the first 'cavity' consists of the outside space and pockets.\n";
    output_report << "Note 4:\tSome very small isolated chunks of probe 1 cores can be detected and lead to small cavities.\n";
    output_report << "Note 5:\tProbe occupied volume correspond to empty space as defined by the molecular surface (similar to the Connolly surface).\n";
    output_report << "Note 6:\tProbe accessible volume correspond to empty space as defined\n";
    output_report << "\tby the surface accessible to its core (similar to the Lee-Richards surface).\n";
    output_report << "Note 7:\tFor a detailed shape of each cavity, check the surface maps.\n\n";

    output_report << "Cavity\tOccupied\tAccessible\t" << (_data.calc_surface_areas ? "Excluded\tAccessible\t" : "") << "Cavity center coordinates (Å)\n";
    output_report << "ID\tVolume (Å^3)\tVolume (Å^3)\t" << (_data.calc_surface_areas ? "Surface (Å^2)\tSurface (Å^2)\t" : "") << "x\ty\tz\n";
    for(unsigned int i = 0; i < _data.cavities.size(); i++){
      std::array<double,3> cav_center = _data.getCavCenter(i);
      // default precision is 6, which means that double values will take less than a tab space
      output_report << i+1 << "\t"
                    << _data.cavities[i].getVolume() << "\t\t"
                    << _data.cavities[i].core_vol << "\t\t";
      if(_data.calc_surface_areas){
        output_report << _data.cavities[i].surf_shell << "\t\t"
                      << _data.cavities[i].surf_core << "\t\t";
      }
      output_report << cav_center[0] << "\t" << cav_center[1] << "\t" << cav_center[2] << "\n";
    }
  }

  if(_data.make_full_map || _data.make_cav_maps){
    output_report << "\n\n\t/////////////////////////////\n";
    output_report << "\t// Surface map information //\n";
    output_report << "\t/////////////////////////////\n\n";
    output_report << "The surface map files generated (.dx, OpenDX format) can be opened with\n";
    output_report << "PyMOL, USCF Chimera or USCF ChimeraX.\n\n";
    output_report << "Use the following isosurface levels to visualize the desired surface:\n";
    output_report << "Level 0.5 : Van der Waals surface\n";
    if(_data.probe_mode){
      output_report << "Level 1.5 : Molecular surface (probes 1 and 2 excluded, similar to the Connolly surface)\n";
      output_report << "Level 3 : Internal cavities and pockets (probe 1 excluded, similar to the Connolly surface but only 'inside')\n";
      output_report << "Level 5 : Probe 1 accessible surface (similar to Lee-Richards molecular surface but only 'inside')\n";
    }
    else{
      output_report << "Level 2 : Molecular surface (probe 1 excluded, similar to the Connolly surface)\n";
      output_report << "Level 5 : Probe 1 accessible surface (similar to the Lee-Richards molecular surface)\n";
    }
    output_report << "\nFor help on how to vizualize maps:\n";
    output_report << " - in PyMOL, simply open the map file then click 'A' in the right panel, choose mesh or surface and select the level.\n";
    output_report << "   For more information, check https://pymolwiki.org/index.php/Isomesh and https://pymolwiki.org/index.php/Isosurface \n";
    output_report << " - in USCF Chimera, check https://www.cgl.ucsf.edu/chimera/docs/ContributedSoftware/volumeviewer/volumeviewer.html \n";
    output_report << " - in USCF ChimeraX, check https://www.cgl.ucsf.edu/chimerax/docs/user/tools/volumeviewer.html \n";

    output_report << "\nIf you wish to visualize the structure file in PyMOL with the same element radii as in the MoloVol calculation,\n";
    output_report << "paste the following command lines (all at once) in the command prompt of PyMOL after opening the structure file.\n\n";

    for(std::unordered_map<std::string, double>::iterator it = radius_map.begin(); it != radius_map.end(); it++){
      if(isIncluded(it->first, _data.included_elements)){
      output_report << "alter (elem " << it->first << "),vdw=" << it->second << "\n";
      }
    }
    output_report << "rebuild\n\n";
  }
  output_report.close();
}

//////////////////////
// STRUCTURE OUTPUT //
//////////////////////

void Model::writeXYZfile(std::vector<std::tuple<std::string, double, double, double>> &atom_coordinates, std::string output_type){
  std::ofstream output_structure(output_folder+"/structure_"+output_type+"_"+ _time_stamp +".xyz");
  output_structure << output_type << "\nStructure generated with MoloVol\n\n";
  for(size_t i = 0; i < atom_coordinates.size(); i++){
    output_structure << std::get<0>(atom_coordinates[i]) << " ";
    output_structure << std::to_string(std::get<1>(atom_coordinates[i])) << " ";
    output_structure << std::to_string(std::get<2>(atom_coordinates[i])) << " ";
    output_structure << std::to_string(std::get<3>(atom_coordinates[i])) << "\n";
  }
  output_structure.close();
}

////////////////////////
// SURFACE MAP OUTPUT //
////////////////////////

void Model::writeTotalSurfaceMap(){
  // save commonly used variable
  std::array<unsigned long int,3> n_elements = _cell.getGrid(0).getNumElements();
  double vxl_length = _cell.getVxlSize();
  std::array<double,3> cell_min = _cell.getMin();
  std::array<double,3> origin;
  std::array<unsigned int,3> start_index = {0,0,0};
  std::array<unsigned int,3> end_index;
  for(int i = 0; i < 3; i++){
    end_index[i] = n_elements[i];
  }

  // in two probes mode, the outer space occupied by the second probe is useless for the surface map
  // thus the surface map can be reduced on each side by the radius of probe 2
  if(_data.probe_mode){
    for(int i = 0; i < 3; i++){
      start_index[i] += getProbeRad2()/vxl_length;
      end_index[i] -= getProbeRad2()/vxl_length;
      n_elements[i] = end_index[i] - start_index[i];
    }
  }

  // in unit cell analysis mode, only the content of the unit cell should be displayed in the surface map
  // one extra voxel is added on each side to see the surface up to the boundary of the unit cell
  if(_data.analyze_unit_cell){
    for(int i = 0; i < 3; i++){
      // +0.5 to avoid rounding errors
      start_index[i] = int(0.5 - cell_min[i]/vxl_length)-1;
      end_index[i] = 2 + start_index[i] + int(0.5 + _cart_matrix[i][i]/vxl_length);
      n_elements[i] = end_index[i] - start_index[i];
    }
  }

  for (int i = 0; i < 3; i++){
    origin[i] = cell_min[i] + ((double(start_index[i]) + 0.5) * vxl_length);
  }

  std::string file_name = "/full_surface_map_" + _time_stamp + ".dx";
  writeSurfaceMap(file_name, vxl_length, n_elements, origin, start_index, end_index);
}

void Model::writeCavitiesMaps(){
  // save commonly used variable
  double vxl_length = _cell.getVxlSize();
  std::array<double,3> cell_min = _cell.getMin();
  std::array<double,3> origin;
  std::array<unsigned int,3> start_index;
  std::array<unsigned int,3> end_index;

  // loop over each cavity id
  for(size_t id = 0; id < _data.cavities.size(); id++){
    std::array<unsigned long int,3> n_elements = _cell.getGrid(0).getNumElements();
    start_index = _data.cavities[id].min_index;
    end_index = _data.cavities[id].max_index;
    // increase size of surface map grid by 1 voxel in each direction to avoid having surfaces on the border of the map
    for(char i = 0; i < 3; i++){
      if(start_index[i] > 0){start_index[i]--;}
      // increase end_index twice because it should be above the range of indexes checked like vector and array sizes in C++
      if(end_index[i] < n_elements[i]){end_index[i]++;}
      if(end_index[i] < n_elements[i]){end_index[i]++;}
      n_elements[i] = end_index[i] - start_index[i];
      origin[i] = cell_min[i] + ((double(start_index[i]) + 0.5) * vxl_length);
    }
    std::string file_name = "/cav" + std::to_string(id+1) + "_surface_map_" + _time_stamp + ".dx";
    writeSurfaceMap(file_name, vxl_length, n_elements, origin, start_index, end_index, true, id);
  }
}

void Model::writeSurfaceMap(std::string file_name,
                            double vxl_length,
                            std::array<unsigned long int,3> n_elements,
                            std::array<double,3> origin,
                            std::array<unsigned int,3> start_index,
                            std::array<unsigned int,3> end_index,
                            const bool partial_map,
                            const unsigned char id){

  // assemble data
  Container3D<Voxel>* surface_map = &_cell.getGrid(0);

  // create map for assigning numbers to types
  const std::map<char,int> typeToNum =
    {{0b00000011, 0},
     {0b00000101, 1},
     {0b00001001, 6},
     {0b00010001, 4},
     {0b00100001, 2},
     {0b01000001, 2}};

  // create and open new file
  std::ofstream output_file;
  output_file.open(output_folder + file_name);
  // comments
  output_file << "# OpenDX density file generated by MoloVol\n";
  output_file << "# Contains 3D surface map data to read in PyMOL or Chimera\n";
  output_file << "# Data is written in C array order: In grid[x,y,z] the axis x is fastest\n";
  output_file << "# varying, then y, then finally z.\n";
  // line
  output_file << "object 1 class gridpositions counts";
  for (char i = 0; i < 3; i++){
    output_file << ' ' << n_elements[i];
  }
  output_file << "\n";
  // line
  output_file << "origin ";
  for (char i = 0; i < 3; i++){
    output_file << ' ' << origin[i];
  }
  output_file << '\n';
  // 3 lines
  for (char i = 0; i < 3; i++){
    output_file << "delta";
    for (char j = 0; j < 3; j++){
      output_file << ' ' << ((i==j)? vxl_length : 0);
    }
    output_file << '\n';
  }
  // line
  output_file << "object 2 class gridconnections counts";
  for (char i = 0; i < 3; i++){
    output_file << ' ' << n_elements[i];
  }
  output_file << '\n';
  // line
  output_file << "object 3 class array type double rank 0 items " << (n_elements[0]*n_elements[1]*n_elements[2]) << " data follows\n";
  // data
  int column = 0;
  for(unsigned long int x = start_index[0]; x < end_index[0]; x++){
    for(unsigned long int y = start_index[1]; y < end_index[1]; y++){
      for(unsigned long int z = start_index[2]; z < end_index[2]; z++){
        if (typeToNum.count(surface_map->getElement(x,y,z).getType()) != 0){
          if (partial_map? surface_map->getElement(x,y,z).getID() == _data.cavities[id].id : true){
            output_file << typeToNum.find(surface_map->getElement(x,y,z).getType())->second;
          }
          else {
            output_file << 0;
          }
        }
        else { // TODO inform the user that there is something odd with the surface map
          output_file << -2;
        }
        output_file << ((column == 2)? '\n' : ' ');
        column = (column+1)%3;
      }
    }
  }
  if (column != 0) {
    output_file << '\n';
  }
  output_file << "attribute \"dep\" string \"positions\"\n";
  output_file << "object \"density\" class field\n";
  output_file << "component \"positions\" value 1\n";
  output_file << "component \"connections\" value 2\n";
  output_file << "component \"data\" value 3";

  // close the file
  output_file.close();
}
