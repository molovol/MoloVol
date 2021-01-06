
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

// function that allows running a calculation with minimal inputs, without the GUI
bool Ctrl::runFromCommandLine(
    std::string atom_filepath,
    std::string radius_filepath,
    double grid_step,
    int max_depth,
    double rad_probe1){
  if(current_calculation == NULL){current_calculation = new Model();}

  std::unordered_map<std::string, double> rad_map = current_calculation->importRadiusMap(radius_filepath);
  current_calculation->setRadiusMap(rad_map);
  current_calculation->readAtomsFromFile(atom_filepath, false);

  std::vector<std::string> included_elements = current_calculation->listElementsInStructure();

  runCalculation(atom_filepath, grid_step, max_depth, rad_map, included_elements, rad_probe1);
  return true;
}

bool Ctrl::runCalculation(){
  return runCalculation(
      gui->getAtomFilepath(),
      gui->getGridsize(),
      gui->getDepth(),
      gui->generateRadiusMap(),
      gui->getIncludedElements(),
      gui->getProbe1Radius(),
      gui->getProbe2Radius(),
      gui->getProbeMode(),
      gui->getAnalyzeUnitCell(),
      gui->getMaxRad(),
      gui->generateChemicalFormulaFromGrid());
}

bool Ctrl::runCalculation(
    std::string atom_filepath,
    double grid_step,
    int max_depth,
    std::unordered_map<std::string, double> rad_map,
    std::vector<std::string> included_elements,
    double rad_probe1,
    double rad_probe2,
    bool option_probe_mode,
    bool option_unit_cell,
    double max_rad,
    std::string chemical_formula){
  // create an instance of the model class
  // ensures, that there is only ever one instance of the model class
  if(current_calculation == NULL){
    current_calculation = new Model();
  }

  if(!current_calculation->setProbeRadii(rad_probe1, rad_probe2, option_probe_mode)){
    return false; // abort calculation if radius2 is smaller than radius 1
  }

  // give a radius map to the model that may differ from the radius map that was originally imported
  current_calculation->setRadiusMap(rad_map);
  
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


  current_calculation->defineCell(grid_step, max_depth); // set size of the box containing all atoms
  
  // generate result report
  /*
  std::vector<std::string> parameters = getGuiParameters();
  current_calculation->createReport(atom_filepath, parameters);
  */

  // measure time and run calculation
  CalcResultBundle data = current_calculation->calcVolume(); // assign voxel types and get the volume

  notifyUser("Result for " + chemical_formula);
  notifyUser("Elapsed time: " + std::to_string(data.getTime()) + " s");
  notifyUser("VdW Volume: " + std::to_string(data.volumes['a']) + Symbol::angstrom() + Symbol::cubed());

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
