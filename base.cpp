#include <wx/wxprec.h>
#include <vector>

#ifndef WX_PRECOMP
#  include <wx/wx.h>
#endif

#include "base.h"
#include "test.h"
#include "filereading.h"

IMPLEMENT_APP(MainApp)

///////////////////////////
// MAIN APP IS INITIATED //
///////////////////////////

bool MainApp::OnInit()
{
  // initialise a new MainFrame object and have MainWin point to that object
  MainFrame *MainWin = new MainFrame(_("Hello World!"), wxDefaultPosition, wxDefaultSize);
  // call member function of the MainFrame object to set visibility
  MainWin->Show(true);
  SetTopWindow(MainWin);
  return true;
};

/////////////////
// EVENT TABLE //
/////////////////

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
  // assign a function to button Hello
  EVT_BUTTON(BUTTON_Calc, MainFrame::OnCalc)
  EVT_BUTTON(BUTTON_Browse, MainFrame::OnBrowse)
END_EVENT_TABLE()

////////////////////////////////////
// INITIALISATION OF GUI ELEMENTS //
////////////////////////////////////

void MainFrame::InitTopLevel()
{
  // create new sizer
  wxBoxSizer *topLevelSizer = new wxBoxSizer(wxVERTICAL);
  // widgets to sizer
  topLevelSizer->Add(browsePanel,0,wxEXPAND,20);
  topLevelSizer->Add(sandrPanel,0,wxEXPAND,20);
  // set sizer
  this->SetSizer(topLevelSizer);
}

void MainFrame::InitBrowsePanel(){
  // create new sizer
  wxBoxSizer *browserSizer = new wxBoxSizer(wxHORIZONTAL);
  // widgets to sizer
  browserSizer->Add(browseButton,1,wxALIGN_LEFT | wxALIGN_CENTRE_VERTICAL | wxALL,10);
  browserSizer->Add(filepathText,5,wxALIGN_RIGHT | wxALL,10);
  // set sizer
  browsePanel->SetSizer(browserSizer);
}

void MainFrame::InitSandr(){
  wxBoxSizer *sandrSizer = new wxBoxSizer(wxHORIZONTAL);
  sandrSizer->Add(outputText,5,wxALIGN_LEFT | wxALL,10);
  sandrSizer->Add(calcButton,1,wxALIGN_RIGHT | wxALIGN_CENTRE_VERTICAL | wxALL,10);

  sandrPanel->SetSizer(sandrSizer);
}

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


////////////////////////////
// MAIN FRAME CONSTRUCTOR //
////////////////////////////

MainFrame::MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size)
  : wxFrame((wxFrame*) NULL, -1, title, pos, size)
{
  // Panel in Frame
  // construct file browser panel
  browsePanel = new wxPanel
    (this,
     PANEL_Browse,
     wxDefaultPosition,
     wxDefaultSize,
     wxTAB_TRAVERSAL,
     "contains file browser"
    );
  browsePanel->SetBackgroundColour(wxColour(192,192,192));  

  // construct send and receive panel
  sandrPanel = new wxPanel
    (this,
     PANEL_Sandr,
     wxDefaultPosition,
     wxDefaultSize,
     wxTAB_TRAVERSAL,
     "initiate user-program communication"
    );
  sandrPanel->SetBackgroundColour(wxColour(192,192,192));  
 
  InitTopLevel();
 
  // button and text control in Panel 
  browseButton = new wxButton
    (browsePanel, 
     BUTTON_Browse,
     "Browse", 
     wxDefaultPosition, 
     wxDefaultSize, 
     0,
     wxDefaultValidator,
     "browse filepath"
    );

  filepathText = new wxTextCtrl
    (browsePanel,
     TEXT_Filename,
     wxEmptyString,
     wxDefaultPosition,
     wxDefaultSize,
     0,
     wxDefaultValidator,
     "file to use"
    );
  //filepathText->SetDefaultStyle(wxTextAttr(wxNullColour, *wxWHITE));
  filepathText->SetBackgroundColour(wxColour(255,255,255));

  InitBrowsePanel();

  // Text and Button in Panel 
  calcButton = new wxButton
    (sandrPanel, 
     BUTTON_Calc,
     "Start", 
     wxDefaultPosition, 
     wxDefaultSize, 
     0,
     wxDefaultValidator,
     "begin calculation"
    );

  outputText = new wxTextCtrl
    (sandrPanel,
     TEXT_Output,
     _("Output"),
     wxDefaultPosition,
     wxSize(-1,90), // height of the output text control
     wxTE_MULTILINE | wxTE_READONLY,
     wxDefaultValidator,
     "output result"
    );
  outputText->SetBackgroundColour(wxColour(125,125,125));
  
  InitSandr();  


};

////////////////////////////////
// METHODS FOR EVENT HANDLING //
////////////////////////////////

void MainFrame::OnExit(wxCommandEvent& event)
{
  this->Close(TRUE);
}

void MainFrame::OnPrint(wxCommandEvent& event)
{
  std::string text = "fuck you";
  printToOutput(text);
  return;
}

void MainFrame::OnCalc(wxCommandEvent& event){
  std::string filepath = (filepathText->GetValue()).ToStdString();
  outputText->SetLabel(filepath);
  printToOutput(filepath);
  
  // so far only xyz files allowed
  std::vector<Atom> atoms = readAtomsFromFile(filepath);
  
  clearOutput();
  for (int i = 0; i< atoms.size(); i++){
    std::string out = "Atom " + std::to_string(i) + ": " + atoms[i].symbol + "\n";
    appendOutput(out);
  }

  return;
}

void MainFrame::OnBrowse(wxCommandEvent& event){
  wxFileDialog openFileDialog
    (this, 
     _("Select File"), 
     "", 
     "",
     "XYZ files (*.xyz)|*.xyz", 
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
  filepathText->SetLabel(openFileDialog.GetPath());
  return;
}
