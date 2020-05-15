#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#  include <wx/wx.h>
#endif

#include "base.h"

IMPLEMENT_APP(MainApp)

/////////////////////////////
// MAIN APP IS INITIALISED //
/////////////////////////////

bool MainApp::OnInit()
{
  // initialise a new MainFrame object and have MainWin point to that object
  MainFrame* MainWin = new MainFrame(_("CaVol"), wxDefaultPosition, wxDefaultSize);
  // call member function of the MainFrame object to set visibility
  MainWin->Show(true);
  SetTopWindow(MainWin);
  return true;
};

////////////////////////////////////
// INITIALISATION OF GUI ELEMENTS //
////////////////////////////////////

/////////////////////
// TOP LEVEL FRAME //
/////////////////////
void MainFrame::InitTopLevel(){
  
  browsePanel = new wxPanel
    (this,
     PANEL_Browse,
     wxDefaultPosition,
     wxDefaultSize,
     wxTAB_TRAVERSAL,
     "contains file browser"
    );
  browsePanel->SetBackgroundColour(col_panel);  
  
  // parameter Panel
  //
  
  parameterPanel = new wxPanel(this, PANEL_Parameters, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  parameterPanel->SetBackgroundColour(col_panel); 

  //


  // construct send and receive panel
  sandrPanel = new wxPanel
    (this,
     PANEL_Sandr,
     wxDefaultPosition,
     wxDefaultSize,
     wxTAB_TRAVERSAL,
     "initiate user-program communication"
    );
  sandrPanel->SetBackgroundColour(col_panel); 

  // create new sizer
  wxBoxSizer *topLevelSizer = new wxBoxSizer(wxVERTICAL);
  // widgets to sizer
  topLevelSizer->Add(browsePanel,0,wxEXPAND,20);
  //
  topLevelSizer->Add(parameterPanel,0,wxEXPAND,20);
  //
  topLevelSizer->Add(sandrPanel,0,wxEXPAND,20);
  // set sizer
  this->SetSizer(topLevelSizer);
}

//////////////////
// BROWSE PANEL //
//////////////////
void MainFrame::InitBrowsePanel(){
 
  atomfilePanel = new wxPanel(browsePanel, PANEL_Atomfile, wxDefaultPosition, wxDefaultSize, 0);
  radiusfilePanel = new wxPanel(browsePanel, PANEL_Radiusfile, wxDefaultPosition, wxDefaultSize, 0);

  // create new sizer
  wxStaticBoxSizer *browserSizer = new wxStaticBoxSizer(wxVERTICAL,browsePanel);
  // widgets to sizer
  browserSizer->Add(atomfilePanel,0,wxEXPAND,0);
  browserSizer->Add(radiusfilePanel,0,wxEXPAND,0);
  // set sizer
  browsePanel->SetSizer(browserSizer);
}

////////////////////////////
// SEND AND RECEIVE PANEL //
////////////////////////////
void MainFrame::InitSandr(){
  
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
  outputText->SetBackgroundColour(col_output);

  wxStaticBoxSizer *sandrSizer = new wxStaticBoxSizer(wxHORIZONTAL,sandrPanel);
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

/////////////////////////
// ATOM FILE SELECTION //
/////////////////////////

void MainFrame::InitAtomfilePanel(){  
  browseButton = new wxButton
    (atomfilePanel, BUTTON_Browse, "Browse", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);

  filepathText = new wxTextCtrl
    (atomfilePanel, TEXT_Filename, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
  filepathText->SetBackgroundColour(col_white);

  InitFilePanel(atomfilePanel, browseButton, filepathText);
}

///////////////////////////
// RADIUS FILE SELECTION //
///////////////////////////

void MainFrame::InitRadiusfilePanel(){  
  radiusButton = new wxButton
    (radiusfilePanel, BUTTON_Radius, "Browse");

  radiuspathText = new wxTextCtrl
    (radiusfilePanel, TEXT_Radius, "./inputfile/radii.txt");
  radiuspathText->SetBackgroundColour(col_white);
  
  InitFilePanel(radiusfilePanel, radiusButton, radiuspathText);
  
}

//////////////////////
// PARAMETERS PANEL //
//////////////////////

void MainFrame::InitParametersPanel(){
  // init gridsize and depth panels
  gridsizePanel = new wxPanel(parameterPanel, PANEL_Grid, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  gridsizePanel->SetBackgroundColour(col_panel); 
  
  depthPanel = new wxPanel(parameterPanel, PANEL_Depth, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
//  depthPanel->SetBackgroundColour(col_panel); 
  
  // create new sizer
  wxStaticBoxSizer *sizer = new wxStaticBoxSizer(wxVERTICAL,parameterPanel);
  // widgets to sizer
  sizer->Add(gridsizePanel,0,wxEXPAND,20);
  sizer->Add(depthPanel,0,wxEXPAND,20);
  // set sizer
  parameterPanel->SetSizer(sizer);

  return;
}

void MainFrame::InitGridPanel(){
  
  // static text
  
  gridsizeText = new wxStaticText(gridsizePanel, TEXT_Grid, "Grid step size:");	

  // panel
  gridsizeInputPanel = new wxPanel(gridsizePanel, PANEL_Depth, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
//  gridsizeInputPanel->SetBackgroundColour(col_panel);

  // create sizer
  wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
  // widgets to sizer
  sizer->Add(gridsizeText,0,wxALL | wxALIGN_CENTRE_VERTICAL,10);
  sizer->Add(gridsizeInputPanel,0,0,0);
  // set sizer
  gridsizePanel->SetSizer(sizer);

  return;
}

void MainFrame::InitGridinputPanel(){
  
  gridsizeInputText = new wxTextCtrl(gridsizeInputPanel, TEXT_Gridinput, "0.1");
  gridsizeInputText->SetBackgroundColour(col_white);
  
  gridsizeUnitText = new wxStaticText(gridsizeInputPanel, TEXT_Gridunit, "A");	
  
  // create sizer
  wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
  // widgets to sizer
  sizer->Add(gridsizeInputText, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 10);
  sizer->Add(gridsizeUnitText, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 10);
  // set sizer
  gridsizeInputPanel->SetSizer(sizer);
  return;
}

void MainFrame::InitDepthPanel(){
  
  depthText = new wxStaticText(depthPanel, TEXT_Depth, "Maximum tree depth:");

  depthInput = new wxSpinCtrl
    (depthPanel, SPIN_Depthinput, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 20, 4);	
  depthInput->SetBackgroundColour(col_white);

  // create sizer
  wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
  // widgets to sizer
  sizer->Add(depthText,0,wxALIGN_CENTRE_VERTICAL | wxALL,10);
  sizer->Add(depthInput,0,wxALIGN_CENTRE_VERTICAL | wxALL,10);
  // set sizer
  depthPanel->SetSizer(sizer);
  
  return;
}
//
