
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
  // create an instance of the model class
  current_calculation = new Model();
  
  // read atoms from file and save a vector containing the atoms
  current_calculation->readAtomsFromFile(filepath);
  
  // set space size (size of unit cell/ box containing all atoms)
  const double grid_size = 0.1;
  // find min and max of coordinates
  current_calculation->defineCell();
//  current_calculation->debug();

  // display to user
  std::string text = "Done";
  gui->printToOutput(text);

  return true;
}

