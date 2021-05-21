#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#  include <wx/wx.h>
#endif

#include "base.h"
#include "controller.h"
#include "misc.h"
#include <vector>

/////////////////
// EVENT TABLE //
/////////////////

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
  EVT_BUTTON(BUTTON_Calc, MainFrame::OnCalc)
  EVT_BUTTON(BUTTON_Browse, MainFrame::OnAtomBrowse)
  EVT_BUTTON(BUTTON_Radius, MainFrame::OnRadiusBrowse)
  EVT_BUTTON(BUTTON_LoadFiles, MainFrame::OnLoadFiles)
  EVT_BUTTON(BUTTON_Report, MainFrame::OnExportReport)
  EVT_BUTTON(BUTTON_TotalMap, MainFrame::OnExportTotalMap)
  EVT_BUTTON(BUTTON_CavityMap, MainFrame::OnExportCavityMap)
  EVT_CHECKBOX(CHECKBOX_TwoProbes, MainFrame::ProbeModeChange)
  EVT_GRID_CELL_CHANGING(MainFrame::GridChange)
END_EVENT_TABLE()

////////////////////////////////
// METHODS FOR EVENT HANDLING //
////////////////////////////////

// exit program
void MainFrame::OnExit(wxCommandEvent& event){
  this->Close(TRUE);
}

// TODO remove obsolete debugging functions
// display text in output, used for debugging
void MainFrame::OnPrint(wxCommandEvent& event){
  std::string text = "treat yourself well";
  printToOutput(text);
}

// begin calculation
void MainFrame::OnCalc(wxCommandEvent& event){
  enableGuiElements(false);
  // saving locally, to avoid issues if user changes the tickbox during calculation (shouldn't be possible,
  // but this way it's double proofed)
  const bool probe_mode = getProbeMode();
  const bool surface_option = getCalcSurfaceAreas(); 

  // stop calculation if probe 2 radius is too small in two probes mode
  if(probe_mode && getProbe1Radius() > getProbe2Radius()){
    Ctrl::getInstance()->notifyUser("Probes radii invalid!\nSet probe 2 radius > probe 1 radius.");
    wxYield(); // without wxYield, the clicks on disabled buttons are queued
    enableGuiElements(true);
    return;
  }

  if(!Ctrl::getInstance()->runCalculation()){
    wxYield(); // without wxYield, the clicks on disabled buttons are queued
    enableGuiElements(true);
    return;
  }

  // write report file if option is toggled
  if(getMakeReport()){
    Ctrl::getInstance()->exportReport();
  }

  // write total surface map file if option is toggled
  if(getMakeSurfaceMap()){
    Ctrl::getInstance()->exportSurfaceMap(false);
  }

  if(getMakeCavityMaps()){
    Ctrl::getInstance()->exportSurfaceMap(true);
  }

  wxYield(); // without wxYield, the clicks on disabled buttons are queued
  setDefaultState(reportButton,true);
  setDefaultState(totalMapButton,true);
  setDefaultState(cavityMapButton, outputGrid->GetNumberRows() != 0);
  enableGuiElements(true);
}

// load input files to display radii list
void MainFrame::OnLoadFiles(wxCommandEvent& event){
  enableGuiElements(false);

  Ctrl::getInstance()->loadRadiusFile();
  Ctrl::getInstance()->loadAtomFile();

  Ctrl::getInstance()->newCalculation();

  // reset output
  clearOutputText();
  clearOutputGrid();
  toggleButtons(); // sets accessibility of buttons

  wxYield();
  enableGuiElements(true);
}

// browse for atom file
void MainFrame::OnAtomBrowse(wxCommandEvent& event){
  std::string filetype = "XYZ and PDB files (*.xyz;*.pdb)|*.xyz;*pdb";
  OnBrowse(event, filetype, filepathText);
  // if user selects a .pdb file, then the .pdf file options are unlocked
  toggleOptionsPdb();
  enableGuiElements(true);
}

// browse for radius file
void MainFrame::OnRadiusBrowse(wxCommandEvent& event){
  std::string filetype = "TXT files (*.txt)|*.txt";
  OnBrowse(event, filetype, radiuspathText);
}

// browse (can only be called by another method function)
void MainFrame::OnBrowse(wxCommandEvent& event, std::string& filetype, wxTextCtrl* textbox){
  wxFileDialog openFileDialog
    (this,
     _("Select File"),
     "",
     "",
     filetype,
     wxFD_OPEN|wxFD_FILE_MUST_EXIST,
     wxDefaultPosition,
     wxDefaultSize,
     "file browser"
    );

  // if user closes dialog
  if (openFileDialog.ShowModal() == wxID_CANCEL)
    return;

  // proceed loading the file chosen by the user;
  wxFileInputStream input_stream(openFileDialog.GetPath());
  // if filepath is invalid
  if (!input_stream.IsOk())
  {
    wxLogError("Cannot open file '%s'.", openFileDialog.GetPath());
    return;
  }

  textbox->SetLabel(openFileDialog.GetPath());
  // import files after file selection
  if (!getAtomFilepath().empty() && !getRadiusFilepath().empty()){
    OnLoadFiles(event);
  }
}

// toggle the probe 2 radius box depending on the probe mode
void MainFrame::ProbeModeChange(wxCommandEvent& event){
  probe2InputText->Enable(event.IsChecked());
  setDefaultState(probe2InputText, event.IsChecked());
}

// Functions to dynamically change the color of the atom list grid cells
void MainFrame::GridChange(wxGridEvent& event){
  int col = event.GetCol();
  int row = event.GetRow();
  wxString value = event.GetString();
  if (col == 0){
    if (value == "1"){ //
      atomListGrid->SetCellBackgroundColour(row,1,col_white);
      atomListGrid->SetCellBackgroundColour(row,2,col_white);
    }
    else {
      atomListGrid->SetCellBackgroundColour(row,1,col_grey_cell);
      atomListGrid->SetCellBackgroundColour(row,2,col_grey_cell);
    }
  }
  else if (col == 3){
    if (wcstod(value,NULL) == 0){
      atomListGrid->SetCellBackgroundColour(row,3,col_red_cell);
    }
    else {
      atomListGrid->SetCellBackgroundColour(row,3,col_cyan_cell);
    }
  }
  atomListGrid->ForceRefresh();
}

///////////////
// ON EXPORT //
///////////////

std::string MainFrame::OpenExportFileDialog(const std::string file_type, const std::string file_extension){
  // open file dialog
  wxFileDialog save_dialog(this, _("Export " + file_type + " as..."), "", "", file_extension, wxFD_SAVE);

  // if user closes dialog
  if (save_dialog.ShowModal() == wxID_CANCEL) {return "";}
  
  return save_dialog.GetPath().ToStdString();
}

void MainFrame::OnExportReport(wxCommandEvent& event){
  const std::string file = "report";
  // check whether report can be generated
  if (!Ctrl::getInstance()->isCalculationDone()){
    printToOutput("Data missing to generate " + file + "!"); 
    return;
  }
  std::string path = OpenExportFileDialog(file, "*.txt");
  if (path.empty()) {return;}
  
  // create report
  Ctrl::getInstance()->exportReport(path);
}

void MainFrame::OnExportTotalMap(wxCommandEvent& event){
  const std::string file = "total surface map";
  if (!Ctrl::getInstance()->isCalculationDone()){
    printToOutput("Data missing to generate " + file + "!"); 
    return;
  }
  
  std::string path = OpenExportFileDialog(file, "*.dx");
  if (path.empty()) {return;}
  
  Ctrl::getInstance()->exportSurfaceMap(path, false);
}

void MainFrame::OnExportCavityMap(wxCommandEvent& event){
  const std::string file = "cavity surface maps";
  if (!Ctrl::getInstance()->isCalculationDone() || outputGrid->GetNumberRows() == 0){
    printToOutput("Data missing to generate " + file + "!"); 
    return;
  }
  
  std::string path = OpenExportFileDialog(file, "*.dx");
  if (path.empty()) {return;}
  
  Ctrl::getInstance()->exportSurfaceMap(path, true);
}

////////////////////////////////
// GUI enabling and disabling //
////////////////////////////////

void MainFrame::enableGuiElements(bool inp){
  // disables all interactable widgets listed in InitDefaultStates() in base_init.cpp
  // enables all widgets whose default state has been set to true
  for (std::map<wxWindow*, bool>::iterator it = default_states.begin(); it != default_states.end(); it++){
    it->first->Enable( inp ? it->second : false );
  }
}

void MainFrame::setDefaultState(wxWindow* widget, bool state){
  default_states[widget] = state;
}

void MainFrame::toggleOptionsPdb(){
  setDefaultState(pdbHetatmCheckbox, fileExtension(getAtomFilepath()) == "pdb" );
  setDefaultState(unitCellCheckbox, fileExtension(getAtomFilepath()) == "pdb" );
  if(fileExtension(getAtomFilepath()) != "pdb"){
    unitCellCheckbox->SetValue(false);
  }
}

void MainFrame::toggleButtons(){
  setDefaultState(reportButton,false);
  setDefaultState(totalMapButton,false);
  setDefaultState(cavityMapButton,false);
  setDefaultState(loadFilesButton, true);
  setDefaultState(calcButton, atomListGrid->GetNumberRows() != 0);
}

