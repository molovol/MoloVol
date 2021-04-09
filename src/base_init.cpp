#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#  include <wx/wx.h>
#endif

#include "base.h"
#include "controller.h"
#include <cassert>

// wxWidgets macro that contains the entry point, initialised the app, and calls wxApp::OnInit()
IMPLEMENT_APP(MainApp)

/////////////////////////////
// MAIN APP IS INITIALISED //
/////////////////////////////

// contains all command line options
static const wxCmdLineEntryDesc gCmdLineDesc[] =
{
  { wxCMD_LINE_SWITCH, "s", "silent", "Silence GUI", wxCMD_LINE_VAL_NONE, 0},
  { wxCMD_LINE_OPTION, "u", "unittest", "Run a unittest", wxCMD_LINE_VAL_STRING},
  { wxCMD_LINE_NONE }
};

// first custom code that is run
bool MainApp::OnInit()
{
  wxCmdLineParser parser = wxCmdLineParser(argc,argv);
  parser.SetDesc(gCmdLineDesc);
  assert(parser.Parse()==0);
  wxString unittest_id;
  if (parser.Found("u",&unittest_id)){
    silenceGUI(true); // not really needed
    std::cout << "Selected unit test: " << unittest_id << std::endl;
    if (unittest_id=="excluded"){
      Ctrl::getInstance()->unittestExcluded();
    }
    else if (unittest_id=="protein"){
      Ctrl::getInstance()->unittestProtein();
    }
    else if (unittest_id=="radius"){
      Ctrl::getInstance()->unittestRadius();
    }
    else {
      std::cout << "Invalid selection" << std::endl;}
    return true;
  }
  // initialise the GUI
  MainFrame* MainWin = new MainFrame(_("MoloVol"), wxDefaultPosition, wxDefaultSize);
  MainWin->SetBackgroundColour(col_win);
  MainWin->Show(true);
  SetTopWindow(MainWin);
  return true;
};

// OnRun() is called after OnInit() returns true. In order to suppress the GUI, the attribute "silent" has to
// be toggled. this can be done through the command line
int MainApp::OnRun(){
  if (isSilent()){return 0;} // end application if GUI is silenced
  else {return wxApp::OnRun();} // proceed normally
}

// set default states of GUI elements here
void MainFrame::InitDefaultStates(){
  // set default accessibility of interactable gui controls
  wxWindow* widgets_enabled[] = {
    browseButton,
    radiusButton,
    filepathText,
    radiuspathText,
    atomListGrid,
    surfaceAreaCheckbox,
    twoProbesCheckbox,
    probe1InputText,
    gridsizeInputText,
    depthInput,
    reportCheckbox,
    surfaceMapCheckbox,
    cavityMapsCheckbox};
  wxWindow* widgets_disabled[] = {
    pdbHetatmCheckbox,
    loadFilesButton,
    unitCellCheckbox,
    probe2InputText,
    calcButton};

  // initialise map
  for (auto i : widgets_enabled){
    default_states[i] = true;
  }
  for (auto i : widgets_disabled){
    default_states[i] = false;
  }
}

void MainApp::silenceGUI(bool set){_silent = set;}
bool MainApp::isSilent(){return _silent;}

////////////////////////////////////
// INITIALISATION OF GUI ELEMENTS //
////////////////////////////////////

/////////////////////
// TOP LEVEL FRAME //
/////////////////////
void MainFrame::InitTopLevel(){
  // contains browse panel and atom list panel
  leftMainPanel = new wxPanel
    (this,
     PANEL_LeftMain,
     wxDefaultPosition,
     wxDefaultSize,
     wxTAB_TRAVERSAL
    );
  leftMainPanel->SetBackgroundColour(col_panel);

  // contains parameter panel and send-and-receive panel
  rightMainPanel = new wxPanel
    (this,
     PANEL_RightMain,
     wxDefaultPosition,
     wxDefaultSize,
     wxTAB_TRAVERSAL
    );
  rightMainPanel->SetBackgroundColour(col_panel);

  InitLeftMainPanel();
  InitRightMainPanel();

  wxBoxSizer *topLevelSizerH = new wxBoxSizer(wxHORIZONTAL);
  topLevelSizerH->Add(leftMainPanel,1,wxRIGHT | wxEXPAND,5);
  topLevelSizerH->Add(rightMainPanel,1,wxLEFT | wxEXPAND,5);
  SetSizerAndFit(topLevelSizerH);

  InitDefaultStates();
}

/////////////////////////
// SECOND LEVEL PANELS //
/////////////////////////

void MainFrame::InitLeftMainPanel(){
  // contains file panels and load button
  browsePanel = new wxPanel
    (leftMainPanel,
     PANEL_Browse,
     wxDefaultPosition,
     wxDefaultSize,
     wxTAB_TRAVERSAL
    );
  browsePanel->SetBackgroundColour(col_panel);
  browsePanel->SetMaxSize(wxSize(-1,160));

  // contains a grid widget that displays a table of atoms
  atomListPanel = new wxPanel
    (leftMainPanel,
     PANEL_AtomList,
     wxDefaultPosition,
     wxDefaultSize,
     wxTAB_TRAVERSAL);
  atomListPanel->SetBackgroundColour(col_panel);
  atomListPanel->SetMaxSize(wxSize(-1,200));

  InitBrowsePanel();
  InitAtomListPanel();

  wxBoxSizer *leftSizerV = new wxBoxSizer(wxVERTICAL);
  leftSizerV->Add(browsePanel,1,wxEXPAND,20);
  leftSizerV->Add(atomListPanel,1,wxEXPAND,20);
  leftMainPanel->SetSizerAndFit(leftSizerV);

}

void MainFrame::InitRightMainPanel(){

  // contains panels for user input
  parameterPanel = new wxPanel
    (rightMainPanel,
     PANEL_Parameters,
     wxDefaultPosition,
     wxDefaultSize,
     wxTAB_TRAVERSAL);
  parameterPanel->SetBackgroundColour(col_panel);

  // contains calculate button and output text control
  sandrPanel = new wxPanel
    (rightMainPanel,
     PANEL_Sandr,
     wxDefaultPosition,
     wxDefaultSize,
     wxTAB_TRAVERSAL);
  sandrPanel->SetBackgroundColour(col_panel);

  InitParametersPanel();
  InitSandr();

  wxBoxSizer *rightSizerV = new wxBoxSizer(wxVERTICAL);
  rightSizerV->Add(parameterPanel,0,wxEXPAND,20);
  rightSizerV->Add(sandrPanel,0,wxEXPAND,20);
  rightMainPanel->SetSizerAndFit(rightSizerV);

}

//////////////////
// BROWSE PANEL //
//////////////////
void MainFrame::InitBrowsePanel(){

  atomfilePanel = new wxPanel(browsePanel, PANEL_Atomfile, wxDefaultPosition, wxDefaultSize, 0);
  radiusfilePanel = new wxPanel(browsePanel, PANEL_Radiusfile, wxDefaultPosition, wxDefaultSize, 0);
  fileOptionsPanel = new wxPanel(browsePanel, PANEL_FileOptions, wxDefaultPosition, wxDefaultSize, 0);

  InitAtomfilePanel();
  InitRadiusfilePanel();
  InitFileOptionsPanel();

  wxStaticBoxSizer *browserSizer = new wxStaticBoxSizer(wxVERTICAL,browsePanel);
  browserSizer->Add(radiusfilePanel,0,wxEXPAND,0);
  browserSizer->Add(atomfilePanel,0,wxEXPAND,0);
  browserSizer->Add(fileOptionsPanel,0,wxEXPAND,0);
  browsePanel->SetSizerAndFit(browserSizer);

}

////////////////////////////
// SEND AND RECEIVE PANEL //
////////////////////////////
void MainFrame::InitSandr(){

  calcButton = new wxButton
    (sandrPanel,
     BUTTON_Calc,
     "Calculate",
     wxDefaultPosition,
     wxDefaultSize,
     0,
     wxDefaultValidator,
     "begin calculation"
    );
	calcButton->Enable(false);

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

  wxStaticBoxSizer *sandrSizer = new wxStaticBoxSizer(wxHORIZONTAL,sandrPanel);
  sandrSizer->Add(outputText,5,wxALIGN_LEFT | wxALL,10);
  sandrSizer->Add(calcButton,1,wxALIGN_CENTRE_VERTICAL | wxALL,10);
  sandrPanel->SetSizerAndFit(sandrSizer);

}

////////////////////
// FILE SELECTION //
////////////////////

// function used in InitAtomfilePanel and InitRadiusfilePanel to create and set the sizer
void MainFrame::SetSizerFilePanel(wxPanel* panel, wxStaticText* text, wxButton* button, wxTextCtrl* path){

  wxBoxSizer *fileSizer = new wxBoxSizer(wxHORIZONTAL);
  fileSizer->Add(text, 0, wxALIGN_CENTRE_VERTICAL | wxALL, 10);
  fileSizer->Add(path, 5, wxALL, 10);
  fileSizer->Add(button, 0, wxALL, 10);
  panel->SetSizerAndFit(fileSizer);
}

void MainFrame::InitAtomfilePanel(){
  atomText = new wxStaticText(atomfilePanel, TEXT_Atom, "Structure file:");
  browseButton = new wxButton
    (atomfilePanel, BUTTON_Browse, "Browse", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);

  filepathText = new wxTextCtrl(atomfilePanel,
								TEXT_Filename,
								wxEmptyString);

  SetSizerFilePanel(atomfilePanel, atomText, browseButton, filepathText);
}

void MainFrame::InitRadiusfilePanel(){
  radiusText = new wxStaticText(radiusfilePanel, TEXT_Radius, "Elements radii:");
  radiusButton = new wxButton
    (radiusfilePanel, BUTTON_Radius, "Browse");

  radiuspathText = new wxTextCtrl(radiusfilePanel,
								  TEXT_Radiuspath,
								  "./inputfile/radii.txt");

  SetSizerFilePanel(radiusfilePanel, radiusText, radiusButton, radiuspathText);
}

/////////////////////////////////////
// FILE SELECTION OPTIONS AND LOAD //
/////////////////////////////////////

void MainFrame::InitFileOptionsPanel(){
  pdbHetatmCheckbox = new wxCheckBox
    (fileOptionsPanel,
     CHECKBOX_Hetatm,
     "Include HETATM (for pdb files)",
     wxDefaultPosition,
     wxDefaultSize,
     0,
     wxDefaultValidator,
     "include HETATM"
    );
  pdbHetatmCheckbox->Enable(false);
  // Biochemists know what HETATM represent but other chemists might not
  // thus it is better to include HETATM by default as they are mostly useful for non-biochemists
  pdbHetatmCheckbox->SetValue(true);

  loadFilesButton = new wxButton
    (fileOptionsPanel,
     BUTTON_LoadFiles,
     "Reload",
     wxDefaultPosition,
     wxDefaultSize,
     0,
     wxDefaultValidator,
     "load input files"
    );
  loadFilesButton->Enable(false);

  wxBoxSizer *fileOptionsSizer = new wxBoxSizer(wxHORIZONTAL);
  fileOptionsSizer->Add(pdbHetatmCheckbox,1,wxALIGN_LEFT | wxALIGN_CENTRE_VERTICAL | wxALL,10);
  fileOptionsSizer->Add(loadFilesButton,1,wxALIGN_CENTRE_VERTICAL | wxALL,10);
  fileOptionsPanel->SetSizerAndFit(fileOptionsSizer);
}

//////////////////////
// PARAMETERS PANEL //
//////////////////////

void MainFrame::InitParametersPanel(){
  unitCellCheckbox = new wxCheckBox
    (parameterPanel,
     CHECKBOX_UnitCell,
     "Analyze crystal unit cell (for pdb files)",
     wxDefaultPosition,
     wxDefaultSize,
     0,
     wxDefaultValidator,
     "Unit Cell"
    );
  unitCellCheckbox->Enable(false);
  unitCellCheckbox->SetValue(false);

  surfaceAreaCheckbox = new wxCheckBox
    (parameterPanel,
     CHECKBOX_SurfaceArea,
     "Calculate surface areas (NOT AVAILABLE YET !)",
     wxDefaultPosition,
     wxDefaultSize,
     0,
     wxDefaultValidator,
     "Surface Area"
    );
  surfaceAreaCheckbox->Enable(true);
  surfaceAreaCheckbox->SetValue(false);

  twoProbesCheckbox = new wxCheckBox
    (parameterPanel,
     CHECKBOX_TwoProbes,
     "Use two probes mode (to dicern the inside from the outside of molecules)",
     wxDefaultPosition,
     wxDefaultSize,
     0,
     wxDefaultValidator,
     "Two probes"
    );
  twoProbesCheckbox->Enable(true);
  twoProbesCheckbox->SetValue(false);

  // contains input controls for probe 1 (small) radius
  probe1Panel = new wxPanel(parameterPanel, PANEL_Probe1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  probe1Panel->SetBackgroundColour(col_panel);

  // contains input controls for probe 2 (large) radius
  probe2Panel = new wxPanel(parameterPanel, PANEL_Probe2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  probe2Panel->SetBackgroundColour(col_panel);

  // contains input controls for grid size
  gridsizePanel = new wxPanel(parameterPanel, PANEL_Grid, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  gridsizePanel->SetBackgroundColour(col_panel);

  // contains input controls for tree depth
  depthPanel = new wxPanel(parameterPanel, PANEL_Depth, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  depthPanel->SetBackgroundColour(col_panel);

  reportCheckbox = new wxCheckBox
    (parameterPanel,
     CHECKBOX_Report,
     "Generate report file",
     wxDefaultPosition,
     wxDefaultSize,
     0,
     wxDefaultValidator,
     "Report file"
    );
  twoProbesCheckbox->Enable(true);
  reportCheckbox->SetValue(false);

  surfaceMapCheckbox = new wxCheckBox
    (parameterPanel,
     CHECKBOX_SurfaceMap,
     "Generate a total surface map file (to visualize in PyMol and Chimera)",
     wxDefaultPosition,
     wxDefaultSize,
     0,
     wxDefaultValidator,
     "Surface Map"
    );
  surfaceMapCheckbox->Enable(true);
  surfaceMapCheckbox->SetValue(false);

  cavityMapsCheckbox = new wxCheckBox
    (parameterPanel,
     CHECKBOX_CavityMaps,
     "Generate a surface map file for each cavity/pocket (to visualize in PyMol and Chimera) (NOT AVAILABLE YET !)",
     wxDefaultPosition,
     wxDefaultSize,
     0,
     wxDefaultValidator,
     "Cavity Maps"
    );
  cavityMapsCheckbox->Enable(true);
  cavityMapsCheckbox->SetValue(false);

  outputpathPanel = new wxPanel(parameterPanel, PANEL_Outputpath, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

  InitProbe1Panel();
  InitProbe2Panel();
  InitGridPanel();
  InitDepthPanel();
  InitOutputpathPanel();

  wxStaticBoxSizer *parameterSizer = new wxStaticBoxSizer(wxVERTICAL,parameterPanel);
  parameterSizer->Add(unitCellCheckbox,1,wxALIGN_LEFT | wxALL,10);
  parameterSizer->Add(surfaceAreaCheckbox,1,wxALIGN_LEFT | wxALL,10);
  parameterSizer->Add(twoProbesCheckbox,1,wxALIGN_LEFT | wxALL,10);
  parameterSizer->Add(probe1Panel,0,wxEXPAND,20);
  parameterSizer->Add(probe2Panel,0,wxEXPAND,20);
  parameterSizer->Add(gridsizePanel,0,wxEXPAND,20);
  parameterSizer->Add(depthPanel,0,wxEXPAND,20);
  parameterSizer->Add(reportCheckbox,1,wxALIGN_LEFT | wxALL,10);
  parameterSizer->Add(surfaceMapCheckbox,1,wxALIGN_LEFT | wxALL,10);
  parameterSizer->Add(cavityMapsCheckbox,1,wxALIGN_LEFT | wxALL,10);
  parameterSizer->Add(outputpathPanel,0,wxEXPAND,20);
  parameterPanel->SetSizerAndFit(parameterSizer);

  return;
}

void MainFrame::InitProbe1Panel(){

  probe1Text = new wxStaticText(probe1Panel, TEXT_Probe1, "Probe 1 radius (small):");

  // contains input control for probe 1 radius and text field for unit
  probe1InputPanel = new wxPanel(probe1Panel, PANEL_Probe1Input, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  probe1InputPanel->SetBackgroundColour(col_panel);

  InitProbe1InputPanel();

  wxBoxSizer *probe1Sizer = new wxBoxSizer(wxHORIZONTAL);
  probe1Sizer->Add(probe1Text,0,wxALL | wxALIGN_CENTRE_VERTICAL,10);
  probe1Sizer->Add(probe1InputPanel,0,0,0);
  probe1Panel->SetSizerAndFit(probe1Sizer);

  return;
}

void MainFrame::InitProbe1InputPanel(){

  probe1InputText = new wxTextCtrl(probe1InputPanel, TEXT_Probe1Input, "1.2");

  probe1UnitText = new wxStaticText(probe1InputPanel, TEXT_Probe1Unit, L"\u212B  (note: approximate H\u2082O radius = 1.4 \u212B)  "); // unicode for angstrom, biochemists often use a probe corresponding to a molecule of water

  wxBoxSizer *probe1Inputsizer = new wxBoxSizer(wxHORIZONTAL);
  probe1Inputsizer->Add(probe1InputText, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 10);
  probe1Inputsizer->Add(probe1UnitText, 0, wxALIGN_CENTRE_VERTICAL, 10);
  probe1InputPanel->SetSizerAndFit(probe1Inputsizer);

  return;
}

void MainFrame::InitProbe2Panel(){

  probe2Text = new wxStaticText(probe2Panel, TEXT_Probe2, "Probe 2 radius (large):");

  // contains input control for probe 2 radius and text field for unit
  probe2InputPanel = new wxPanel(probe2Panel, PANEL_Probe2Input, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  probe2InputPanel->SetBackgroundColour(col_panel);

  InitProbe2InputPanel();

  wxBoxSizer *probe2Sizer = new wxBoxSizer(wxHORIZONTAL);
  probe2Sizer->Add(probe2Text,0,wxALL | wxALIGN_CENTRE_VERTICAL,10);
  probe2Sizer->Add(probe2InputPanel,0,0,0);
  probe2Panel->SetSizerAndFit(probe2Sizer);

  return;
}

void MainFrame::InitProbe2InputPanel(){

  probe2InputText = new wxTextCtrl(probe2InputPanel, TEXT_Probe2Input, "5");

  probe2UnitText = new wxStaticText(probe2InputPanel, TEXT_Probe2Unit, L"\u212B "); // unicode for angstrom

  wxBoxSizer *probe2Inputsizer = new wxBoxSizer(wxHORIZONTAL);
  probe2Inputsizer->Add(probe2InputText, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 10);
  probe2Inputsizer->Add(probe2UnitText, 0, wxALIGN_CENTRE_VERTICAL, 10);
  probe2InputPanel->SetSizerAndFit(probe2Inputsizer);

  probe2InputText->Enable(false);

  return;
}

void MainFrame::InitGridPanel(){

  gridsizeText = new wxStaticText(gridsizePanel, TEXT_Grid, "Grid step size:");

  // contains input control for grid size and text field for unit
  gridsizeInputPanel = new wxPanel(gridsizePanel, PANEL_Gridinput, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  gridsizeInputPanel->SetBackgroundColour(col_panel);

  InitGridinputPanel();

  wxBoxSizer *gridsizeSizer = new wxBoxSizer(wxHORIZONTAL);
  gridsizeSizer->Add(gridsizeText,0,wxALL | wxALIGN_CENTRE_VERTICAL,10);
  gridsizeSizer->Add(gridsizeInputPanel,0,0,0);
  gridsizePanel->SetSizerAndFit(gridsizeSizer);

  return;
}

void MainFrame::InitGridinputPanel(){

  gridsizeInputText = new wxTextCtrl(gridsizeInputPanel, TEXT_Gridinput, "0.1");

  gridsizeUnitText = new wxStaticText(gridsizeInputPanel, TEXT_Gridunit, L"\u212B "); // unicode for angstrom

  wxBoxSizer *gridsizeInputsizer = new wxBoxSizer(wxHORIZONTAL);
  gridsizeInputsizer->Add(gridsizeInputText, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 10);
  gridsizeInputsizer->Add(gridsizeUnitText, 0, wxALIGN_CENTRE_VERTICAL, 10);
  gridsizeInputPanel->SetSizerAndFit(gridsizeInputsizer);

  return;
}

void MainFrame::InitDepthPanel(){

  depthText = new wxStaticText(depthPanel, TEXT_Depth, "Maximum tree depth:");

  depthInput = new wxSpinCtrl
    (depthPanel, SPIN_Depthinput, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 20, 4);

  wxBoxSizer *depthSizer = new wxBoxSizer(wxHORIZONTAL);
  depthSizer->Add(depthText,0,wxALIGN_CENTRE_VERTICAL | wxALL,10);
  depthSizer->Add(depthInput,0,wxALIGN_CENTRE_VERTICAL | wxALL,10);
  depthPanel->SetSizerAndFit(depthSizer);
  return;
}

void MainFrame::InitOutputpathPanel(){
  outputdirText = new wxStaticText(outputpathPanel, TEXT_Outputdir, "Output directory:");
  outputdirPicker = new wxDirPickerCtrl(outputpathPanel, BUTTON_Output, ".");

  wxBoxSizer *outputSizer = new wxBoxSizer(wxHORIZONTAL);
  outputSizer->Add(outputdirText,0, wxALIGN_CENTRE_VERTICAL | wxALL,10);
  outputSizer->Add(outputdirPicker,5, wxALL,10);
  outputpathPanel->SetSizerAndFit(outputSizer);
}

/////////////////////
// ATOM LIST PANEL //
/////////////////////

void MainFrame::InitAtomListPanel(){
  atomListGrid = new wxGrid(atomListPanel, GRID_AtomList, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);
  atomListGrid->CreateGrid(0, 4, wxGrid::wxGridSelectCells);
  atomListGrid->SetDefaultCellAlignment (wxALIGN_CENTRE, wxALIGN_CENTRE);

  // columns
  atomListGrid->SetColLabelValue(0, "Include");
  atomListGrid->SetColFormatBool(0);

  atomListGrid->SetColLabelValue(1, "Element");

  atomListGrid->SetColLabelValue(2, "Number \nof Atoms");
  atomListGrid->SetColFormatNumber(2);

  atomListGrid->SetColLabelValue(3, L"Radius (\u212B)");
  atomListGrid->SetColFormatFloat(3, 5, 3); // 2nd argument is width, last argument is precision

  wxStaticBoxSizer *atomListSizer = new wxStaticBoxSizer(wxVERTICAL,atomListPanel);
  atomListSizer->Add(atomListGrid,1,wxEXPAND,20); // proportion factor has to be 1, else atom list does not expand
  atomListPanel->SetSizerAndFit(atomListSizer);
  return;
}


