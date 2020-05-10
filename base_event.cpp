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
END_EVENT_TABLE()

////////////////////////////////
// METHODS FOR EVENT HANDLING //
////////////////////////////////

// exit program
void MainFrame::OnExit(wxCommandEvent& event)
{
  this->Close(TRUE);
}

// display text in output
void MainFrame::OnPrint(wxCommandEvent& event)
{
  std::string text = "fuck you";
  printToOutput(text);
  return;
}

// begin calculation
void MainFrame::OnCalc(wxCommandEvent& event){
      
  std::string filepath = (filepathText->GetValue()).ToStdString();
  printToOutput(filepath);
  
  // so far only xyz files allowed
  Ctrl::getInstance()->runCalculation(filepath);
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
