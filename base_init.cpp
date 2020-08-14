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
  MainWin->SetBackgroundColour(col_win);
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
  leftMainPanel = new wxPanel
    (this,
     PANEL_LeftMain,
     wxDefaultPosition,
     wxDefaultSize,
     wxTAB_TRAVERSAL,
     "contains file browser and radii list"
    );
  leftMainPanel->SetBackgroundColour(col_panel);

  rightMainPanel = new wxPanel
    (this,
     PANEL_RightMain,
     wxDefaultPosition,
     wxDefaultSize,
     wxTAB_TRAVERSAL,
     "contains input parameters and output"
    );
  rightMainPanel->SetBackgroundColour(col_panel);

  // browse Panel

  browsePanel = new wxPanel
    (leftMainPanel,
     PANEL_Browse,
     wxDefaultPosition,
     wxDefaultSize,
     wxTAB_TRAVERSAL,
     "contains file browser"
    );
  browsePanel->SetBackgroundColour(col_panel);

  // radii list Panel

  atomListPanel = new wxPanel
    (leftMainPanel,
     PANEL_AtomList,
     wxDefaultPosition,
     wxDefaultSize,
     wxTAB_TRAVERSAL);
  atomListPanel->SetBackgroundColour(col_panel);

  // parameter Panel

  parameterPanel = new wxPanel
    (rightMainPanel,
     PANEL_Parameters,
     wxDefaultPosition,
     wxDefaultSize,
     wxTAB_TRAVERSAL);
  parameterPanel->SetBackgroundColour(col_panel);

  // construct send and receive panel

  sandrPanel = new wxPanel
    (rightMainPanel,
     PANEL_Sandr,
     wxDefaultPosition,
     wxDefaultSize,
     wxTAB_TRAVERSAL,
     "initiate user-program communication"
    );
  sandrPanel->SetBackgroundColour(col_panel);

}

//////////////////
// BROWSE PANEL //
//////////////////
void MainFrame::InitBrowsePanel(){

  atomfilePanel = new wxPanel(browsePanel, PANEL_Atomfile, wxDefaultPosition, wxDefaultSize, 0);
  radiusfilePanel = new wxPanel(browsePanel, PANEL_Radiusfile, wxDefaultPosition, wxDefaultSize, 0);
  fileOptionsPanel = new wxPanel(browsePanel, PANEL_FileOptions, wxDefaultPosition, wxDefaultSize, 0);

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
     wxSize(-1,100), // height of the output text control
     wxTE_MULTILINE | wxTE_READONLY,
     wxDefaultValidator,
     "output result"
    );
  outputText->SetBackgroundColour(col_output);

}

void MainFrame::InitFilePanel(wxPanel* panel, wxButton* button, wxTextCtrl* text){
  // create new sizer
  wxBoxSizer *fileSizer = new wxBoxSizer(wxHORIZONTAL);
  // widgets to sizer
  fileSizer->Add(button,1,wxALIGN_LEFT | wxALIGN_CENTRE_VERTICAL | wxALL,10);
  fileSizer->Add(text,5,wxALIGN_RIGHT | wxALL,10);
  // set sizer
  panel->SetSizerAndFit(fileSizer);
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

/////////////////////////////////////
// FILE SELECTION OPTIONS AND LOAD //
/////////////////////////////////////

void MainFrame::InitFileOptionsPanel(){
  loadFilesButton = new wxButton
    (fileOptionsPanel,
     BUTTON_LoadFiles,
     "Load files",
     wxDefaultPosition,
     wxDefaultSize,
     0,
     wxDefaultValidator,
     "load input files"
    );
}

//////////////////////
// PARAMETERS PANEL //
//////////////////////

void MainFrame::InitParametersPanel(){
  // init gridsize and depth panels
  gridsizePanel = new wxPanel(parameterPanel, PANEL_Grid, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  gridsizePanel->SetBackgroundColour(col_panel);

  depthPanel = new wxPanel(parameterPanel, PANEL_Depth, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  depthPanel->SetBackgroundColour(col_panel);

  return;
}

void MainFrame::InitGridPanel(){

  // static text
  gridsizeText = new wxStaticText(gridsizePanel, TEXT_Grid, "Grid step size:");

  // panel
  gridsizeInputPanel = new wxPanel(gridsizePanel, PANEL_Depth, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  gridsizeInputPanel->SetBackgroundColour(col_panel);

  return;
}

void MainFrame::InitGridinputPanel(){

  gridsizeInputText = new wxTextCtrl(gridsizeInputPanel, TEXT_Gridinput, "0.1");
  gridsizeInputText->SetBackgroundColour(col_white);

  gridsizeUnitText = new wxStaticText(gridsizeInputPanel, TEXT_Gridunit, L"\u212B "); // unicode for angstrom

  return;
}

void MainFrame::InitDepthPanel(){

  depthText = new wxStaticText(depthPanel, TEXT_Depth, "Maximum tree depth:");

  depthInput = new wxSpinCtrl
    (depthPanel, SPIN_Depthinput, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 20, 4);
  depthInput->SetBackgroundColour(col_white);

  return;
}

/////////////////////
// ATOM LIST PANEL //
/////////////////////

void MainFrame::InitAtomListPanel(){
  // init gridsize and depth panels
  atomListGrid = new wxGrid(atomListPanel, GRID_AtomList, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS, "Atom list");
  atomListGrid->CreateGrid(0, 4, wxGrid::wxGridSelectCells);
  atomListGrid->SetDefaultCellAlignment (wxALIGN_CENTRE, wxALIGN_CENTRE);
  atomListGrid->SetColLabelValue(0, "Include?");
  atomListGrid->SetColLabelValue(1, "Element");
  atomListGrid->SetColLabelValue(2, "Number \nof Atoms");
  atomListGrid->SetColLabelValue(3, L"Radius (\u212B)");
  atomListGrid->SetColFormatBool(0);
  atomListGrid->SetColFormatNumber(2);
  atomListGrid->SetColFormatFloat(3, 5, 3); // 2nd argument is width, last argument is precision
  return;
}

/////////////////////////
// FIT SIZES OF PANELS //
/////////////////////////

void MainFrame::FitSizes(){
// Sizers need to be set in order from most grand-children to parent

// Children^4
//////////////

// gridsizeInputPanel
  wxBoxSizer *gridsizeInputsizer = new wxBoxSizer(wxHORIZONTAL);
  // widgets to sizer
  gridsizeInputsizer->Add(gridsizeInputText, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 10);
  gridsizeInputsizer->Add(gridsizeUnitText, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 10);
  // set sizer
  gridsizeInputPanel->SetSizerAndFit(gridsizeInputsizer);


// Children^3
//////////////

// fileOptionPanel
  wxBoxSizer *fileOptionsSizer = new wxBoxSizer(wxHORIZONTAL);
  // widgets to sizer
  fileOptionsSizer->Add(loadFilesButton,1,wxALIGN_LEFT | wxALIGN_CENTRE_VERTICAL | wxALL,10);
  // set sizer
  fileOptionsPanel->SetSizerAndFit(fileOptionsSizer);

// gridsizePanel
  wxBoxSizer *gridsizeSizer = new wxBoxSizer(wxHORIZONTAL);
  // widgets to sizer
  gridsizeSizer->Add(gridsizeText,0,wxALL | wxALIGN_CENTRE_VERTICAL,10);
  gridsizeSizer->Add(gridsizeInputPanel,0,0,0);
  // set sizer
  gridsizePanel->SetSizerAndFit(gridsizeSizer);

// depthPanel
  wxBoxSizer *depthSizer = new wxBoxSizer(wxHORIZONTAL);
  // widgets to sizer
  depthSizer->Add(depthText,0,wxALIGN_CENTRE_VERTICAL | wxALL,10);
  depthSizer->Add(depthInput,0,wxALIGN_CENTRE_VERTICAL | wxALL,10);
  // set sizer
  depthPanel->SetSizerAndFit(depthSizer);


// Children^2
//////////////

// browserPanel
  wxStaticBoxSizer *browserSizer = new wxStaticBoxSizer(wxVERTICAL,browsePanel);
  // widgets to sizer
  browserSizer->Add(radiusfilePanel,0,wxEXPAND,0);
  browserSizer->Add(atomfilePanel,0,wxEXPAND,0);
  browserSizer->Add(fileOptionsPanel,0,wxEXPAND,0);
  // set sizer
  browsePanel->SetSizerAndFit(browserSizer);

// atomListPanel
  wxStaticBoxSizer *atomListSizer = new wxStaticBoxSizer(wxVERTICAL,atomListPanel);
  // widgets to sizer
  atomListSizer->Add(atomListGrid,0,wxEXPAND,20);
  // set sizer
  atomListPanel->SetSizerAndFit(atomListSizer);

// parameterPanel
  wxStaticBoxSizer *parameterSizer = new wxStaticBoxSizer(wxVERTICAL,parameterPanel);
  // widgets to sizer
  parameterSizer->Add(gridsizePanel,0,wxEXPAND,20);
  parameterSizer->Add(depthPanel,0,wxEXPAND,20);
  // set sizer
  parameterPanel->SetSizerAndFit(parameterSizer);

// sandrPanem
  wxStaticBoxSizer *sandrSizer = new wxStaticBoxSizer(wxHORIZONTAL,sandrPanel);
  // widgets to sizer
  sandrSizer->Add(outputText,5,wxALIGN_LEFT | wxALL,10);
  sandrSizer->Add(calcButton,1,wxALIGN_RIGHT | wxALIGN_CENTRE_VERTICAL | wxALL,10);
  // set sizer
  sandrPanel->SetSizerAndFit(sandrSizer);


// Children
//////////////

// leftMainPanel
  wxBoxSizer *leftSizerV = new wxBoxSizer(wxVERTICAL);
  // widgets to sizer
  leftSizerV->Add(browsePanel,0,wxEXPAND,20);
  //
  leftSizerV->Add(atomListPanel,0,wxEXPAND,20);
  // set sizer
  leftMainPanel->SetSizerAndFit(leftSizerV);

// rightMainPanel
  wxBoxSizer *rightSizerV = new wxBoxSizer(wxVERTICAL);
  // widgets to sizer
  rightSizerV->Add(parameterPanel,0,wxEXPAND,20);
  //
  rightSizerV->Add(sandrPanel,0,wxEXPAND,20);
  // set sizer
  rightMainPanel->SetSizerAndFit(rightSizerV);


// Parent
//////////////

// Main Panel
  wxBoxSizer *topLevelSizerH = new wxBoxSizer(wxHORIZONTAL);
  // widgets to sizer
  topLevelSizerH->Add(leftMainPanel,0,wxRIGHT | wxEXPAND,5);
  //
  topLevelSizerH->Add(rightMainPanel,0,wxLEFT | wxEXPAND,5);
  // set sizer
  this->SetSizerAndFit(topLevelSizerH); // Not sure how the "this" works here

  return;
}

//
