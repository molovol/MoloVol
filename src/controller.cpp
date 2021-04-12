
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

// default function call: transfer data from GUI to Model
bool Ctrl::runCalculation(){
  // create an instance of the model class
  // ensures, that there is only ever one instance of the model class
  if(current_calculation == NULL){
    current_calculation = new Model();
  }

  // TODO: Place the following code into a wrapper function

  // save parameters in model
  if(!current_calculation->setParameters(
      gui->getAtomFilepath(),
      gui->getOutputDir(),
      gui->getIncludeHetatm(),
      gui->getAnalyzeUnitCell(),
      gui->getProbeMode(),
      gui->getProbe1Radius(),
      gui->getProbe2Radius(),
      gui->getGridsize(),
      gui->getDepth(),
      gui->getMakeReport(),
      gui->getMakeSurfaceMap(),
      gui->getMakeCavityMaps(),
      gui->generateRadiusMap(),
      gui->getIncludedElements(),
      gui->getMaxRad())){
    return false;
  }
  auto start = std::chrono::steady_clock::now();
  CalcReportBundle data = current_calculation->generateVolumeData();
  auto end = std::chrono::steady_clock::now();
  double total_time = std::chrono::duration<double>(end-start).count();

  if (data.success){
    notifyUser("Result for " + data.chemical_formula);
    notifyUser("Elapsed time: " + std::to_string(total_time) + " s");
    notifyUser("Van der Waals volume: " + std::to_string(data.volumes[0b00000011]) + " A^3");
    notifyUser("Excluded void volume: " + std::to_string(data.volumes[0b00000101]) + " A^3");
    notifyUser("Probe 1 core volume: " + std::to_string(data.volumes[0b00001001]) + " A^3");
    notifyUser("Probe 1 shell volume: " + std::to_string(data.volumes[0b00010001]) + " A^3");
    if(data.probe_mode){
      notifyUser("Probe 2 core volume: " + std::to_string(data.volumes[0b00100001]) + " A^3");
      notifyUser("Probe 2 shell volume: " + std::to_string(data.volumes[0b01000001]) + " A^3");
    }
  }
  else{
    notifyUser("Calculation failed!");
  }
  return data.success;
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

// TODO remove if obselete
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
