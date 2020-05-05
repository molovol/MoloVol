
#include "controller.h"
#include "base.h"
#include "atom.h"
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
  
  // read atoms from file
  current_calculation->readAtomsFromFile(filepath);

  // display to user
  std::string text = "Done";
  gui->printToOutput(text);
  
  return true;
}

