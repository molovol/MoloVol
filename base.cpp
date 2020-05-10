#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#  include <wx/wx.h>
#endif

#include "base.h"
#include "controller.h"

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
