
#include "controller.h"
#include "base.h"
#include "atom.h" // i don't know why
#include "model.h"
#include <vector>

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
    
bool Ctrl::runCalculation(std::string& atom_filepath){ //* std::string& radius_filepath
  return false;
}

bool Ctrl::runCalculation(){ 
  
  // create an instance of the model class
  current_calculation = new Model();
  
  std::string atom_filepath = gui->getAtomFilepath();
  std::string radius_filepath = gui->getRadiusFilepath();
  // read atoms from file and save a vector containing the atoms
  current_calculation->readRadiiFromFile(radius_filepath);
  current_calculation->readAtomsFromFile(atom_filepath);
  
  const double grid_step = gui->getGridsize();//0.1; // get from user
  const int max_depth = gui->getDepth();//4; // get from user
  // set space size (size of unit cell/ box containing all atoms)
  current_calculation->defineCell(grid_step, max_depth);
  
  current_calculation->calcVolume();
  
  // display to user
  std::string text = "Done";
  gui->printToOutput(text);

  return true;
}

