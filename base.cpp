#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#  include <wx/wx.h>
#endif
#include "base.h"

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
  browserSizer->Add(filenameText,5,wxALIGN_RIGHT | wxALL,10);
  // set sizer
  browsePanel->SetSizer(browserSizer);
}

void MainFrame::InitSandr(){
  wxBoxSizer *sandrSizer = new wxBoxSizer(wxHORIZONTAL);
  sandrSizer->Add(outputText,3,wxALIGN_LEFT | wxALL,10);
  sandrSizer->Add(calcButton,1,wxALIGN_RIGHT | wxALIGN_CENTRE_VERTICAL | wxALL,10);

  sandrPanel->SetSizer(sandrSizer);
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
     "browse filename"
    );

  filenameText = new wxTextCtrl
    (browsePanel,
     TEXT_Filename,
     wxEmptyString,
     wxDefaultPosition,
     wxDefaultSize,
     0,
     wxDefaultValidator,
     "file to use"
    );
  //filenameText->SetDefaultStyle(wxTextAttr(wxNullColour, *wxWHITE));
  filenameText->SetBackgroundColour(wxColour(255,255,255));

  InitBrowsePanel();

/*  
  browseFile = new wxFileCtrl
    (browsePanel,
     FILE_Browse,
     wxEmptyString,
     wxEmptyString,
     wxFileSelectorDefaultWildcardStr,
     wxFC_DEFAULT_STYLE,
     wxDefaultPosition,
     wxDefaultSize,
     wxFileCtrlNameStr
    );

  InitFileBrowser();
*/
  


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

  outputText = new wxStaticText
    (sandrPanel,
     TEXT_Output,
     "Output",
     wxDefaultPosition,
     wxSize(250,60),
     0,
     "output result"
    );
  outputText->SetBackgroundColour(wxColour(255,255,255));
  
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
  printToOutput(this,text);
  return;
}

void MainFrame::OnCalc(wxCommandEvent& event){
  int a = 1;
  int b = 2;
  int c = calcSum(a, b);
  std::string c_str = std::to_string(c); 
  printToOutput(this,c_str);
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
  // if filename is invalid
  if (!input_stream.IsOk())
  {
    wxLogError("Cannot open file '%s'.", openFileDialog.GetPath());
    return;
  }
  filenameText->SetLabel(openFileDialog.GetPath());
  return;
}
