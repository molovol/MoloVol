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
void MainFrame::OnExit(wxCommandEvent& event){
  this->Close(TRUE);
}

// display text in output, used for debugging
void MainFrame::OnPrint(wxCommandEvent& event){
  std::string text = "treat yourself well";
  printToOutput(text);
}

// begin calculation
void MainFrame::OnCalc(wxCommandEvent& event){
  enableGuiElements(false);

  Ctrl::getInstance()->runCalculation();

  wxYield(); // is this necessary?
  // without wxYield, the clicks on disabled buttons are queued
  enableGuiElements(true);
}

// load input files to display radii list
void MainFrame::OnLoadFiles(wxCommandEvent& event){
  enableGuiElements(false);

  Ctrl::getInstance()->loadInputFiles();

  wxYield(); // is this necessary?
  // without wxYield, the clicks on disabled buttons are queued
  enableGuiElements(true);
}

void MainFrame::enableGuiElements(bool inp){
  // deactivate the Start and Load files buttons during the calculation
  calcButton->Enable(inp);
  loadFilesButton->Enable(inp);
  // prevents editing of the atom list grid during the calculation
  atomListGrid->EnableEditing(inp);
}

// browse for atom file
void MainFrame::OnAtomBrowse(wxCommandEvent& event){
  std::string filetype = "XYZ and PDB files (*.xyz;*.pdb)|*.xyz;*pdb";
  OnBrowse(filetype, filepathText);
}

// browse for radius file
void MainFrame::OnRadiusBrowse(wxCommandEvent& event){
  std::string filetype = "TXT files (*.txt)|*.txt";
  OnBrowse(filetype, radiuspathText);
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
  //load automatically
  Ctrl::getInstance()->loadInputFiles();
  wxYield(); // is this necessary?
  enableGuiElements(true);
}

// Functions to dynamically change the color of the atom list grid cells
// TODO: add events to set radius cell in red if radius = 0
// TODO: add events to set element and number cells in grey if radius element is excluded
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

