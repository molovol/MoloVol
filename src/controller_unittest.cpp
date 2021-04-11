#include "controller.h"
#include "base.h"
#include "atom.h" // i don't know why
#include "model.h"
#include <cmath>
#include <map>

bool Ctrl::unittestExcluded(){
  if(current_calculation == NULL){current_calculation = new Model();}

  // parameters for unittest:
  const std::string atom_filepaths[] =
  {"./inputfile/probetest_pair.xyz", "./inputfile/probetest_triplet.xyz", "./inputfile/probetest_quadruplet.xyz"};
  const std::string radius_filepath = "./inputfile/radii.txt";
  const double grid_step = 0.1;
  const int max_depth = 4;
  const double rad_probe1 = 1.2;
  const double expected_volumes[] = {1.399000, 4.393000, 9.054000};

  // preparation for calling runCalculation()
  std::unordered_map<std::string, double> rad_map = current_calculation->importRadiusMap(radius_filepath);
  current_calculation->setRadiusMap(rad_map);

  double error[3];
  CalcReportBundle data[3];
  for (int i = 0; i < 3; i++){
    current_calculation->readAtomsFromFile(atom_filepaths[i], false);
    std::vector<std::string> included_elements = current_calculation->listElementsInStructure();
    
    current_calculation->setParameters(
        atom_filepaths[i],
        "./output",
        false,
        false,
        false,
        rad_probe1,
        0,
        grid_step,
        max_depth,
        false,
        false,
        false,
        rad_map,
        included_elements,
        3);

    data[i] = current_calculation->generateVolumeData();
    if(data[i].success){
      error[i] = abs(data[i].volumes[0b00000101]-expected_volumes[i])/data[i].volumes[0b00000101];
    }
    else{
      std::cout << "Calculation failed" << std::endl;
      return false;
    }
  }

  for (int i = 0; i < 3; i++){
    printf("f: %40s, g: %4.1f, d: %4i, r: %4.1f\n", atom_filepaths[i].c_str(), grid_step, max_depth, rad_probe1);
    printf("Error: %20.10f, Time: %10.5f\n", error[i], data[i].getTime());
  }

  return true;
}

bool Ctrl::unittestProtein(){
  if(current_calculation == NULL){current_calculation = new Model();}

  // parameters for unittest:
  const std::string atom_filepath = "./inputfile/6s8y.xyz";
  const std::string radius_filepath = "./inputfile/radii.txt";
  const double grid_step = 0.1;
  const int max_depth = 4;
  const double rad_probe1 = 0.5;
  const double expected_vdwVolume = 14337.422000;
  const double expected_time = 155;

  std::unordered_map<std::string, double> rad_map = current_calculation->importRadiusMap(radius_filepath);

  double error_vdwVolume;
  double diff_time;
  CalcReportBundle data;
  current_calculation->readAtomsFromFile(atom_filepath, false);
  std::vector<std::string> included_elements = current_calculation->listElementsInStructure();

  current_calculation->setParameters(
      atom_filepath,
      "./output",
      false,
      false,
      false,
      rad_probe1,
      0,
      grid_step,
      max_depth,
      false,
      false,
      false,
      rad_map,
      included_elements,
      3);

  data = current_calculation->generateVolumeData();
  if(data.success){
    error_vdwVolume = abs(data.volumes[0b00000011]-expected_vdwVolume)/data.volumes[0b00000011];
    diff_time = data.getTime() - expected_time;

    printf("f: %40s, g: %4.1f, d: %4i, r: %4.1f\n", atom_filepath.c_str(), grid_step, max_depth, rad_probe1);
    printf("Error vdW: %20.10f, Excluded: %20.10f, Time: %10.5f s\n", error_vdwVolume, data.volumes[0b00000101], diff_time);
    printf("Type Assignment: %10.5f s, Volume Tally: %10.5f s\n", data.type_assignment_elapsed_seconds, data.volume_tally_elapsed_seconds);
  }
  else{
    std::cout << "Calculation failed" << std::endl;
  }
  return true;
}

bool Ctrl::unittestRadius(){
  if(current_calculation == NULL){current_calculation = new Model();}

  // parameters for unittest:
  const std::string atom_filepath = "./inputfile/probetest_pair.xyz";
  const std::string radius_filepath = "./inputfile/radii.txt";
  double rad_probe2 = 1.2;
  bool two_probe = true;

  std::unordered_map<std::string, double> rad_map = current_calculation->importRadiusMap(radius_filepath);
  {int max_depth = 4;
    //for (int max_depth = 4; max_depth < ; max_depth++){
    {double grid_step = 0.1;
      //for (double grid_step = 1; grid_step>0.01; grid_step-=0.01){
      {double rad_probe1 = 0;
        //for (double rad_probe1 = 2; rad_probe1 < 2.01; rad_probe1 += 0.1){


        CalcReportBundle data;
        current_calculation->readAtomsFromFile(atom_filepath, false);
        std::vector<std::string> included_elements = current_calculation->listElementsInStructure();

        current_calculation->setParameters(
            atom_filepath,
            "./output",
            false,
            false,
            two_probe,
            rad_probe1,
            rad_probe2,
            grid_step,
            max_depth,
            false,
            false,
            false,
            rad_map,
            included_elements,
            3);

        data = current_calculation->generateVolumeData();

        if(data.success){
          printf("f: %40s, g: %4.2f, d: %4i, r: %4.1f\n", atom_filepath.c_str(), grid_step, max_depth, rad_probe1);
          printf("vdW: %20.10f, Excluded: %20.10f\n", data.volumes[0b00000011], data.volumes[0b00000101]);
          printf("Time elapsed: %10.5f s\n", data.getTime());
        }
        else{
          std::cout << "Calculation failed" << std::endl;
        }
      }
    }
  }
  return true;
}

