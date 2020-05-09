
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
    
bool Ctrl::runCalculation(std::string& filepath){
  // controller receives:
  //  filepath to xyz file
  //  filepath to radii file

  // create an instance of the model class
  current_calculation = new Model();
  
  // read atoms from file and save a vector containing the atoms
  std::string radii_file = "./inputfile/radii.txt"; //* this has to be generalised. maybe file selection for user?
  
  current_calculation->readRadiiFromFile(radii_file);
  current_calculation->readAtomsFromFile(filepath);
  // set space size (size of unit cell/ box containing all atoms)
  const double grid_step = 0.1; // get from user
  const int max_depth = 4; // get from user
  current_calculation->defineCell(grid_step, max_depth);
  
  current_calculation->calcVolume();
  
  // display to user
  std::string text = "Done";
  gui->printToOutput(text);

  return true;
}

