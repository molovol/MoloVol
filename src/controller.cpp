
#include "controller.h"
#include "base.h"
#include "atom.h" // i don't know why
#include "model.h"
#include "misc.h"
#include "special_chars.h"
#include <chrono>
#include <utility>
#include <map>

///////////////////////
// STATIC ATTRIBUTES //
///////////////////////

Ctrl* Ctrl::instance = NULL;
MainFrame* Ctrl::gui = NULL;

/////////////
// METHODS //
/////////////

void Ctrl::registerView(MainFrame* inp_gui){
  gui = inp_gui;
}

Ctrl* Ctrl::getInstance(){
  if(instance == NULL){
    instance = new Ctrl();
  }
  return instance;
}

bool Ctrl::loadRadiusFile(){
  // create an instance of the model class
  // ensures, that there is only ever one instance of the model class
  if(current_calculation == NULL){
    current_calculation = new Model();
  }

  std::string radius_filepath = gui->getRadiusFilepath();
  if(!current_calculation->readRadiusFileSetMaps(radius_filepath)){
    notifyUser("Invalid radii definition file!");
    notifyUser("Please select a valid file or set radii manually.");
    // shouldn't this return false? -JM
  }
  // refresh atom list using new radius map
  gui->displayAtomList(current_calculation->generateAtomList());
  return true;
}

bool Ctrl::loadAtomFile(){
  // create an instance of the model class
  // ensures, that there is only ever one instance of the model class
  if(current_calculation == NULL){
    current_calculation = new Model();
  }

  bool successful_import;
  try{successful_import = current_calculation->readAtomsFromFile(gui->getAtomFilepath(), gui->getIncludeHetatm());}

  catch (const ExceptIllegalFileExtension& e){
    notifyUser("Invalid structure file format!");
    successful_import = false;
  }
  catch (const ExceptInvalidInputFile& e){
    notifyUser("Invalid structure file!");
    successful_import = false;
  }

  gui->displayAtomList(current_calculation->generateAtomList()); // update gui

  return successful_import;
}

// called by user through GUI, where the data isn't needed afterwards
bool Ctrl::runCalculation(){
  // start the timer for the total calculation time
  auto start = std::chrono::steady_clock::now();

  // create an instance of the model class
  // ensures, that there is only ever one instance of the model class
  if(current_calculation == NULL){
    current_calculation = new Model();
  }

  // create a report bundle
  CalcReportBundle data;

  // save parameters in model
  if(!current_calculation->setParameters(
      gui->getAtomFilepath(),
      gui->getIncludeHetatm(),
      gui->getAnalyzeUnitCell(),
      gui->getProbeMode(),
      gui->getProbe1Radius(),
      gui->getProbe2Radius(),
      gui->getGridsize(),
      gui->getDepth(),
      //TODO add get output folder
      gui->getMakeReport(),
      gui->getMakeSurfaceMap(),
      gui->getMakeCavityMaps(),
      gui->generateRadiusMap(),
      gui->getIncludedElements(),
      gui->getMaxRad())){
    data.success = false;
    return data.success;
  }

  // process atom data for unit cell analysis if the option it ticked
  if(gui->getAnalyzeUnitCell()){
    if(!current_calculation->processUnitCell()){
      data.success = false;
      return data.success;
    }
  }

  // determine which atoms will be taken into account
  current_calculation->setAtomListForCalculation();
  // place atoms in a binary tree for faster access
  current_calculation->storeAtomsInTree();
  // set size of the box containing all atoms
  current_calculation->defineCell();

  current_calculation->generateChemicalFormula();

  // assign voxel types and store the volume(s)
  data = current_calculation->calcVolume();

  // start the timer and save the total calculation time
  auto end = std::chrono::steady_clock::now();
  double calc_time = std::chrono::duration<double>(end-start).count();
  current_calculation->setTotalCalcTime(calc_time);

  if (data.success){
    notifyUser("Result for " + data.chemical_formula);
    notifyUser("Elapsed time: " + std::to_string(calc_time) + " s");
    notifyUser("Van der Waals volume: " + std::to_string(data.volumes[0b00000011]) + " A^3");
    notifyUser("Excluded void volume: " + std::to_string(data.volumes[0b00000101]) + " A^3");
    notifyUser("Probe 1 core volume: " + std::to_string(data.volumes[0b00001001]) + " A^3");
    notifyUser("Probe 1 shell volume: " + std::to_string(data.volumes[0b00010001]) + " A^3");
    if(data.probe_mode){
      notifyUser("Probe 2 core volume: " + std::to_string(data.volumes[0b00100001]) + " A^3");
      notifyUser("Probe 2 shell volume: " + std::to_string(data.volumes[0b01000001]) + " A^3");
    }
    return true;
  }
  return false;
}

void Ctrl::notifyUser(std::string str, bool to_gui){
  str = "\n" + str;
  if (to_gui){
    gui->appendOutput(str);
  }
  else {
    std::cout << str;
  }
}

void Ctrl::prepareOutput(std::string atomFilePath){
  if(!current_calculation->createOutputFolder(fileName(atomFilePath))){
      notifyUser("New output folder could not be created.\nThe output file(s) will be created in the program folder.");
  }
}

void Ctrl::exportReport(){
  current_calculation->createReport();
}

void Ctrl::exportSurfaceMap(){
  current_calculation->writeSurfaceMap();
}
