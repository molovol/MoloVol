
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

Ctrl* Ctrl::s_instance = NULL;
MainFrame* Ctrl::s_gui = NULL;

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
  s_gui = inp_gui;
}

Ctrl* Ctrl::getInstance(){
  if(s_instance == NULL){
    s_instance = new Ctrl();
  }
  return s_instance;
}

bool Ctrl::loadRadiusFile(){
  // create an instance of the model class
  // ensures, that there is only ever one instance of the model class
  if(_current_calculation == NULL){
    _current_calculation = new Model();
  }

  std::string radius_filepath = s_gui->getRadiusFilepath();
  // even if there is no valid radii file, the program can be used by manually setting radii in the GUI after loading a structure
  if(!_current_calculation->importRadiusFile(radius_filepath)){
    displayErrorMessage(101);
  }
  // refresh atom list using new radius map
  s_gui->displayAtomList(_current_calculation->generateAtomList());
  return true;
}

bool Ctrl::loadAtomFile(){
  // create an instance of the model class
  // ensures, that there is only ever one instance of the model class
  if(_current_calculation == NULL){
    _current_calculation = new Model();
  }

  bool successful_import;
  try{successful_import = _current_calculation->readAtomsFromFile(s_gui->getAtomFilepath(), s_gui->getIncludeHetatm());}

  catch (const ExceptIllegalFileExtension& e){
    displayErrorMessage(103);
    successful_import = false;
  }
  catch (const ExceptInvalidInputFile& e){
    displayErrorMessage(102);
    successful_import = false;
  }

  s_gui->displayAtomList(_current_calculation->generateAtomList()); // update gui

  return successful_import;
}

// default function call: transfer data from GUI to Model
bool Ctrl::runCalculation(){
  // reset abort flag
  setAbortFlag(false);
  s_gui->extSetProgressBar(0);
  // create an instance of the model class
  // ensures, that there is only ever one instance of the model class
  if(_current_calculation == NULL){
    _current_calculation = new Model();
  }

  // PARAMETERS
  // save parameters in model
  if(!_current_calculation->setParameters(
      s_gui->getAtomFilepath(),
      s_gui->getOutputDir(),
      s_gui->getIncludeHetatm(),
      s_gui->getAnalyzeUnitCell(),
      s_gui->getCalcSurfaceAreas(),
      s_gui->getProbeMode(),
      s_gui->getProbe1Radius(),
      s_gui->getProbe2Radius(),
      s_gui->getGridsize(),
      s_gui->getDepth(),
      s_gui->getMakeReport(),
      s_gui->getMakeSurfaceMap(),
      s_gui->getMakeCavityMaps(),
      s_gui->generateRadiusMap(),
      s_gui->getIncludedElements(),
      s_gui->getMaxRad())){
    return false;
  }

  // CALCULATION
  CalcReportBundle data = _current_calculation->generateData();
  calculationDone(data.success);

  // OUTPUT
  clearOutput();
  if(data.success){
    if(!data.cavities.empty()){s_gui->extDisplayCavityList(data.cavities);}
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
    if(data.calc_surface_areas && !Ctrl::getInstance()->getAbortFlag()){
      std::wstring surf_unit = Symbol::angstrom() + Symbol::squared();
      notifyUser("\n<SURFACE>");
      notifyUser("\nVan der Waals surface: " + std::to_string(data.getSurfVdw()) + " ");
      notifyUser(surf_unit);
      if(data.probe_mode){
        notifyUser("\nMolecular surface: " + std::to_string(data.getSurfMolecular()) + " ");
        notifyUser(surf_unit);
      }
      notifyUser("\n"+prefix+"robe excluded surface: " + std::to_string(data.getSurfProbeExcluded()) + " ");
      notifyUser(surf_unit);
      notifyUser("\n"+prefix+"robe accessible surface: " + std::to_string(data.getSurfProbeAccessible()) + " ");
      notifyUser(surf_unit);
    }
  }
  else{
    if(!getAbortFlag()){displayErrorMessage(200);}
  }
  updateStatus((data.success && !Ctrl::getInstance()->getAbortFlag())? "Calculation done." : "Calculation aborted.");

  if (data.success){
    // export if appropriate option is toggled
    if(data.make_report){exportReport();}
    if(data.make_full_map){exportSurfaceMap(false);}
    if(data.make_cav_maps){exportSurfaceMap(true);}
  }

  return data.success;
}

void Ctrl::clearOutput(){
  if (_to_gui) {
    s_gui->extClearOutputText();
    s_gui->extClearOutputGrid();
  }
}

void Ctrl::notifyUser(std::string str){
  if (_to_gui){
    s_gui->extAppendOutput(str);
  }
  else {
    std::cout << str;
  }
}

void Ctrl::notifyUser(std::wstring wstr){
  if (_to_gui){
    s_gui->extAppendOutputW(wstr);
  }
  else {
    std::cout << wstr;
  }
}

void Ctrl::updateStatus(const std::string str){
  if (_to_gui) {
    s_gui->extSetStatus(str);
  }
  else {
    std::cout << str << std::endl;
  }
}

void Ctrl::updateProgressBar(const int percentage){
  assert (percentage <= 100);
  if (_to_gui) {
    s_gui->extSetProgressBar(percentage);
  }
  else {
    std::cout << std::to_string(percentage) + "\%"  << std::endl;
  }
}

void Ctrl::exportReport(std::string path){
  _current_calculation->createReport(path);
  if(s_gui->getAnalyzeUnitCell()){
    _current_calculation->writeCrystStruct(path);
  }
}

void Ctrl::exportReport(){
  _current_calculation->createReport();
  if(s_gui->getAnalyzeUnitCell()){
    _current_calculation->writeCrystStruct();
  }
}

void Ctrl::exportSurfaceMap(const std::string path, bool cavities){
  if(cavities){
    _current_calculation->writeCavitiesMaps(path);
  }
  else{
    _current_calculation->writeTotalSurfaceMap(path);
  }
}

void Ctrl::exportSurfaceMap(bool cavities){
  if(cavities){
    _current_calculation->writeCavitiesMaps();
  }
  else{
    _current_calculation->writeTotalSurfaceMap();
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

void Ctrl::setAbortFlag(const bool state){
  _abort_calculation = state;
}

bool Ctrl::getAbortFlag(){
  return _abort_calculation;
}

// checks whether worker thread has received a signal to stop the calculation and
// updates the progress of the calculation
void Ctrl::updateCalculationStatus(){
  if (_to_gui){
    setAbortFlag(s_gui->receivedAbortCommand());
  }
}

////////////////////
// ERROR MESSAGES //
////////////////////

static const std::map<int, std::string> s_error_codes = {
  {0, "Unidentified error code."}, // no user should ever see this
  // 1xx: Invalid Input
  {100, "Import failed!"},
  {101, "Invalid radius definition file. Please select a valid file or set radii manually."},
  {102, "Invalid structure file. Please select a valid file."},
  {103, "Invalid file format. Please make sure that the input files have the correct file extensions."},
  {104, "Invalid probe radius input. The large probe must have a larger radius than the small probe."},
  {105, "Invalid entry in structure file encountered. Some atoms have not been imported. Please check the format of the input file."},
  {106, "Invalid element symbol(s) in radius file detected. Some radii may be assigned incorrectly. Please make sure that all element symbols begin with an alphabetic character."},
  {107, "Invalid radius value in radius file detected. Some radii may be set to 0. Please make sure that all radii are numeric."},
  // 11x: unit cell files
  {111, "Space group not found. Check the structure file, or untick the Unit Cell Analysis tickbox."},
  {112, "Invalid unit cell parameters. Check the structure file, or untick the Unit Cell Analysis tickbox."},
  {113, "Space group or symmetry not found. Check the structure and space group files or untick the Unit Cell Analysis tickbox"},
  {114, "Invalid ATOM or HETATM line encountered. Import may be incomplete. Check the structure file."},
  // 2xx: Issue during Calculation
  {200, "Calculation failed!"},
  {201, "Total number of cavities (255) exceeded. Consider changing the probe size. Calculation will proceed."},
  // 3xx: Issue with Output
  {300, "Output failed!"},
  {301, "Data missing to export file. Calculation may be still running or has not been started."},
  {302, "Invalid output directory. Please select a valid output directory."},
  {303, "An unidentified issue has been encountered while writing the surface map."},
  // 9xx: Issues with command line arguments
  {900, "Command line interface failed!"},
  {901, "At least one required command line argument missing."}
};

void Ctrl::displayErrorMessage(const int error_code){
  if (_to_gui){
    s_gui->extOpenErrorDialog(error_code, getErrorMessage(error_code));
  }
  else{
    printErrorMessage(error_code);
  }
}

void Ctrl::printErrorMessage(const int error_code){
  std::cout << getErrorMessage(error_code) << std::endl;
}

std::string Ctrl::getErrorMessage(const int error_code){
  if (s_error_codes.find(error_code) == s_error_codes.end()) {
    return s_error_codes.find(0)->second;
  }
  else {
    return s_error_codes.find(error_code)->second;
  }
}
