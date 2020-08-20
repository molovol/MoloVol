#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#  include <wx/wx.h>
#endif

#include "base.h"
#include "special_chars.h"
#include <string>

//////////////////////////////////
// METHODS FOR MANIPULATING GUI //
//////////////////////////////////

void MainFrame::clearOutput(){
  outputText->SetValue("");
}

void MainFrame::printToOutput(std::string& text){
  outputText->SetValue(text);
}

void MainFrame::appendOutput(std::string& text){
  outputText->SetValue(outputText->GetValue() + text);
}

std::string MainFrame::getAtomFilepath(){
  return filepathText->GetValue().ToStdString();
}

std::string MainFrame::getRadiusFilepath(){
  return radiuspathText->GetValue().ToStdString();
}

bool MainFrame::getIncludeHetatm(){
  return pdbHetatmCheckbox->GetValue();
}

double MainFrame::getGridsize(){
  return std::stod(gridsizeInputText->GetValue().ToStdString());
}

int MainFrame::getDepth(){
  return depthInput->GetValue();
}

//* get from user input
double MainFrame::getProbeRadius(){
  return r_probe;
}

void MainFrame::displayAtomList(std::vector<std::tuple<std::string, int, double>> symbol_number_radius){
  // delete all rows
  // DeleteRows causes an error if there are no rows
  if (atomListGrid->GetNumberRows() > 0) {
//    atomListGrid->ClearGrid(); // seems unneccessary
    atomListGrid->DeleteRows(0, atomListGrid->GetNumberRows());
  }

  for (int row = 0; row < symbol_number_radius.size(); row++) {
    atomListGrid->AppendRows(1, true);
    // column 0 (include checkbox)
    atomListGrid->SetCellValue("1",row,0);
    // column 1 (symbol of atom)
    atomListGrid->SetCellValue(std::get<0>(symbol_number_radius[row])                  , row, 1);
    atomListGrid->SetReadOnly(row,1,true);
    // column 2 (number of atom)
    atomListGrid->SetCellValue(std::to_string(std::get<1>(symbol_number_radius[row]))  , row, 2);
    atomListGrid->SetReadOnly(row,2,true);
    // column 3 (radius of atom)
    atomListGrid->SetCellValue(std::to_string(std::get<2>(symbol_number_radius[row]))  , row, 3);
  }
  return;
}

std::string MainFrame::generateChemicalFormulaFromGrid(){
  std::string chemical_formula_suffix = "";
  std::string chemical_formula_prefix = "";
  for (int row = 0; row < atomListGrid->GetNumberRows(); row++){
    if (atomListGrid->GetCellValue(row,0) == "1"){ // if checkbox "include" is checked
      std::string symbol = atomListGrid->GetCellValue(row,1).ToStdString();
      std::string subscript = Symbol::subscript(atomListGrid->GetCellValue(row,2).ToStdString());

      // by convention: carbon comes first, then hydrogen, then in alphabetical order
      if (symbol == "C"){
        chemical_formula_prefix = symbol + subscript + chemical_formula_prefix;
      }
      else if (symbol == "H"){
        chemical_formula_prefix += symbol + subscript;
      }

      else {
        chemical_formula_suffix += symbol + subscript;
      }

    }
  }
  return chemical_formula_prefix + chemical_formula_suffix;
}

std::unordered_map<std::string, double> MainFrame::generateRadiusMapFromView(){
  std::unordered_map<std::string, double> radius_map;
  for (int i = 0; i < atomListGrid->GetNumberRows(); i++){
    if (atomListGrid->GetCellValue(i,0) == "1"){
      std::string symbol = atomListGrid->GetCellValue(i,1).wxString::ToStdString();
      double radius;
      atomListGrid->GetCellValue(i,3).ToDouble(&radius);
      radius_map[symbol] = radius;
    }
  }
  return radius_map;
}
