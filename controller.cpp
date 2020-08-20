
#include "controller.h"
#include "base.h"
#include "atom.h" // i don't know why
#include "model.h"
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

bool Ctrl::loadInputFiles(){
  // create an instance of the model class
  // ensures, that there is only ever one instance of the model class
  if(current_calculation == NULL){
    current_calculation = new Model();
  }

  std::string atom_filepath = gui->getAtomFilepath();
  std::string radius_filepath = gui->getRadiusFilepath();
  
  if (current_calculation->filesExist(atom_filepath, radius_filepath)){
    // read atoms from file and save a vector containing the atoms
    current_calculation->importFiles(atom_filepath, radius_filepath);

    // get atom list from model and pass onto view
    gui->displayAtomList(current_calculation->generateAtomList());
  }
  else{
    notifyUser("Invalid File Path!");
    return false;
  }
  return true;
}

bool Ctrl::runCalculation(){
  
  std::string atom_filepath = gui->getAtomFilepath();
  std::string radius_filepath = gui->getRadiusFilepath();
 
  // if import files have changed "press" the load button
  if (current_calculation->importFilesChanged(atom_filepath, radius_filepath)){
    if (!loadInputFiles()) {return false;} // if loading unsuccessful, abort calculation
  }

  if(current_calculation == NULL){
    current_calculation = new Model();
  }
  
  // radius map is generated from grid in gui, then passed to model for calculation
  current_calculation->setRadiusMap(gui->generateRadiusMapFromView());
  current_calculation->updateAtomRadii();

  Ctrl::notifyUser("Result for " + gui->generateChemicalFormulaFromGrid());

//  std::string atom_filepath = gui->getAtomFilepath();
 // import already done by load button
//  current_calculation->importFiles(atom_filepath);

//  current_calculation->readAtomsFromFile(atom_filepath);
//  current_calculation->storeAtomsInTree(); // TODO consider moving this to readAtomsFromFile method in model class
  // get user inputs
  const double grid_step = gui->getGridsize();
  const int max_depth = gui->getDepth();
  const double r_probe = gui->getProbeRadius();
  // set space size (size of box containing all atoms)
  current_calculation->defineCell(grid_step, max_depth);
  current_calculation->findCloseAtoms(r_probe);
  
  // measure time and run calculation
  auto start = std::chrono::steady_clock::now();
  current_calculation->calcVolume();
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;

  notifyUser("Elapsed time: " + std::to_string(elapsed_seconds.count()) + " s");
  
  return true;
}

void Ctrl::notifyUser(std::string str){
  str = "\n" + str;
  gui->appendOutput(str);
}


