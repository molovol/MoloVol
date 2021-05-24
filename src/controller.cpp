
#include "controller.h"
#include "base.h"
#include "atom.h" // i don't know why
#include "model.h"
#include "misc.h"
#include "exception.h"
#include "special_chars.h"
#include <chrono>
#include <utility>
#include <map>

///////////////////////
// STATIC ATTRIBUTES //
///////////////////////

Ctrl* Ctrl::instance = NULL;
MainFrame* Ctrl::gui = NULL;

////////////////
// TOGGLE GUI //
////////////////

void Ctrl::disableGUI(){
  _to_gui = false;
}

void Ctrl::enableGUI(){
  _to_gui = true;
}

bool Ctrl::isGUIEnabled(){
  return _to_gui;
}

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
  // even if there is no valid radii file, the program can be used by manually setting radii in the GUI after loading a structure
  if(!current_calculation->readRadiusFileSetMaps(radius_filepath)){
    notifyUser("\nInvalid radii definition file!");
    notifyUser("\nPlease select a valid file or set radii manually.");
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

  // PARAMETERS
  // save parameters in model
  if(!current_calculation->setParameters(
      gui->getAtomFilepath(),
      gui->getOutputDir(),
      gui->getIncludeHetatm(),
      gui->getAnalyzeUnitCell(),
      gui->getCalcSurfaceAreas(),
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

  // CALCULATION
  CalcReportBundle data = current_calculation->generateData();
  calculationDone(data.success);

  // OUTPUT
  clearOutput();
  if(data.success){
    if(!data.cavities.empty()){gui->displayCavityList(data.cavities);}
    std::wstring vol_unit = Symbol::angstrom() + Symbol::cubed();

    notifyUser("Result for ");
    notifyUser(Symbol::generateChemicalFormulaUnicode(data.chemical_formula));
    notifyUser("\nElapsed time: " + std::to_string(data.getTime()) + " s");
    
    notifyUser("\n<VOLUME>");
    notifyUser("\nVan der Waals volume: " + std::to_string(data.volumes[0b00000011]) + " ");
    notifyUser(vol_unit);
    notifyUser("\nProbe inaccessible volume: " + std::to_string(data.volumes[0b00000101]) + " ");
    notifyUser(vol_unit);
    std::string prefix = data.probe_mode? "Small p" : "P";
    notifyUser("\n"+ prefix +"robe core volume: " + std::to_string(data.volumes[0b00001001]) + " ");
    notifyUser(vol_unit);
    notifyUser("\n"+ prefix +"robe shell volume: " + std::to_string(data.volumes[0b00010001]) + " ");
    notifyUser(vol_unit);
    if(data.probe_mode){
      notifyUser("\nLarge probe core volume: " + std::to_string(data.volumes[0b00100001]) + " ");
      notifyUser(vol_unit);
      notifyUser("\nLarge probe shell volume: " + std::to_string(data.volumes[0b01000001]) + " ");
      notifyUser(vol_unit);
    }
    if(data.calc_surface_areas){
      std::wstring surf_unit = Symbol::angstrom() + Symbol::squared();
      notifyUser("\n<SURFACE>");
      notifyUser("\nVan der Waals surface: " + std::to_string(data.getSurfVdw()) + " ");
      notifyUser(surf_unit);
      if(data.probe_mode){
        notifyUser("\nMolecular surface: " + std::to_string(data.getSurfMolecular()) + " ");
        notifyUser(surf_unit);
      }
      notifyUser("\nProbe excluded surface: " + std::to_string(data.getSurfProbeExcluded()) + " ");
      notifyUser(surf_unit);
      notifyUser("\nProbe accessible surface: " + std::to_string(data.getSurfProbeAccessible()) + " ");
      notifyUser(surf_unit);
    }
  }
  else{
    notifyUser("\nCalculation failed!");
  }
  return data.success;
}

void Ctrl::clearOutput(){
  if (_to_gui) {
    gui->clearOutputText();
    gui->clearOutputGrid();
  }
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

void Ctrl::updateStatus(const std::string str){
  if (_to_gui) {
    gui->setStatus(str);
  }
  else {
    std::cout << str << std::endl;
  }
}

void Ctrl::exportReport(std::string path){
  current_calculation->createReport(path);
}

void Ctrl::exportReport(){
  current_calculation->createReport();
}

void Ctrl::exportSurfaceMap(const std::string path, bool cavities){
  if(cavities){
    current_calculation->writeCavitiesMaps(path);
  }
  else{
    current_calculation->writeTotalSurfaceMap(path);
  }
}

void Ctrl::exportSurfaceMap(bool cavities){
  if(cavities){
    current_calculation->writeCavitiesMaps();
  }
  else{
    current_calculation->writeTotalSurfaceMap();
  }
}

////////////////////////
// CALCULATION STATUS //
////////////////////////

void Ctrl::newCalculation(){
  _calculation_finished = false;
}

void Ctrl::calculationDone(const bool state){
  _calculation_finished = state;
}

bool Ctrl::isCalculationDone(){
  return _calculation_finished;
}

////////////////////
// ERROR MESSAGES //
////////////////////

static const std::map<int, std::string> s_error_codes = {
  {101, "Total number of cavities (255) exceeded. Consider changing the probe size. Calculation will proceed."}
};

void Ctrl::displayErrorMessage(const int error_code){
  if (_to_gui){
    gui->openErrorDialog(error_code, getErrorMessage(error_code));
  }
  else{
    printErrorMessage(101);
  }
}

void Ctrl::printErrorMessage(const int error_code){
  std::cout << getErrorMessage(error_code) << std::endl;
}

std::string Ctrl::getErrorMessage(const int error_code){
  return s_error_codes.find(error_code)->second;
}
