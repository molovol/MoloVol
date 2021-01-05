
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
  if(!current_calculation->readRadiiAndAtomNumFromFile(radius_filepath)){
    notifyUser("Invalid radii definition file!");
    notifyUser("Please select a valid file or set radii manually.");
  }
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
  return runCalculation(
      gui->getAtomFilepath(),
      gui->getGridsize(),
      gui->getDepth(),
      gui->generateRadiusMap(),
      gui->getProbe1Radius(),
      gui->getProbe2Radius(),
      gui->getProbeMode(),
      gui->getAnalyzeUnitCell(),
      gui->getMaxRad(),
      gui->getIncludedElements(),
      gui->generateChemicalFormulaFromGrid());
}

bool Ctrl::runCalculation(
    std::string atom_filepath,
    double grid_step,
    int max_depth,
    std::unordered_map<std::string, double> rad_map,
    double rad_probe1,
    double rad_probe2,
    bool option_probe_mode,
    bool option_unit_cell,
    double max_rad,
    std::vector<std::string> included_elements,
    std::string chemical_formula){
  // create an instance of the model class
  // ensures, that there is only ever one instance of the model class
  if(current_calculation == NULL){
    current_calculation = new Model();
  }

  if(!current_calculation->setProbeRadii(rad_probe1, rad_probe2, option_probe_mode)){
    return false; // abort calculation if radius2 is smaller than radius 1
  }

  current_calculation->setRadiusMap(rad_map); // what is this for?
  
  /* no point in making folders for each calculation
  if(!current_calculation->createOutputFolder()){
    notifyUser("New output folder could not be created.\nThe output file(s) will be created in the program folder.");
  }
  */

  // process atom data for unit cell analysis if the option it ticked
  if(option_unit_cell){
    if(!current_calculation->processUnitCell(max_rad, rad_probe1, rad_probe2, grid_step)){
      return false;
    }
  }

  current_calculation->setAtomListForCalculation(included_elements, option_unit_cell); // what is this for?
  current_calculation->storeAtomsInTree(); // place atoms in a binary tree for faster access

  notifyUser("Result for " + chemical_formula);

  current_calculation->defineCell(grid_step, max_depth); // set size of the box containing all atoms
  
//  current_calculation->linkAtomsToAdjacentAtoms(rad_probe1); // currently not in use

  // generate result report
  /*
  std::vector<std::string> parameters = getGuiParameters();
  current_calculation->createReport(atom_filepath, parameters);
  */

  // measure time and run calculation
  auto start = std::chrono::steady_clock::now();
  current_calculation->calcVolume(); // assign voxel types and get the volume
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;

  notifyUser("Elapsed time: " + std::to_string(elapsed_seconds.count()) + " s");

  return true;
}

// generate parameter list for report
std::vector<std::string> Ctrl::getGuiParameters(){
  std::vector<std::string> parameters;
  if(fileExtension(gui->getAtomFilepath()) == "pdb"){

    parameters.push_back(std::string(((gui->getIncludeHetatm())? "Include" : "Exclude")) 
        + std::string(" HETATM from pdb file"));

    if(gui->getAnalyzeUnitCell()){
      parameters.push_back("Analyze crystal structure unit cell");
    }
  }
  if(gui->getProbeMode()){
    parameters.push_back("Probe mode: two probes");
    parameters.push_back("Probe 1 radius: " + std::to_string(gui->getProbe1Radius()) + " A");
    parameters.push_back("Probe 2 radius: " + std::to_string(gui->getProbe2Radius()) + " A");
  }
  else{
    parameters.push_back("Probe mode: one probe");
    parameters.push_back("Probe radius: " + std::to_string(gui->getProbe1Radius()) + " A");
  }
  parameters.push_back("Grid step size (resolution): " + std::to_string(gui->getGridsize()) + " A");
  parameters.push_back("Maximum tree depth (algorithm acceleration): " + std::to_string(gui->getDepth()));
  return parameters;
}

void Ctrl::notifyUser(std::string str){
  str = "\n" + str;
  gui->appendOutput(str);
}
