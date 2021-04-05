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

bool MainFrame::getAnalyzeUnitCell(){
  return unitCellCheckbox->GetValue();
}

bool MainFrame::getCalcSurfaceAreas(){
  return surfaceAreaCheckbox->GetValue();
}

bool MainFrame::getProbeMode(){
  return twoProbesCheckbox->GetValue();
}

double MainFrame::getProbe1Radius(){
  return std::stod(probe1InputText->GetValue().ToStdString());
}

double MainFrame::getProbe2Radius(){
  if(getProbeMode()){
    return std::stod(probe2InputText->GetValue().ToStdString());
  }
  else{ // when the probe mode is set to one probe, the second probe radius is considered null
    return 0;
  }
}

double MainFrame::getGridsize(){
  return std::stod(gridsizeInputText->GetValue().ToStdString());
}

int MainFrame::getDepth(){
  return depthInput->GetValue();
}

bool MainFrame::getMakeReport(){
  return reportCheckbox->GetValue();
}

bool MainFrame::getMakeSurfaceMap(){
  return surfaceMapCheckbox->GetValue();
}

bool MainFrame::getMakeCavityMaps(){
  return cavityMapsCheckbox->GetValue();
}

std::string MainFrame::getOutputDir(){
  return outputdirPicker->GetPath().ToStdString();
}

void MainFrame::displayAtomList(std::vector<std::tuple<std::string, int, double>> symbol_number_radius){
  // delete all rows
  // DeleteRows causes an error if there are no rows
  if (atomListGrid->GetNumberRows() > 0) {
    atomListGrid->DeleteRows(0, atomListGrid->GetNumberRows());
  }

  for (size_t row = 0; row < symbol_number_radius.size(); row++) {
    atomListGrid->AppendRows(1, true);
    // column 1 (symbol of atom)
    atomListGrid->SetCellValue(row, 1, std::get<0>(symbol_number_radius[row]));
    atomListGrid->SetReadOnly(row,1,true);
    // column 2 (number of atom)
    atomListGrid->SetCellValue(row, 2, std::to_string(std::get<1>(symbol_number_radius[row])));
    atomListGrid->SetReadOnly(row,2,true);
    // column 3 (radius of atom)
    atomListGrid->SetCellValue(row, 3, std::to_string(std::get<2>(symbol_number_radius[row])));
    // column 0 (include checkbox)
    // if no radius is found for the element, color cells to point it to the user
    if (std::wcstod(atomListGrid->GetCellValue(row, 3), NULL) == 0){
      atomListGrid->SetCellBackgroundColour(row, 1, col_grey_cell);
      atomListGrid->SetCellBackgroundColour(row, 2, col_grey_cell);
      atomListGrid->SetCellBackgroundColour(row, 3, col_red_cell);
    }
    else { // if a radius is found, include by default the element
      atomListGrid->SetCellValue(row, 0, "1");
    }
    // refresh the grid to enforce the update of cell values and parameters
    // without this, it was sometimes observed that the last cell was not updated properly
    atomListGrid->ForceRefresh();
  }
}

std::unordered_map<std::string, double> MainFrame::generateRadiusMap(){
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

double MainFrame::getMaxRad(){
  double max_rad = 0;
  double radius = 0;
  for (int i = 0; i < atomListGrid->GetNumberRows(); i++){
    if (atomListGrid->GetCellValue(i,0) == "1"){
      atomListGrid->GetCellValue(i,3).ToDouble(&radius);
      if (radius > max_rad){
        max_rad = radius;
      }
    }
  }
  return max_rad;
}

std::vector<std::string> MainFrame::getIncludedElements(){
  std::vector<std::string> included_elements;
  for (int i = 0; i < atomListGrid->GetNumberRows(); i++){
    if (atomListGrid->GetCellValue(i,0) == "1"){
        included_elements.emplace_back(atomListGrid->GetCellValue(i,1).wxString::ToStdString());
    }
  }
  return included_elements;
}
