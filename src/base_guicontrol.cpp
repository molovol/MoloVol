#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#  include <wx/wx.h>
#endif

#include "base.h"
#include "special_chars.h"
#include "misc.h"
#include "flags.h"
#include "container3d.h"
#include "voxel.h"
#include <string>
#include <wx/msgdlg.h>

//////////////////////////////////
// METHODS FOR MANIPULATING GUI //
//////////////////////////////////

// THREAD SAFE

void MainFrame::extClearOutputText(){
  GetEventHandler()->CallAfter(&MainFrame::clearOutputText);
}

void MainFrame::extClearOutputGrid(){
  GetEventHandler()->CallAfter(&MainFrame::clearOutputGrid);
}

void MainFrame::extAppendOutput(const std::string text){
  GetEventHandler()->CallAfter(&MainFrame::appendOutput, text);
}

void MainFrame::extAppendOutputW(const std::wstring wtext){
  GetEventHandler()->CallAfter(&MainFrame::appendOutputW, wtext);
}

void MainFrame::extSetStatus(const std::string status){
  GetEventHandler()->CallAfter(&MainFrame::setStatus, status);
}

void MainFrame::extSetProgressBar(const int percentage){
  GetEventHandler()->CallAfter(&MainFrame::setProgressBar, percentage);
}

void MainFrame::extDisplayCavityList(const GridData& table){
  GetEventHandler()->CallAfter(&MainFrame::displayCavityList, table);
}

void MainFrame::extOpenErrorDialog(const int error_code, const std::string& error_message){
  const std::pair<int, std::string> code_message = std::make_pair(error_code, error_message);
  GetEventHandler()->CallAfter(&MainFrame::openErrorDialog, code_message);
}

void MainFrame::extRenderSurface(const Container3D<Voxel>& surf_data, const std::array<double,3> origin, 
    const double grid_step, const bool probe_mode, const unsigned char n_cavities, const std::vector<Atom>& atomlist){

  auto render = [this, surf_data, origin, grid_step, probe_mode, n_cavities, atomlist](){
    renderMolecule(atomlist);
    renderSurface(surf_data, origin, grid_step, std::make_pair(probe_mode, n_cavities));
  };

  GetEventHandler()->CallAfter(render);
}

// NOT THREAD SAFE

void MainFrame::clearOutputText(){
  outputText->SetValue("");
}

void MainFrame::clearOutputGrid(){
  if (outputGrid->GetNumberRows() > 0){
    outputGrid->DeleteRows(0, outputGrid->GetNumberRows());
  }
  if (outputGrid->GetNumberCols() > 0){
    outputGrid->DeleteCols(0, outputGrid->GetNumberCols());
  }
}

void MainFrame::printToOutput(const std::string text){
  outputText->SetValue(text);
}

void MainFrame::appendOutput(const std::string text){
  outputText->SetValue(outputText->GetValue() + text);
}

void MainFrame::appendOutputW(const std::wstring text){
  outputText->SetValue(outputText->GetValue() + text);
}

std::string MainFrame::getAtomFilepath(){
  return filepathText->GetValue().ToStdString();
}

std::string MainFrame::getElementsFilepath(){
  return elementspathText->GetValue().ToStdString();
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
  try {
    return std::stod(probe1DropDown->GetValue().ToStdString());
  }
  catch (const std::invalid_argument& e){
    throw;
  }
}

double MainFrame::getProbe2Radius(){
  try{
    return getProbeMode()? std::stod(probe2InputText->GetValue().ToStdString()) : 0;
  }
  catch (const std::invalid_argument& e){
    throw;
  }
}

double MainFrame::getGridsize(){
  try{
    return std::stod(gridsizeInputText->GetValue().ToStdString());
  }
  catch (const std::invalid_argument& e){
    throw;
  }
}

int MainFrame::getDepth() const {
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
  return dirpickerText->GetValue().ToStdString();
}

void MainFrame::displayAtomList(std::vector<std::tuple<std::string, int, double>> symbol_number_radius){
  // delete all rows
  // DeleteRows causes an error if there are no rows
  if (atomListGrid->GetNumberRows() > 0) {
    atomListGrid->DeleteRows(0, atomListGrid->GetNumberRows());
  }

  for (size_t row = 0; row < symbol_number_radius.size(); row++) {
    atomListGrid->AppendRows(1, true);
    // column 0 (include tick box)
    atomListGrid->SetCellAlignment(row, 0, wxALIGN_CENTER, wxALIGN_CENTER);
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
      atomListGrid->SetCellBackgroundColour(row, 1, _col_grey_cell);
      atomListGrid->SetCellBackgroundColour(row, 2, _col_grey_cell);
      atomListGrid->SetCellBackgroundColour(row, 3, _col_red_cell);
    }
    else { // if a radius is found, include by default the element
      atomListGrid->SetCellValue(row, 0, "1");
    }
    // refresh the grid to enforce the update of cell values and parameters
    // without this, it was sometimes observed that the last cell was not updated properly
    atomListGrid->ForceRefresh();
  }
}

void MainFrame::displayCavityList(const GridData& table_data){
  // delete all rows and columns
  clearOutputGrid();
  // add appropriate number of rows and columns
  outputGrid->AppendCols(table_data.getNumberCols());
  outputGrid->AppendRows(table_data.getNumberRows(false));

  for (int col = 0; col < outputGrid->GetNumberCols(); ++col){
    // add grid values
    for (int row = 0; row < outputGrid->GetNumberRows(); ++row){
      outputGrid->SetCellValue(row, col, table_data.getValue(row, col));
      outputGrid->SetReadOnly(row, col, true);
    }
    // add column labels
    outputGrid->SetColLabelValue(col, wxString(table_data.getHeader(col)) + "\n" + wxString(table_data.getUnit(col)));
    outputGrid->AutoSizeColumn(col);
    if (table_data.hideCol(col)){
      outputGrid->SetColSize(col, 0);
    }
    // set cell format
    switch (table_data.getFormat(col)){
      case mvFORMAT_NUMBER :
        outputGrid->SetColFormatNumber(col);
        break;
      case mvFORMAT_FLOAT :
        outputGrid->SetColFormatFloat(col, 5, 3);
        break;
    }
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

void MainFrame::setStatus(const std::string str){
  statusBar->SetStatusText(str);
}

void MainFrame::setProgressBar(const int percentage){
  progressGauge->SetValue(percentage);
}

void MainFrame::renderMolecule(const std::vector<Atom>& atomlist){
#ifdef MOLOVOL_RENDERER
  m_renderWin->UpdateMolecule(atomlist);
#endif
}

void MainFrame::renderSurface(const Container3D<Voxel>& surf_data, const std::array<double,3> origin, 
    const double grid_step, const std::pair<bool,unsigned char> args){
#ifdef MOLOVOL_RENDERER
  m_renderWin->UpdateSurface(surf_data, origin, grid_step, args.first, args.second);
  
  // Only render if window is currently visible
  if (m_renderWin->IsShown()) {
    m_renderWin->Render();
  }
  
  m_renderWin->Show(true);
#endif
}

////////////////////
// DIALOG POP UPS //
////////////////////

void MainFrame::openErrorDialog(const std::pair<int,std::string>& code_message){
  const int error_code = code_message.first;
  const std::string error_message = code_message.second;
  wxMessageDialog error_dialog(this, error_message, "Error Code " + std::to_string(error_code), wxICON_ERROR | wxOK | wxCENTRE);
  error_dialog.ShowModal();
}
