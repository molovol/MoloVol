
#include "controller.h"
#include "base.h"
#include "atom.h"
#include "filereading.h" // readAtomsFromFile
#include <vector>

Ctrl* Ctrl::instance = NULL;
MainFrame* Ctrl::gui = NULL;

void Ctrl::runCalculation(std::string& filepath){
  std::vector<Atom> atoms = readAtomsFromFile(filepath);
  
  gui->clearOutput();
  for (int i = 0; i< atoms.size(); i++){
    std::string out = "Atom " + std::to_string(i) + ": " + atoms[i].symbol + "\n";
    gui->appendOutput(out);
  }
  return;
}

void Ctrl::registerView(MainFrame* inp_gui){
  gui = inp_gui;
}

Ctrl* Ctrl::getInstance(){
  if(instance == NULL){
    instance = new Ctrl();
  }
  return instance;
}
    
    
        // implement this:
        //
    // void MainFrame::OnCalc(wxCommandEvent& event){
    //   std::string filepath = (filepathText->GetValue()).ToStdString();
    //   outputText->SetLabel(filepath);
    //   printToOutput(filepath);
    // 
    //   // so far only xyz files allowed
    //   std::vector<Atom> atoms = readAtomsFromFile(filepath);
    // 
    //   clearOutput();
    //   for (int i = 0; i< atoms.size(); i++){
    //     std::string out = "Atom " + std::to_string(i) + ": " + atoms[i].symbol + "\n";
    //     appendOutput(out);
    //   }
    // 
    //   return;
    // }
