#include "model.h"
#include "atom.h"
#include "controller.h"
#include "misc.h"
#include "container3d.h"
#include "griddata.h"
#include "voxel.h"
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>
#include <filesystem>

///////////////////
// RESULT REPORT //
///////////////////

std::string makeExportFileName(const std::string, const CalcReportBundle&, const char, const unsigned char=0);

int optimalPrecision(const double value);

void Model::createReport(){
  createReport(makeExportFileName(_output_folder, _data, 'r'));
}

void Model::createReport(std::string path){
  const double AVOGADRO = 6.02214076e23;
  std::string small_p = (_data.probe_mode)? "Small probe" : "Probe";
  std::string small_p_tab = (_data.probe_mode)? "" : "\t";
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
  output_report << "Website: https://molovol.com\n";
  output_report << "Source code available at https://github.com/molovol/MoloVol under the MIT licence\n";
  output_report << "Copyright © 2020-2025 Jasmin B. Maglic, Roy Lavendomme\n\n";
  output_report << "MoloVol program: calculation results report\n";
  output_report << "version: " + Ctrl::getVersion() + "\n\n";
  output_report << "Time of the calculation: " << _time_stamp << "\n";
  output_report << "Duration of the calculation: " << _data.getTime() << " s\n\n";
  output_report << "Structure file analyzed: " << _data.atom_file_path << "\n";
  output_report << "Chemical formula: " + _data.chemical_formula << "\n";
  output_report << "Molar mass: ";
  output_report << _data.molar_mass << " g/mol";
  if (isnan(_data.molar_mass)){
    output_report << " - Structure contains atoms with unknown atomic weights.";
  }
  output_report << "\n\n";
  output_report << "Please cite the following reference and include the calculation parameters below in the methods when reporting results.\n";
  output_report << "MoloVol: an easy-to-use program for analyzing cavities, volumes and surface areas of chemical structures.\n";
  output_report << "J. B. Maglic, R. Lavendomme, J. Appl. Cryst. 2022, 55, 1033–1044. DOI: 10.1107/S1600576722004988\n";


  output_report << "\n\n\t////////////////////////////\n";
  output_report << "\t// Calculation parameters //\n";
  output_report << "\t////////////////////////////\n\n";
  if(fileExtension(_data.atom_file_path) == "pdb"){
    output_report << std::string((_data.inc_hetatm)? "Include" : "Exclude") + " HETATM from pdb file\n";
  }
  output_report << "Crystal unit cell analysis: " + std::string((_data.analyze_unit_cell)? "Yes" : "No") + "\n";
  if(_data.probe_mode){
    output_report << "Probe mode: two probes\n";
    output_report << "Small probe radius: " << getProbeRad1() << " A\n";
    output_report << "Large probe radius: " << getProbeRad2() << " A\n";
  }
  else{
    output_report << "Probe mode: one probe\n";
    output_report << "Probe radius: " << getProbeRad1() << " A\n";
  }
  output_report << "Grid resolution: " << _data.grid_step << " A\n";
  output_report << "Optimization depth: " << _data.max_depth << "\n";
  output_report << "Elements radii:\n";
  for(std::unordered_map<std::string, double>::iterator it = _radius_map.begin(); it != _radius_map.end(); it++){
    if(isIncluded(it->first, _data.included_elements)){
      output_report << it->first << " : " << it->second << " A\n";
    }
  }

  output_report << "\n\n\t//////////////////////////////\n";
  output_report << "\t// Total Volumes calculated //\n";
  output_report << "\t//////////////////////////////\n\n";
  double unit_cell_vol = 0;
  std::array<double, 3> cav_vol_per_type = getTotalVolPerType(_data.cavities, _data.probe_mode);

  if(_data.analyze_unit_cell){
    output_report << "Orthogonal(ized) unit cell axes: " << _cart_matrix[0][0] << " A, " << _cart_matrix[1][1] << " A, " << _cart_matrix[2][2] << " A\n";
    unit_cell_vol = _cart_matrix[0][0]*_cart_matrix[1][1]*_cart_matrix[2][2];
    output_report << "Total unit cell volume: " << unit_cell_vol << " A^3\n";
    output_report << "Density: " << _data.molar_mass * 1e24 / (unit_cell_vol * AVOGADRO) << " g/cm^3\n\n";
  }

  // layout function for individual rows
  auto row = [this](
      std::string cell0,
      double cell1,
      std::string cell2,
      std::string cell3="",
      double cell4=0,
      std::string cell5=""
      ){
    const int col_width[] = {33,14,6,21,8}; // last column gets as much as needed
    std::string str = "";
    str += field(col_width[0],cell0) + " ";
    str += field(col_width[1],std::to_string(cell1),'r') + " ";
    if (_data.analyze_unit_cell && cell4!=0) {
      str += field(col_width[2],cell2) + " ";
      str += field(col_width[3],cell3) + " ";
      str += field(col_width[4],std::to_string(cell4),'r') + " ";
      str += cell5;
    }
    else{
      str += cell2;
    }
    return str + "\n";
  };

  // factor to convert A^3 to cm^3/g
  double volume_macro_factor = AVOGADRO * 1e-24 / _data.molar_mass;

  // layout function for volume data display
  auto vol_block = [row, unit_cell_vol, volume_macro_factor](std::string text, double vol, std::string subtext=""){
    std::string str = "";
    str += row(text + ":", vol,"A^3","(unit cell fraction:", vol/unit_cell_vol, ")");
    str += row(subtext,vol * volume_macro_factor, "cm^3/g");
    return str + "\n";
  };

  output_report << vol_block("Van der Waals volume", _data.volumes[0b00000011]);
  output_report << vol_block("Probe excluded void volume", _data.volumes[0b00000101]);
  output_report << vol_block("Molecular volume",
      _data.volumes[0b00000011] + _data.volumes[0b00000101], "(vdw + probe excluded void)");
  // TODO: for unit cell analysis when pores are defined, add molecular volume with isolated cavities and total pore volume
  if(!_data.analyze_unit_cell){
    output_report << vol_block("Molecular volume",
        _data.volumes[0b00000011] + _data.volumes[0b00000101] + cav_vol_per_type[0], "with isolated cavities");
  }
  if(!_data.probe_mode && !_data.analyze_unit_cell){
    output_report << small_p << " core volume: No physical meaning, contains all volume outside the structure.\n\n";
  }
  else{
    output_report << vol_block(small_p + " core volume", _data.volumes[0b00001001]);
  }
  output_report << vol_block(small_p + " shell volume", _data.volumes[0b00010001]);
  if(_data.probe_mode || _data.analyze_unit_cell){
    output_report << vol_block(small_p + " occupied volume", _data.volumes[0b00001001] + _data.volumes[0b00010001], "(core + shell)");
  }
  if(!_data.probe_mode && _data.analyze_unit_cell){
    output_report << "For porous materials, this probe occupied volume is the value typically reported for total pore volume.\n";
    output_report << "(only if the pore network is fully accessible without isolated cavities, check the cavity list below and surface maps)\n";
  }

  if(_data.probe_mode){
    if(!_data.analyze_unit_cell){
      output_report << "Large probe core volume: No physical meaning, contains all volume outside the structure.\n\n";
    }
    else{
      output_report << vol_block("Large probe core volume", _data.volumes[0b00100001]);
    }
    output_report << vol_block("Large probe shell volume", _data.volumes[0b01000001]);
    if(_data.analyze_unit_cell){
      output_report << vol_block("Large probe occupied volume", _data.volumes[0b00100001] + _data.volumes[0b01000001], "(core + shell)");
    }
  }

  if(_data.calc_surface_areas){
    output_report << "\n\t////////////////////////////////////\n";
    output_report << "\t// Total Surface Areas calculated //\n";
    output_report << "\t////////////////////////////////////\n\n";
    // factor to convert A^2 to m^2/g
    double area_macro_factor = AVOGADRO * 1e-20 / _data.molar_mass;

    // layout function for surface data display
    auto surf_block = [row,area_macro_factor](std::string text, double surf, std::string subtext=""){
      std::string str = "";
      str += row(text + ":", surf, "A^2");
      str += row(subtext, surf * area_macro_factor, "m^2/g");
      return str + "\n";
    };

    output_report << surf_block("Van der Waals surface", _data.surf_vdw);
    output_report << surf_block(small_p + " excluded surface", _data.surf_probe_excluded, "(similar to Connolly surface)");
    output_report << surf_block(small_p + " accessible surface", _data.surf_probe_accessible, "(similar to Lee-Richards surface)");
	if(!_data.probe_mode && _data.analyze_unit_cell){
      output_report << "For porous materials, this probe accessible surface area is the value typically reported for BET and Langmuir surface areas.\n";
      output_report << "(only if the pore network is fully accessible without isolated cavities, check the cavity list below and surface maps)\n";
	  output_report << "Check the user manual to understand the limitations of this calculated value compared to experimental data.\n";
    }
    if(_data.probe_mode){
      output_report << surf_block("Molecular surface", _data.surf_molecular, "(both probes excluded surface)");
    }
    else if(!_data.analyze_unit_cell){
      output_report << "Molecular open surface (reachable from outside): see outside space surface data in cavities list below.\n\n";
    }
    // TODO: for unit cell analysis when pores are defined, add total pore excluded and accessible surfaces
  }

  if(!_data.cavities.empty()){
    output_report << "\n\t///////////////////\n";
    output_report << "\t// Cavities data //\n";
    output_report << "\t///////////////////\n\n";

    if(_data.cavities.size() >= 255){
      output_report << "!!! WARNING !!!\n";
      output_report << "Maximum number of cavities reached. Some cavities might be missing.\n";
      output_report << "To solve this issue, change probe radii (e.g. smaller large probe and/or larger small probe).\n\n";
    }
    output_report << "Note 1:\tSeparate cavities are defined by space accessible to the core of the small probe.\n";
    output_report << "\tTwo cavities can be in contact but if a probe cannot pass from one to the other, they are considered separated.\n";
    output_report << "Note 2:\tCavities are listed in decreasing order of volume and cavity surface map file names match the IDs.\n";
    output_report << "Note 3:\tIn single probe mode, pockets and tunnels are counted in the 'outside space'.\n";
    output_report << "Note 4:\tSome very small isolated chunks of small probe cores can be detected and lead to small cavities.\n";
    output_report << "Note 5:\tProbe occupied volume corresponds to empty space as defined by the molecular surface (similar to the Connolly surface).\n";
    output_report << "\tThis occupied volume is typically the volume reported for cavities in cage compounds.\n";
    output_report << "Note 6:\tProbe accessible volume corresponds to empty space as defined\n";
    output_report << "\tby the surface accessible to its core (similar to the Lee-Richards surface).\n";
    output_report << "Note 7:\tFor a detailed shape of each cavity, check the surface maps.\n\n";

    // TODO: change output when cavity types are features for unit cell analysis
    // store data in GridData
    GridData cavity_data({
      GridCol("Cavity ID", ""),
      GridCol("Occupied", "Volume (A^3)"),
      GridCol("Accessible", "Volume (A^3)"),
      GridCol("Excluded", "Surface (A^2)", !_data.calc_surface_areas, 0),
      GridCol("Accessible", "Surface (A^2)", !_data.calc_surface_areas, 0),
      GridCol("Cavity Type", "", _data.analyze_unit_cell, 0),
      GridCol("Cavity center coordinates (A)", "x"),
      GridCol("\n", "y"), // the newline character is not printed, used here to skip the line
      GridCol("", "z")
    });

    auto toStringFixedPrecision = [](const double value){
      std::stringstream ss;
      ss << std::setprecision(optimalPrecision(value)) << value;
      return ss.str();
    };

    for(unsigned int i = 0; i < _data.cavities.size(); i++){
      std::array<double,3> cav_center = _data.getCavCenter(i);

      std::string occ_vol = (!_data.probe_mode && !_data.analyze_unit_cell && _data.cavities[i].id == 1)? "Outside"
        : toStringFixedPrecision(_data.cavities[i].getVolume());
      std::string access_vol = (!_data.probe_mode && !_data.analyze_unit_cell && _data.cavities[i].id == 1)? "Outside"
        : toStringFixedPrecision(_data.cavities[i].core_vol);

      cavity_data.storeValues({
        std::to_string(i+1),
        occ_vol,
        access_vol,
        toStringFixedPrecision(_data.cavities[i].surf_shell),
        toStringFixedPrecision(_data.cavities[i].surf_core),
        _data.cavities[i].cavTypeDescriptor(_data.probe_mode),
        toStringFixedPrecision(cav_center[0]),
        toStringFixedPrecision(cav_center[1]),
        toStringFixedPrecision(cav_center[2]),
      });
    }

    // function that writes table to file
    auto writeTable = [](std::ofstream& output_report, const GridData& cavity_data){
      const std::vector<int> col_width = {10,14,14,14,14,14,8,8};

      std::vector<std::vector<std::string>> table_rows = {cavity_data.getHeaders(), cavity_data.getSubheaders()};
      for (size_t row = 0; row < cavity_data.getNumberRows(false); ++row){
        table_rows.push_back(cavity_data.getRow(row));
      }

      for (const auto& row : table_rows){
        for (size_t i = 0; i < col_width.size(); ++i){
          // newline character is used here as a hack to skip printing only whitespaces
          if (cavity_data.hideCol(i) || row[i] == "\n"){continue;}
          output_report << field(col_width[i], row[i]);
        }
        output_report << row[cavity_data.getNumberCols()-1] << "\n";
      }
    };

    // call function
    writeTable(output_report, cavity_data);
  }
  output_report << "\n\n\t/////////////////////////////\n";
  output_report << "\t// Surface map information //\n";
  output_report << "\t/////////////////////////////\n\n";
  output_report << "The surface map files generated (.dx, OpenDX format) can be opened with\n";
  output_report << "PyMOL, USCF Chimera or USCF ChimeraX.\n\n";
  output_report << "Use the following isosurface levels to visualize the desired surface:\n";
  output_report << "Level 0.5 : Van der Waals surface\n";
  if(_data.probe_mode){
    output_report << "Level 1.5 : Molecular surface (both probes excluded, similar to the Connolly surface)\n";
    output_report << "Level 3.0 : Isolated cavities, pockets and tunnels (small probe excluded, similar to the Connolly surface but only 'inside')\n";
    output_report << "Level 5.0 : Small probe accessible surface (similar to Lee-Richards molecular surface but only 'inside')\n";
  }
  else{
    output_report << "Level 2.0 : Molecular and cavity surface (probe excluded, similar to the Connolly surface)\n";
    output_report << "Level 5.0 : Probe accessible surface (similar to the Lee-Richards molecular surface)\n";
  }
  output_report << "\nFor help on how to vizualize maps check the user manual or:\n";
  output_report << " - in PyMOL, simply open the map file then click 'A' in the right panel, choose mesh or surface and select the level.\n";
  output_report << "   For more information, check https://pymolwiki.org/index.php/Isomesh and https://pymolwiki.org/index.php/Isosurface \n";
  output_report << " - in USCF Chimera, check https://www.cgl.ucsf.edu/chimera/docs/ContributedSoftware/volumeviewer/volumeviewer.html \n";
  output_report << " - in USCF ChimeraX, check https://www.cgl.ucsf.edu/chimerax/docs/user/tools/volumeviewer.html \n";

  output_report << "\nIf you wish to visualize the structure file in PyMOL with the same element radii as in the MoloVol calculation,\n";
  output_report << "paste the following command lines (all at once) in the command prompt of PyMOL after opening the structure file.\n";
  output_report << "Note: remove charges from ions before copying the command because only alphabetic characters are used for element symbols in PyMOL.\n";
  output_report << "If an element is present in more than one oxidation state, it is necessary to select atoms separately in PyMOL to apply different radii\n\n";

  for(std::unordered_map<std::string, double>::iterator it = _radius_map.begin(); it != _radius_map.end(); it++){
    if(isIncluded(it->first, _data.included_elements)){
      output_report << "alter (elem " << it->first << "),vdw=" << it->second << "\n";
    }
  }
  output_report << "rebuild\n\n";

  output_report.close();
}

//////////////////////
// STRUCTURE OUTPUT //
//////////////////////


void Model::writeCrystStruct(std::string path){
  path.erase(path.end()-4, path.end());
  writeXYZfile(_data.orth_cell, path+"_struct_orthogonal_cell.xyz", "Orthogonal cell");
  writeXYZfile(_data.supercell, path+"_struct_partial_supercell.xyz", "Partial supercell");
}

void Model::writeCrystStruct(){
  std::string path = makeExportFileName(_output_folder, _data, 'c');
  writeXYZfile(_data.orth_cell, path, "Orthogonal cell");
  path = makeExportFileName(_output_folder, _data, 'p');
  writeXYZfile(_data.supercell, path, "Partial supercell");
}

void Model::writeXYZfile(const std::vector<std::tuple<std::string, double, double, double>> &atom_coordinates, const std::string path, const std::string output_type){
  std::ofstream output_structure(path);
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
  writeTotalSurfaceMap(makeExportFileName(_output_folder, _data, 's'));
}

void Model::writeTotalSurfaceMap(const std::string file_path){
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
  writeSurfaceMap(file_path, vxl_length, n_elements, origin, start_index, end_index);
}

void Model::writeCavitiesMaps(){
  writeCavitiesMaps("make_auto_name");
}

void Model::writeCavitiesMaps(const std::string file_path){
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
    std::string cavity_file_name;
    if (file_path == "make_auto_name"){
      cavity_file_name = makeExportFileName(_output_folder, _data, 's', id+1);
    }
    else{
      cavity_file_name = file_path;
      cavity_file_name.insert(file_path.find_last_of('.'), "_cav" + std::to_string(id+1));
    }
    /*
    // alternative with cav id + name of file
    size_t path_end = file_path.find_last_of("\\/");
    std::string cavity_file_name = file_path.substr(0,path_end+1) + "cav" + std::to_string(id+1) + "_"
      + file_path.substr(path_end+1,file_path.length()-path_end+1);
    */

    writeSurfaceMap(cavity_file_name, vxl_length, n_elements, origin, start_index, end_index, true, id);
  }
}

void Model::writeSurfaceMap(const std::string file_path,
                            double vxl_length,
                            std::array<unsigned long int,3> n_elements,
                            std::array<double,3> origin,
                            std::array<unsigned int,3> start_index,
                            std::array<unsigned int,3> end_index,
                            const bool partial_map,
                            const unsigned char id){
  bool issue_encountered = false;
  // assemble data
  const Container3D<Voxel>* surface_map = &_cell.getGrid(0);

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
  output_file.open(file_path);
  // comments
  output_file << "# OpenDX density file generated by MoloVol\n";
  output_file << "# Contains 3D surface map data to read in PyMOL, Chimera or ChimeraX\n";
  output_file << "# Data is written in C array order: In grid[x,y,z] the axis z is fastest\n";
  output_file << "# varying, then y, then finally x.\n";
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
          issue_encountered = true;
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
  if (issue_encountered) {Ctrl::getInstance()->displayErrorMessage(303);}
}

const Container3D<Voxel>& Model::getSurfaceData() const {
  return _cell.getGrid(0);
}

std::array<double,3> Model::getCellOrigin() const {
  return _cell.getOrigin();
}

const AtomTree& Model::getAtomTree() const {
  return Voxel::getAtomTree();
}

///////////////
// FILE NAME //
///////////////

static const std::map<char,std::string> s_file_descriptor{
  {'r' , "MoloVol-report"},
  {'s' , "surface-map"},
  {'c' , "struct-orthogonal-cell"},
  {'p' , "struct-partial-supercell"}
};

static const std::map<char,std::string> s_file_extension{
  {'r' , ".txt"},
  {'s' , ".dx"},
  {'c' , ".xyz"},
  {'p' , ".xyz"}
};

bool fileExists(const std::string&);

std::string rstripZeros(const std::string str);

std::string makeExportFileName(const std::string dir, const CalcReportBundle& data, const char filetype, const unsigned char n_cav){
  assert(s_file_descriptor.count(filetype));
  std::string filename = "";
  filename += fileName(data.atom_file_path);
  filename += "_" + s_file_descriptor.at(filetype);
  filename += "_grid-" + rstripZeros(std::to_string(data.grid_step));
  filename += "_rad1-" + rstripZeros(std::to_string(data.r_probe1));
  if (data.probe_mode){
    filename += "_rad2-" + rstripZeros(std::to_string(data.r_probe2));
  }
  if(filetype == 's'){
    filename += (n_cav)? "_cav" + std::to_string(n_cav) : "_full-structure";
  }
  std::string fullname = dir + "/" + filename + s_file_extension.at(filetype);
  int i = 2;
  while (fileExists(fullname)){
    fullname = dir + "/" + filename + "_" + std::to_string(i) + s_file_extension.at(filetype);
    i++;
  }
  return fullname;
}

std::string rstripZeros(std::string str){
  return str.erase(str.find_first_of('0',str.find_first_of('.')+2), str.find_first_of('.')+2 - str.length());
}

bool fileExists(const std::string& path){
  if (FILE *file = fopen(path.c_str(), "r")){
    fclose(file);
    return true;
  }
  else {return false;}
}

///////////////////
// AUX FUNCTIONS //
///////////////////

// default precision is 6 which shows as up to 7 characters when decimals are present for values >= 1
// for values < 1 with many digits, the default precision can lead to more than 7 characters
// which is larger than a tabulation and can mess up layout in the report
// thus the precision is adjusted to insure a maximum of 7 characters
int optimalPrecision(const double value){
  if(value < 1e-5){
    return 1;
  }
  else if(value < 1){
    return (int)(6.0 + log(value));
  }
  return 6;
}
