#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#  include <wx/wx.h>
#endif

#include "base.h"
#include "controller.h"
#include <vector>

IMPLEMENT_APP(MainApp)

///////////////////////////
// MAIN APP IS INITIATED //
///////////////////////////

bool MainApp::OnInit()
{
  // initialise a new MainFrame object and have MainWin point to that object
  MainFrame* MainWin = new MainFrame(_("Hello World!"), wxDefaultPosition, wxDefaultSize);
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
  EVT_BUTTON(BUTTON_Browse, MainFrame::OnAtomBrowse)
  EVT_BUTTON(BUTTON_Radius, MainFrame::OnRadiusBrowse)
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
  wxBoxSizer *browserSizer = new wxBoxSizer(wxVERTICAL);
  // widgets to sizer
  browserSizer->Add(atomfilePanel,0,0,20);
  browserSizer->Add(radiusfilePanel,0,0,20);
  // set sizer
  browsePanel->SetSizer(browserSizer);
}

void MainFrame::InitSandr(){
  wxBoxSizer *sandrSizer = new wxBoxSizer(wxHORIZONTAL);
  sandrSizer->Add(outputText,5,wxALIGN_LEFT | wxALL,10);
  sandrSizer->Add(calcButton,1,wxALIGN_RIGHT | wxALIGN_CENTRE_VERTICAL | wxALL,10);

  sandrPanel->SetSizer(sandrSizer);
}

void MainFrame::InitFilePanel(wxPanel* panel, wxButton* button, wxTextCtrl* text){
  // create new sizer
  wxBoxSizer *fileSizer = new wxBoxSizer(wxHORIZONTAL);
  // widgets to sizer
  fileSizer->Add(button,1,wxALIGN_LEFT | wxALIGN_CENTRE_VERTICAL | wxALL,10);
  fileSizer->Add(text,5,wxALIGN_RIGHT | wxALL,10);
  // set sizer
  panel->SetSizer(fileSizer);
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
  Ctrl::getInstance()->registerView(this);
  
  /////////////////////
  // TOP LEVEL FRAME //
  /////////////////////
  
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

  //////////////////
  // BROWSE PANEL //
  //////////////////
 
  atomfilePanel = new wxPanel(browsePanel, PANEL_Atomfile, wxDefaultPosition, wxDefaultSize, 0);
  radiusfilePanel = new wxPanel(browsePanel, PANEL_Radiusfile, wxDefaultPosition, wxDefaultSize, 0);
  
  InitBrowsePanel();
  
  /////////////////////////
  // ATOM FILE SELECTION //
  /////////////////////////
  
  browseButton = new wxButton
    (atomfilePanel, BUTTON_Browse, "Browse", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);

  filepathText = new wxTextCtrl
    (atomfilePanel, TEXT_Filename, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
  filepathText->SetBackgroundColour(wxColour(255,255,255));

  InitFilePanel(atomfilePanel, browseButton, filepathText);

  ///////////////////////////
  // RADIUS FILE SELECTION //
  ///////////////////////////
  
  radiusButton = new wxButton
    (radiusfilePanel, BUTTON_Radius, "Browse");

  radiuspathText = new wxTextCtrl
    (radiusfilePanel, TEXT_Radius, "./inputfile/radii.txt");
  radiuspathText->SetBackgroundColour(wxColour(255,255,255));
  
  InitFilePanel(radiusfilePanel, radiusButton, radiuspathText);

  ////////////////////////////
  // SEND AND RECEIVE PANEL //
  ////////////////////////////
  
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
    printToOutput(filepath);
    
    // so far only xyz files allowed
    Ctrl::getInstance()->runCalculation(filepath);
//  std::vector<Atom> atoms = readAtomsFromFile(filepath);
  /*
  clearOutput();
  for (int i = 0; i< atoms.size(); i++){
    std::string out = "Atom " + std::to_string(i) + ": " + atoms[i].symbol + "\n";
    appendOutput(out);
  }
*/
  return;
}

void MainFrame::OnAtomBrowse(wxCommandEvent& event){
  std::string filetype = "XYZ files (*.xyz)|*.xyz";
  OnBrowse(filetype, filepathText);
  return;
}

void MainFrame::OnRadiusBrowse(wxCommandEvent& event){
  std::string filetype = "TXT files (*.txt)|*.txt";
  OnBrowse(filetype, radiuspathText);
  return;
}

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
