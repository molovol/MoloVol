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

    data[i] = current_calculation->generateData();
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
  const double rad_probe1 = 1.2;
  const double expected_vdwVolume = 14337.422000;
  const double expected_time = 67;

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

  data = current_calculation->generateData();
  if(data.success){
    error_vdwVolume = abs(data.volumes[0b00000011]-expected_vdwVolume)/data.volumes[0b00000011];
    diff_time = data.getTime() - expected_time;

    printf("f: %40s, g: %4.1f, d: %4i, r: %4.1f\n", atom_filepath.c_str(), grid_step, max_depth, rad_probe1);
    printf("Error vdW: %20.10f, Excluded: %20.10f, Time: %10.5f s\n", error_vdwVolume, data.volumes[0b00000101], diff_time);
    printf("Type Assignment: %10.5f s, Volume Tally: %10.5f s\n", data.getTime(1), data.getTime(2));
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

        data = current_calculation->generateData();

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

bool Ctrl::unittest2Probe(){
  if(current_calculation == NULL){current_calculation = new Model();}

  // parameters for unittest:
  const std::string atom_filepath = "./inputfile/probetest_quadruplet.xyz";
  const std::string radius_filepath = "./inputfile/radii.txt";
  double rad_probe2 = 2;
  double rad_probe1 = 0.5;
  bool two_probe = true;
  int max_depth = 4;
  double grid_step = 0.1;

  std::unordered_map<std::string, double> rad_map = current_calculation->importRadiusMap(radius_filepath);

  CalcReportBundle data;
  current_calculation->readAtomsFromFile(atom_filepath, false);
  std::vector<std::string> included_elements = current_calculation->listElementsInStructure();

  current_calculation->setParameters(
      atom_filepath,
      "./output",
      false,
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

  data = current_calculation->generateData();

  if(data.success){
    printf("f: %40s, g: %4.2f, d: %4i, r1: %4.1f, r2: %4.1f\n",
        atom_filepath.c_str(), grid_step, max_depth, rad_probe1, rad_probe2);
    printf("vdW: %20.10f, Excluded: %20.10f\n", data.volumes[0b00000011]-28.866, data.volumes[0b00000101]-6.224);
    printf("P1 Core: %20.10f, P1 Shell: %20.10f\n", data.volumes[0b00001001]-0.202, data.volumes[0b00010001]-5.702);
    printf("P2 Core: %20.10f, P2 Shell: %20.10f\n", data.volumes[0b00100001]-681.534, data.volumes[0b01000001]-309.664);
    printf("Time elapsed: %10.5f s\n", data.getTime()-2.12);
  }
  else{
    std::cout << "Calculation failed" << std::endl;
  }
  return true;
}

bool Ctrl::unittestSurface(){
  if(current_calculation == NULL){current_calculation = new Model();}

  // parameters for unittest:
  const std::string atom_filepath = "./inputfile/6s8y.xyz";
  const std::string radius_filepath = "./inputfile/radii.txt";
  double rad_probe2 = 2;
  double rad_probe1 = 1.2;
  bool two_probe = false;
  bool surf_areas = true;
  int max_depth = 4;
  double grid_step = 0.1;

  std::unordered_map<std::string, double> rad_map = current_calculation->importRadiusMap(radius_filepath);

  CalcReportBundle data;
  current_calculation->readAtomsFromFile(atom_filepath, false);
  std::vector<std::string> included_elements = current_calculation->listElementsInStructure();

  current_calculation->setParameters(
      atom_filepath,
      "./output",
      false,
      false,
      surf_areas,
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

  data = current_calculation->generateData();

  if(data.success){
    printf("f: %40s, g: %4.2f, d: %4i, r1: %4.1f\n", atom_filepath.c_str(), grid_step, max_depth, rad_probe1);
    printf("vdW: %20.10f, Excluded: %20.10f\n", data.volumes[0b00000011]-14337.45, data.volumes[0b00000101]-3952.565);
    printf("P1 Core: %20.10f, P1 Shell: %20.10f\n", data.volumes[0b00001001]-103215.383, data.volumes[0b00010001]-9157.002);
    std::vector<double> cav = {112156.364,30.030,29.831,27.885,23.585,17.232,14.418,13.883,10.940,10.467,9.698,9.116,8.658,3.965,2.354,1.831,1.437,0.479,0.212};
    for (size_t i = 0; i< data.cavities.size(); ++i){
      printf("Cavity vol: %20.10f A^3\n", data.getCavVolume(i)-cav[i]);
//      printf("Cavity vol: %20.10f A^3\n", data.getCavVolume(i));
    }
    printf("Time elapsed: %10.5f s\n", data.getTime()-0.83+0.74-136.8);
  }
  else{
    std::cout << "Calculation failed" << std::endl;
  }
  return true;
}

bool Ctrl::unittestFloodfill(){
  if(current_calculation == NULL){current_calculation = new Model();}

  // parameters for unittest:
  const std::string atom_filepath = "./inputfile/Pd6L4_open_cage_Fujita.xyz";
  const std::string radius_filepath = "./inputfile/radii.txt";
  double rad_probe2 = 4;
  double rad_probe1 = 1.2;
  bool two_probe = true;
  bool surf_areas = false;
  int max_depth = 4;
  double grid_step = 0.3;

  std::unordered_map<std::string, double> rad_map = current_calculation->importRadiusMap(radius_filepath);

  CalcReportBundle data;
  current_calculation->readAtomsFromFile(atom_filepath, false);
  std::vector<std::string> included_elements = current_calculation->listElementsInStructure();

  current_calculation->setParameters(
      atom_filepath,
      "./output",
      false,
      false,
      surf_areas,
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

  data = current_calculation->generateData();

  if(data.success){
    printf("f: %40s, g: %4.2f, d: %4i, r1: %4.1f\n", atom_filepath.c_str(), grid_step, max_depth, rad_probe1);
    printf("vdW: %20.10f, Excluded: %20.10f\n", data.volumes[0b00000011]-28.948, data.volumes[0b00000101]-1.86);
    printf("P1 Core: %20.10f, P1 Shell: %20.10f\n", data.volumes[0b00001001]-247.748, data.volumes[0b00010001]-49.124);
    //std::vector<double> cav = {294.75,1.34,0.252,0.187,0.187,0.078,0.078};
    for (size_t i = 0; i< data.cavities.size(); ++i){
      //printf("Cavity vol: %20.10f A^3\n", data.getCavVolume(i)-cav[i]);
      printf("Cavity vol: %20.10f A^3\n", data.getCavVolume(i));
    }
    printf("Time elapsed: %10.5f s\n", data.getTime()-0.83+0.74);
  }
  else{
    std::cout << "Calculation failed" << std::endl;
  }
  return true;
}
