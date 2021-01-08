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
  CalcResultBundle data[3];
  for (int i = 0; i < 3; i++){
    current_calculation->readAtomsFromFile(atom_filepaths[i], false);
    std::vector<std::string> included_elements = current_calculation->listElementsInStructure();
    data[i] = runCalculation(atom_filepaths[i], grid_step, max_depth, rad_map, included_elements, rad_probe1);
    if(data[i].success){
      error[i] = abs(data[i].volumes['x']-expected_volumes[i])/data[i].volumes['x'];
    }
    else{
      std::cout << "Calculation failed" << std::endl;
    }
  }

  for (int i = 0; i < 3; i++){
    printf("f: %40s, g: %4.1f, d: %4i, r: %4.1f\n", atom_filepaths[i].c_str(), grid_step, max_depth, rad_probe1);
    printf("Error: %20.10f, Time: %10.5f\n", error[i], data[i].getTime());
  }

  return true;
}
