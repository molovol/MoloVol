
#include "controller.h"
#include "base.h"
#include "atom.h" // i don't know why
#include "model.h"
#include "misc.h"
#include <chrono>
#include <utility>

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
  current_calculation->readRadiiAndAtomNumFromFile(radius_filepath);
  // Refresh atom list with new radius map
  gui->displayAtomList(current_calculation->generateAtomList());
  return true;
}

bool Ctrl::loadAtomFile(){

  // create an instance of the model class
  // ensures, that there is only ever one instance of the model class
  if(current_calculation == NULL){
    current_calculation = new Model();
  }

  std::string atom_filepath = gui->getAtomFilepath();

  if (!current_calculation->readAtomsFromFile(atom_filepath, gui->getIncludeHetatm())){
    // refresh the atom list which is now empty due to invalid input file
    gui->displayAtomList(current_calculation->generateAtomList());
    return false;
  }
  else {
    // get atom list from model and pass onto view
    gui->displayAtomList(current_calculation->generateAtomList());
    return true;
  }
}

bool Ctrl::runCalculation(){
  // create an instance of the model class
  // ensures, that there is only ever one instance of the model class
  if(current_calculation == NULL){
    current_calculation = new Model();
  }

  // stop calculation if probe 2 radius is too small in two probes mode
  if(gui->getProbeMode() && gui->getProbe1Radius() > gui->getProbe2Radius()){
    notifyUser("Probes radii invalid!\nSet probe 2 radius > probe 1 radius.");
    return false;
  }

  // radius map is generated from grid in gui, then passed to model for calculation
  current_calculation->setRadiusMap(gui->generateRadiusMap());

  if(!current_calculation->createOutputFolder()){
    notifyUser("New output folder could not be created.\nThe output file(s) will be created in the program folder.");
  }

  // process atom data for unit cell analysis if the option it ticked
  if(gui->getAnalyzeUnitCell()){
    if(!current_calculation->processUnitCell(gui->getMaxRad(), gui->getProbe1Radius(), gui->getProbe2Radius(), gui->getGridsize())){
      return false;
    }
  }

  current_calculation->setAtomListForCalculation(gui->getIncludedElements(), gui->getAnalyzeUnitCell());
  current_calculation->storeAtomsInTree();

  notifyUser("Result for " + gui->generateChemicalFormulaFromGrid());

  // get user inputs
  const double grid_step = gui->getGridsize();
  const int max_depth = gui->getDepth();
  const double r_probe = gui->getProbe1Radius();
  // set space size (size of box containing all atoms)
  current_calculation->defineCell(grid_step, max_depth);
  current_calculation->findCloseAtoms(r_probe);

  // generate result report
  std::vector<std::string> parameters;
  getGuiParameters(parameters);
  current_calculation->createReport(gui->getAtomFilepath(), parameters);

  // measure time and run calculation
  auto start = std::chrono::steady_clock::now();
  current_calculation->calcVolume();
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;

  notifyUser("Elapsed time: " + std::to_string(elapsed_seconds.count()) + " s");

  return true;
}

// generate parameter list for report
void Ctrl::getGuiParameters(std::vector<std::string> &parameters){
  if(fileExtension(gui->getAtomFilepath()) == "pdb"){
    if(gui->getIncludeHetatm()){
      parameters.emplace_back("Include HETATM from pdb file");
    }
    else{
      parameters.emplace_back("Exclude HETATM from pdb file");
    }
    if(gui->getAnalyzeUnitCell()){
      parameters.emplace_back("Analyze crystal structure unit cell");
    }
  }
  if(gui->getProbeMode()){
    parameters.emplace_back("Probe mode: two probes");
    parameters.emplace_back("Probe 1 radius: " + std::to_string(gui->getProbe1Radius()) + " A");
    parameters.emplace_back("Probe 2 radius: " + std::to_string(gui->getProbe2Radius()) + " A");
  }
  else{
    parameters.emplace_back("Probe mode: one probe");
    parameters.emplace_back("Probe radius: " + std::to_string(gui->getProbe1Radius()) + " A");
  }
  parameters.emplace_back("Grid step size (resolution): " + std::to_string(gui->getGridsize()) + " A");
  parameters.emplace_back("Maximum tree depth (algorithm acceleration): " + std::to_string(gui->getDepth()));
}

void Ctrl::notifyUser(std::string str){
  str = "\n" + str;
  gui->appendOutput(str);
}
