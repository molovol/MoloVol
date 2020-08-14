#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#  include <wx/wx.h>
#endif

#include "base.h"
#include "controller.h"
#include <vector>

/////////////////
// EVENT TABLE //
/////////////////

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
  EVT_BUTTON(BUTTON_Calc, MainFrame::OnCalc)
  EVT_BUTTON(BUTTON_Browse, MainFrame::OnAtomBrowse)
  EVT_BUTTON(BUTTON_Radius, MainFrame::OnRadiusBrowse)
  EVT_BUTTON(BUTTON_LoadFiles, MainFrame::OnLoadFiles)
  EVT_GRID_CELL_CHANGING(MainFrame::GridChange)
END_EVENT_TABLE()

////////////////////////////////
// METHODS FOR EVENT HANDLING //
////////////////////////////////

// exit program
void MainFrame::OnExit(wxCommandEvent& event)
{
  this->Close(TRUE);
}

// display text in output, used for debugging
void MainFrame::OnPrint(wxCommandEvent& event)
{
  std::string text = "treat yourself well";
  printToOutput(text);
  return;
}

// begin calculation
void MainFrame::OnCalc(wxCommandEvent& event){

  Ctrl::getInstance()->runCalculation();
  
  return;
}

void MainFrame::enableGuiElements(bool inp){
  // deactivate the Start and Load files buttons during the calculation
  calcButton->Enable(inp);
  loadFilesButton->Enable(inp);
  // prevents editing of the atom list grid during the calculation
  atomListGrid->EnableEditing(inp);
  return;
}

// browse for atom file
void MainFrame::OnAtomBrowse(wxCommandEvent& event){
  std::string filetype = "XYZ files (*.xyz)|*.xyz";
  OnBrowse(filetype, filepathText);
  return;
}

// browse for radius file
void MainFrame::OnRadiusBrowse(wxCommandEvent& event){
  std::string filetype = "TXT files (*.txt)|*.txt";
  OnBrowse(filetype, radiuspathText);
  return;
}

// load input files to display radii list
void MainFrame::OnLoadFiles(wxCommandEvent& event){

  // Deactivate the Start and Load files buttons during the loading
  enableGuiElements(false);
// calcButton->Enable(false);
// loadFilesButton->Enable(false);
  // so far only xyz files allowed
  std::vector<std::tuple<std::string, int, double>> atoms_for_list = Ctrl::getInstance()->loadInputFiles();
  MainFrame::generateAtomList(atoms_for_list);
  // TODO: without wxYield, the button is grayed but still records clicks
  // yet, wxYield is apparently dangerous in an event handler, need to find an alternative
  wxYield();
  // Reactivate the Start and Load files buttons
  MainFrame::calcButton->Enable(true);
  MainFrame::loadFilesButton->Enable(true);
  return;
}

// browse (can only be called by another method function)
void MainFrame::OnBrowse(std::string& filetype, wxTextCtrl* textbox){
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

  // if user closes dialogue
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
  return;
}

// Functions to dynamically change the color of the atom list grid cells
void MainFrame::GridChange(wxGridEvent& event){
  int col = event.GetCol();
  int row = event.GetRow();
  wxString value = atomListGrid->GetCellValue(row,col);
/*  if (col == 0){
    atomListGrid->SetReadOnly(row,col,true);
    if (value == "1"){
      atomListGrid->SetCellBackgroundColour(row,1,col_white);
      atomListGrid->SetCellBackgroundColour(row,2,col_white);
    }
    else {
      atomListGrid->SetCellBackgroundColour(row,1,col_grey_cell);
      atomListGrid->SetCellBackgroundColour(row,2,col_grey_cell);
    }
  }
  else */if (col == 3){
 //   if (wcstod(value,NULL) == 0){
 //     atomListGrid->SetCellBackgroundColour(row,3,col_red_cell);
 //   }
 //   else {
      atomListGrid->SetCellBackgroundColour(row,3,col_cyan_cell);
 //   }
  }
  atomListGrid->Refresh();
}

