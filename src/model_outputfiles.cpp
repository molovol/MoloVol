#include "model.h"
#include "atom.h"
#include "controller.h"
#include "misc.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <filesystem>

///////////////////////////////
// AUX FUNCTION DECLARATIONS //
///////////////////////////////

std::string timeNow();

///////////////////
// RESULT REPORT //
///////////////////

//
bool Model::createOutputFolder(){
  // folder name based on current time to avoid overwriting output files with successive calculations
  output_folder = "./output/" + timeNow();
  if(std::filesystem::create_directories(output_folder)){
    return true;
  }
  else{
    output_folder = "./";
    return false;
  }
}

void Model::createReport(std::string input_filepath, std::vector<std::string> parameters){
  std::ofstream output_report(output_folder+"/MoloVol result report.txt");
  output_report << "MoloVol program: calculation results report\n\n";
  output_report << "Structure file analyzed: " << input_filepath << "\n";
  for(size_t i = 0; i < parameters.size(); i++){
    output_report << parameters[i] << "\n";
  }






  output_report.close();
}

//////////////////////
// STRUCTURE OUTPUT //
//////////////////////

void Model::writeXYZfile(std::vector<std::tuple<std::string, double, double, double>> &atom_coordinates, std::string output_type){
  std::ofstream output_structure(output_folder+"/structure_"+output_type+".xyz");
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



///////////////////
// AUX FUNCTIONS //
///////////////////

// find the current time and convert to a string in format year-month-day_hour-min-sec
std::string timeNow(){
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime (&rawtime);

    strftime (buffer,80,"%Y-%m-%d_%Hh%Mm%Ss",timeinfo);

    return buffer;
}




