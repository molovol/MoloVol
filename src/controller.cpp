
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
    notifyUser("\nInvalid radii definition file!");
    notifyUser("\nPlease select a valid file or set radii manually.");
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
    notifyUser("\nInvalid structure file format!");
    successful_import = false;
  }
  catch (const ExceptInvalidInputFile& e){
    notifyUser("\nInvalid structure file!");
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
  CalcReportBundle data = current_calculation->generateVolumeData();

  if (data.success){
    notifyUser("\nResult for ");
    notifyUser(Symbol::generateChemicalFormulaUnicode(data.chemical_formula));
    notifyUser("\nElapsed time: " + std::to_string(data.getTime()) + " s");
    notifyUser("\nVan der Waals volume: " + std::to_string(data.volumes[0b00000011]) + " ");
    notifyUser(Symbol::angstrom() + Symbol::cubed());
    notifyUser("\nExcluded void volume: " + std::to_string(data.volumes[0b00000101]) + " ");
    notifyUser(Symbol::angstrom() + Symbol::cubed());
    notifyUser("\nProbe 1 core volume: " + std::to_string(data.volumes[0b00001001]) + " ");
    notifyUser(Symbol::angstrom() + Symbol::cubed());
    notifyUser("\nProbe 1 shell volume: " + std::to_string(data.volumes[0b00010001]) + " ");
    notifyUser(Symbol::angstrom() + Symbol::cubed());
    for (size_t i = 1; i < data.cavities.size(); ++i){
      notifyUser("\nCav " + std::to_string(i) + ": " + std::to_string(data.cavities[i]) + " ");
      notifyUser(Symbol::angstrom() + Symbol::cubed());
      notifyUser("(" + std::to_string(data.getCavCentre(i)[0]) + ", "
        + std::to_string(data.getCavCentre(i)[1]) + ", "
        + std::to_string(data.getCavCentre(i)[2]) + ")");
    }
    if(data.probe_mode){
      notifyUser("\nProbe 2 core volume: " + std::to_string(data.volumes[0b00100001]) + " ");
      notifyUser(Symbol::angstrom() + Symbol::cubed());
      notifyUser("\nProbe 2 shell volume: " + std::to_string(data.volumes[0b01000001]) + " ");
      notifyUser(Symbol::angstrom() + Symbol::cubed());
    }
  }
  else{
    notifyUser("\nCalculation failed!");
  }
  return data.success;
}

void Ctrl::notifyUser(std::string str, bool to_gui){
  if (to_gui){
    gui->appendOutput(str);
  }
  else {
    std::cout << str;
  }
}

void Ctrl::notifyUser(std::wstring wstr){
  gui->appendOutput(wstr);
}

// TODO remove if obselete
void Ctrl::prepareOutput(std::string atomFilePath){
  if(!current_calculation->createOutputFolder(fileName(atomFilePath))){
      notifyUser("\nNew output folder could not be created.\nThe output file(s) will be created in the program folder.");
  }
}

void Ctrl::exportReport(){
  current_calculation->createReport();
}

void Ctrl::exportSurfaceMap(){
  current_calculation->writeSurfaceMap();
}
