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
  calc_time = timeNow();
  output_folder = "./output/" + file_name + " MoloVol/" + calc_time + "/";
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
  std::ofstream output_report(output_folder+"/MoloVol report "+calc_time+".txt");
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
  output_report << "version: alpha\n\n"; // TODO make a variable for the version to simplify updates
  output_report << "Time of the calculation: " << calc_time << "\n";
  output_report << "Structure file analyzed: " << _data.atom_file_path << "\n";
  output_report << "Chemical formula: " + _data.chemical_formula << "\n";
  output_report << "Duration of the calculation: " << _data.total_elapsed_seconds << " s\n";

  output_report << "\n////////////////////////////\n";
  output_report << "// Calculation parameters //\n";
  output_report << "////////////////////////////\n\n";
  if(fileExtension(_data.atom_file_path) == "pdb"){
    if(_data.analyze_unit_cell){
      output_report << "Analyze crystal structure unit cell\n";
    }
    output_report << std::string((_data.inc_hetatm)? "Include" : "Exclude") + " HETATM from pdb file\n";
  }
  if(_data.probe_mode){
    output_report << "Probe mode: two probes\n";
    output_report << "Probe 1 radius: " + std::to_string(_r_probe1) + " A\n";
    output_report << "Probe 2 radius: " + std::to_string(_r_probe2) + " A\n";
  }
  else{
    output_report << "Probe mode: one probe\n";
    output_report << "Probe radius: " + std::to_string(_r_probe1) + " A\n";
  }
  output_report << "Grid step size (resolution): " + std::to_string(_data.grid_step) + " A\n";
  output_report << "Maximum tree depth (algorithm acceleration): " + std::to_string(_data.max_depth) << "\n";
  output_report << "Elements radii:\n";
  for(std::unordered_map<std::string, double>::iterator it = radius_map.begin(); it != radius_map.end(); it++){
    if(isIncluded(it->first, _data.included_elements)){
      output_report << it->first + " : " + std::to_string(it->second) + " A\n";
    }
  }

  // TODO consider crystal unit cell report with density, volumes per gram and volume ratio

  output_report << "\n////////////////////////\n";
  output_report << "// Volumes calculated //\n";
  output_report << "////////////////////////\n\n";
  output_report << "Van der Waals volume: " << std::to_string(_data.volumes[0b00000011]) << " A^3\n";
  output_report << "Excluded void volume: " << std::to_string(_data.volumes[0b00000101]) << " A^3\n";
  output_report << "Molecular volume (vdw + excluded void): " << std::to_string(_data.volumes[0b00000011] + _data.volumes[0b00000101]) << " A^3\n";
  output_report << "Probe 1 core volume: " << std::to_string(_data.volumes[0b00001001]) << " A^3\n";
  output_report << "Probe 1 shell volume: " << std::to_string(_data.volumes[0b00010001]) << " A^3\n";
  if(_data.probe_mode){
    output_report << "Internal cavities and pockets volume (probe 1 core + shell): " << std::to_string(_data.volumes[0b00001001] + _data.volumes[0b00010001]) << " A^3\n";
    output_report << "Probe 2 core volume: " << std::to_string(_data.volumes[0b00100001]) << " A^3\n";
    output_report << "Probe 2 shell volume: " << std::to_string(_data.volumes[0b01000001]) << " A^3\n";
  }

  if(_data.make_full_map || _data.make_cav_maps){
    // TODO add commands to change atomic radii in PyMol
    output_report << "\n/////////////////////////////\n";
    output_report << "// Surface map information //\n";
    output_report << "/////////////////////////////\n\n";
    output_report << "The surface map files generated (.dx, OpenDX format) can be opened with\n";
    output_report << "PyMol, USCF Chimera or USCF ChimeraX.\n\n";
    output_report << "Use the following isosurface levels to visualize the desired surface:\n";
    output_report << "Level 0.5 : Van der Waals surface\n";
    if(_data.probe_mode){
      output_report << "Level 1.5 : Molecular surface (probes 1 and 2 excluded, similar to the Connolly surface)\n";
      output_report << "Level 3 : Internal cavities and pockets (probe 1 excluded, similar to the Connolly surface but only 'inside')\n";
      output_report << "Level 5 : Probe 1 accessible surface (similar to Lee-Richards molecular surface but only 'inside')\n";
    }
    else{
      output_report << "Level 2 : Molecular surface (probe 1 excluded, similar to the Connolly surface)\n";
      output_report << "Level 5 : Probe 1 accessible surface (similar to Lee-Richards molecular surface)\n";
    }
    output_report << "\nFor help on how to vizualize maps:\n";
    output_report << " - in Pymol, simply open the map file then click 'A' in the right panel, choose mesh or surface and select the level.\n";
    output_report << "   For more information, check https://pymolwiki.org/index.php/Isomesh and https://pymolwiki.org/index.php/Isosurface \n";
    output_report << " - in USCF Chimera, check https://www.cgl.ucsf.edu/chimera/docs/ContributedSoftware/volumeviewer/volumeviewer.html \n";
    output_report << " - in USCF ChimeraX, check https://www.cgl.ucsf.edu/chimerax/docs/user/tools/volumeviewer.html \n";
  }
  output_report.close();
}

//////////////////////
// STRUCTURE OUTPUT //
//////////////////////

void Model::writeXYZfile(std::vector<std::tuple<std::string, double, double, double>> &atom_coordinates, std::string output_type){
  std::ofstream output_structure(output_folder+"/structure_"+output_type+"_"+calc_time+".xyz");
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

void Model::writeSurfaceMap(){
  // TODO reduce the size of total surface map in two probes mode (can also be reduced to a lesser extent in single probe mode)
  // in two probes mode, the surface map will be unnecessarily large due to the useless probe 2 voxel types filling the sides of the map
  // we can thus reduce the size of the map by removing a chunk of voxel from each side corresponding to the radius of probe 2


  // assemble data
  Container3D<Voxel>* surface_map = &_cell.getGrid(0);
  // save commonly used variable
  std::array<unsigned long int,3> n_elements = surface_map->getNumElements();
  double vxl_length = _cell.getResolution();
  // create map for assigning numbers to types
  std::map<char,int> typeToNum =
    {{0b00000011, 0},
     {0b00000101, 1},
     {0b00001001, 6},
     {0b00010001, 4},
     {0b00100001, 2},
     {0b01000001, 2}};

  // create and open new file
  std::ofstream output_file;
  output_file.open(output_folder + "/full_surface_map_"+calc_time+".dx");
  // comments
  output_file << "# OpenDX density file generated by MoloVol\n";
  output_file << "# Contains 3D surface map data to read in PyMol or Chimera\n";
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
    output_file << ' ' << _cell.getMin()[i] + vxl_length/2;
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
  output_file << "object 3 class array type double rank 0 items " << _cell.totalVxlOnLvl(0) << " data follows\n";
  // data
  int column = 0;
  for(size_t x = 0; x < n_elements[0]; x++){
    for(size_t y = 0; y < n_elements[1]; y++){
      for(size_t z = 0; z < n_elements[2]; z++){
        if (typeToNum.count(surface_map->getElement(x,y,z).getType()) != 0){
          output_file << typeToNum[surface_map->getElement(x,y,z).getType()];
        }
        else {
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




