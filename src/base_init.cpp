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
    silenceGUI(true); // not really needed but doesn't hurt
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
    else if (unittest_id=="2probe"){
      Ctrl::getInstance()->unittest2Probe();
    }
    else if (unittest_id=="surface"){
      Ctrl::getInstance()->unittestSurface();
    }
    else if (unittest_id=="floodfill"){
      Ctrl::getInstance()->unittestFloodfill();
    }
    else {
      std::cout << "Invalid selection" << std::endl;}
    return true;
  }
  // initialise the GUI
  MainFrame* MainWin = new MainFrame(_("MoloVol " + Ctrl::s_version), wxDefaultPosition, wxDefaultSize);
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
  for (const auto& widget : widgets_enabled){
    default_states[widget] = true;
    widget->Enable(true);
  }
  for (const auto& widget : widgets_disabled){
    default_states[widget] = false;
    widget->Enable(false);
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
  // contains import panel (left main) and options panel (right main)
  preCalcPanel = new wxPanel(this,PANEL_PreCalc);
  preCalcPanel->SetBackgroundColour(col_panel);

  postCalcPanel = new wxPanel(this,PANEL_PostCalc);
  postCalcPanel->SetBackgroundColour(col_panel);

  InitPreCalcPanel();
  InitPostCalcPanel();
  
  wxBoxSizer *boxSizerV = new wxBoxSizer(wxVERTICAL);
  boxSizerV->Add(preCalcPanel, 0, wxEXPAND);
  boxSizerV->Add(postCalcPanel, 0, wxEXPAND);
  SetSizerAndFit(boxSizerV);

  InitDefaultStates();
}

////////////////////
// PRE CALC PANEL //
////////////////////

void MainFrame::InitPreCalcPanel(){
  // contains browse panel and atom list panel
  leftMainPanel = new wxPanel(preCalcPanel,PANEL_LeftMain);
  leftMainPanel->SetBackgroundColour(col_panel);

  // contains parameter panel and send-and-receive panel
  rightMainPanel = new wxPanel(preCalcPanel,PANEL_RightMain);
  rightMainPanel->SetBackgroundColour(col_panel);
  
  InitLeftMainPanel();
  InitRightMainPanel();

  wxBoxSizer *topLevelSizerH = new wxBoxSizer(wxHORIZONTAL);
  topLevelSizerH->Add(leftMainPanel, 1, wxEXPAND);
  topLevelSizerH->Add(rightMainPanel, 1, wxEXPAND);
  preCalcPanel->SetSizerAndFit(topLevelSizerH);
}

/////////////////////////
// SECOND LEVEL PANELS //
/////////////////////////

void MainFrame::InitLeftMainPanel(){
  // contains file panels and load button
  browsePanel = new wxPanel(leftMainPanel, PANEL_Browse);
  browsePanel->SetBackgroundColour(col_panel);
  browsePanel->SetMaxSize(wxSize(-1,160));

  // contains a grid widget that displays a table of atoms
  atomListPanel = new wxPanel(leftMainPanel,PANEL_AtomList);
  atomListPanel->SetBackgroundColour(col_panel);
  atomListPanel->SetMaxSize(wxSize(-1,200));

  InitBrowsePanel();
  InitAtomListPanel();

  wxBoxSizer *leftSizerV = new wxBoxSizer(wxVERTICAL);
  leftSizerV->Add(browsePanel,1,wxEXPAND);
  leftSizerV->Add(atomListPanel,1,wxEXPAND);
  leftMainPanel->SetSizerAndFit(leftSizerV);
}

void MainFrame::InitRightMainPanel(){
  // contains panels for user input
  parameterPanel = new wxPanel(rightMainPanel,PANEL_Parameters);
  parameterPanel->SetBackgroundColour(col_panel);

  // contains calculate button
  sandrPanel = new wxPanel(rightMainPanel,PANEL_Sandr);
  sandrPanel->SetBackgroundColour(col_panel);

  InitParametersPanel();
  InitSandr();

  wxBoxSizer *rightSizerV = new wxBoxSizer(wxVERTICAL);
  rightSizerV->Add(parameterPanel, 0, wxEXPAND);
  rightSizerV->Add(sandrPanel, 0, wxEXPAND);
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
  browserSizer->Add(radiusfilePanel,0,wxEXPAND | wxTOP,6);
  browserSizer->Add(atomfilePanel,0,wxEXPAND,0);
  browserSizer->Add(fileOptionsPanel,0,wxEXPAND,0);
  browsePanel->SetSizerAndFit(browserSizer);

}

////////////////////////////
// SEND AND RECEIVE PANEL //
////////////////////////////
void MainFrame::InitSandr(){

  calcButton = new wxButton(sandrPanel,BUTTON_Calc,"Calculate");
	calcButton->Enable(false);

  wxStaticBoxSizer *sandrSizer = new wxStaticBoxSizer(wxHORIZONTAL,sandrPanel);
  sandrSizer->Add(calcButton,1,wxALIGN_CENTRE_VERTICAL);
  sandrPanel->SetSizerAndFit(sandrSizer);

}

////////////////////
// FILE SELECTION //
////////////////////

// function used in InitAtomfilePanel and InitRadiusfilePanel to create and set the sizer
void MainFrame::SetSizerFilePanel(wxPanel* panel, wxStaticText* text, wxButton* button, wxTextCtrl* path){

  wxBoxSizer *fileSizer = new wxBoxSizer(wxHORIZONTAL);
  fileSizer->Add(text, 0, wxALIGN_CENTRE_VERTICAL | wxRIGHT | wxBOTTOM, 10);
  fileSizer->Add(path, 5, wxRIGHT | wxLEFT | wxBOTTOM, 10);
  fileSizer->Add(button, 0, wxLEFT | wxBOTTOM, 10);
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
  pdbHetatmCheckbox = new wxCheckBox(fileOptionsPanel, CHECKBOX_Hetatm, "Include HETATM");
  // Biochemists know what HETATM represent but other chemists might not
  // thus it is better to include HETATM by default as they are mostly useful for non-biochemists
  pdbHetatmCheckbox->SetValue(true);

  loadFilesButton = new wxButton(fileOptionsPanel, BUTTON_LoadFiles, "Reload");

  wxBoxSizer *fileOptionsSizer = new wxBoxSizer(wxHORIZONTAL);
  fileOptionsSizer->Add(pdbHetatmCheckbox,1,wxALIGN_LEFT | wxALIGN_CENTRE_VERTICAL | wxTOP | wxLEFT | wxRIGHT,10);
  fileOptionsSizer->Add(loadFilesButton,1,wxALIGN_CENTRE_VERTICAL | wxTOP | wxLEFT,10);
  fileOptionsPanel->SetSizerAndFit(fileOptionsSizer);
}

//////////////////////
// PARAMETERS PANEL //
//////////////////////

void MainFrame::InitParametersPanel(){
  // this panel is used to set a distance between widgets and the border of the static box
  wxPanel *framePanel = new wxPanel(parameterPanel);
  {
    unitCellCheckbox = new wxCheckBox(framePanel, CHECKBOX_UnitCell, "Analyze crystal unit cell (.pdb file required)");
    surfaceAreaCheckbox = new wxCheckBox(framePanel, CHECKBOX_SurfaceArea, "Calculate surface areas");
    twoProbesCheckbox = new wxCheckBox(framePanel, CHECKBOX_TwoProbes, "Enable two-probe mode");

    // contains input controls for probe 1 (small) radius
    probe1Panel = new wxPanel(framePanel, PANEL_Probe1);
    probe1Panel->SetBackgroundColour(col_panel);

    // contains input controls for probe 2 (large) radius
    probe2Panel = new wxPanel(framePanel, PANEL_Probe2);
    probe2Panel->SetBackgroundColour(col_panel);

    // contains input controls for grid size
    gridsizePanel = new wxPanel(framePanel, PANEL_Grid);
    gridsizePanel->SetBackgroundColour(col_panel);

    // contains input controls for tree depth
    depthPanel = new wxPanel(framePanel, PANEL_Depth);
    depthPanel->SetBackgroundColour(col_panel);

    InitProbe1Panel();
    InitProbe2Panel();
    InitGridPanel();
    InitDepthPanel();

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    int dist_inbetween = 6;
    boxSizer->Add(unitCellCheckbox, 0, wxALIGN_LEFT | wxBOTTOM, dist_inbetween);
    boxSizer->Add(surfaceAreaCheckbox, 0, wxALIGN_LEFT | wxTOP | wxBOTTOM, dist_inbetween);
    boxSizer->Add(twoProbesCheckbox, 0, wxALIGN_LEFT | wxTOP | wxBOTTOM, dist_inbetween);
    boxSizer->Add(probe1Panel, 0, wxEXPAND | wxBOTTOM, dist_inbetween);
    boxSizer->Add(probe2Panel, 0, wxEXPAND | wxBOTTOM, dist_inbetween);
    boxSizer->Add(gridsizePanel, 0, wxEXPAND | wxTOP | wxBOTTOM, dist_inbetween);
    boxSizer->Add(depthPanel, 0, wxEXPAND | wxTOP, dist_inbetween);
    framePanel->SetSizerAndFit(boxSizer);
  }

  wxStaticBoxSizer *parameterSizer = new wxStaticBoxSizer(wxVERTICAL,parameterPanel);
  parameterSizer->Add(framePanel, 0, wxEXPAND | wxALL, 5);
  parameterPanel->SetSizerAndFit(parameterSizer);

  return;
}

void MainFrame::InitProbe1Panel(){

  probe1Text = new wxStaticText(probe1Panel, TEXT_Probe1, "Small Probe radius:");

  // contains input control for probe 1 radius and text field for unit
  probe1InputPanel = new wxPanel(probe1Panel, PANEL_Probe1Input);
  probe1InputPanel->SetBackgroundColour(col_panel);

  InitProbe1InputPanel();

  wxBoxSizer *probe1Sizer = new wxBoxSizer(wxHORIZONTAL);
  probe1Sizer->Add(probe1Text,1,wxALIGN_CENTRE_VERTICAL);
  probe1Sizer->Add(probe1InputPanel,2);
  probe1Panel->SetSizerAndFit(probe1Sizer);

  return;
}

void MainFrame::InitProbe1InputPanel(){

  probe1InputText = new wxTextCtrl(probe1InputPanel, TEXT_Probe1Input, "1.2");

  probe1UnitText = new wxStaticText(probe1InputPanel, TEXT_Probe1Unit, L" \u212B");//  (note: approximate H\u2082O radius = 1.4 \u212B)  "); // unicode for angstrom, biochemists often use a probe corresponding to a molecule of water

  wxBoxSizer *probe1Inputsizer = new wxBoxSizer(wxHORIZONTAL);
  probe1Inputsizer->Add(probe1InputText, 1, wxALIGN_CENTRE_VERTICAL);
  probe1Inputsizer->Add(probe1UnitText, 0, wxALIGN_CENTRE_VERTICAL);
  probe1InputPanel->SetSizerAndFit(probe1Inputsizer);

  return;
}

void MainFrame::InitProbe2Panel(){

  probe2Text = new wxStaticText(probe2Panel, TEXT_Probe2, "Large Probe radius:");

  // contains input control for probe 2 radius and text field for unit
  probe2InputPanel = new wxPanel(probe2Panel, PANEL_Probe2Input, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  probe2InputPanel->SetBackgroundColour(col_panel);

  InitProbe2InputPanel();

  wxBoxSizer *probe2Sizer = new wxBoxSizer(wxHORIZONTAL);
  probe2Sizer->Add(probe2Text, 1, wxALIGN_CENTRE_VERTICAL);
  probe2Sizer->Add(probe2InputPanel, 2);
  probe2Panel->SetSizerAndFit(probe2Sizer);

  return;
}

void MainFrame::InitProbe2InputPanel(){

  probe2InputText = new wxTextCtrl(probe2InputPanel, TEXT_Probe2Input, "5");

  probe2UnitText = new wxStaticText(probe2InputPanel, TEXT_Probe2Unit, L" \u212B"); // unicode for angstrom

  wxBoxSizer *probe2Inputsizer = new wxBoxSizer(wxHORIZONTAL);
  probe2Inputsizer->Add(probe2InputText, 1, wxALIGN_CENTRE_VERTICAL);
  probe2Inputsizer->Add(probe2UnitText, 0, wxALIGN_CENTRE_VERTICAL);
  probe2InputPanel->SetSizerAndFit(probe2Inputsizer);

  probe2InputText->Enable(false);

  return;
}

void MainFrame::InitGridPanel(){

  gridsizeText = new wxStaticText(gridsizePanel, TEXT_Grid, "Grid resolution:");

  // contains input control for grid size and text field for unit
  gridsizeInputPanel = new wxPanel(gridsizePanel, PANEL_Gridinput, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  gridsizeInputPanel->SetBackgroundColour(col_panel);

  InitGridinputPanel();

  wxBoxSizer *gridsizeSizer = new wxBoxSizer(wxHORIZONTAL);
  gridsizeSizer->Add(gridsizeText, 1, wxALIGN_CENTRE_VERTICAL);
  gridsizeSizer->Add(gridsizeInputPanel, 2);
  gridsizePanel->SetSizerAndFit(gridsizeSizer);

  return;
}

void MainFrame::InitGridinputPanel(){

  gridsizeInputText = new wxTextCtrl(gridsizeInputPanel, TEXT_Gridinput, "0.1");

  gridsizeUnitText = new wxStaticText(gridsizeInputPanel, TEXT_Gridunit, L" \u212B"); // unicode for angstrom

  wxBoxSizer *gridsizeInputsizer = new wxBoxSizer(wxHORIZONTAL);
  gridsizeInputsizer->Add(gridsizeInputText, 1, wxALIGN_CENTRE_VERTICAL);
  gridsizeInputsizer->Add(gridsizeUnitText, 0, wxALIGN_CENTRE_VERTICAL);
  gridsizeInputPanel->SetSizerAndFit(gridsizeInputsizer);

  return;
}

void MainFrame::InitDepthPanel(){

  depthText = new wxStaticText(depthPanel, TEXT_Depth, "Optimization depth:");
  depthInput = new wxSpinCtrl
    (depthPanel, SPIN_Depthinput, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 20, 4);

  wxBoxSizer *depthSizer = new wxBoxSizer(wxHORIZONTAL);
  depthSizer->Add(depthText,1,wxALIGN_CENTRE_VERTICAL);
  depthSizer->Add(depthInput,2,wxALIGN_CENTRE_VERTICAL);
  depthPanel->SetSizerAndFit(depthSizer);
  return;
}

/////////////////////
// ATOM LIST PANEL //
/////////////////////

void MainFrame::InitAtomListPanel(){
  atomListGrid = new wxGrid(atomListPanel, GRID_AtomList, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);
  atomListGrid->SetRowLabelSize(30);
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
  atomListSizer->Add(atomListGrid,1,wxEXPAND); // proportion factor has to be 1, else atom list does not expand
  atomListPanel->SetSizerAndFit(atomListSizer);
  return;
}

/////////////////////
// POST CALC PANEL //
/////////////////////

void MainFrame::InitPostCalcPanel(){
  communicationPanel = new wxPanel(postCalcPanel,PANEL_Communication);
  exportPanel = new wxPanel(postCalcPanel,PANEL_Export);

  InitCommunicationPanel();
  InitExportPanel();

  wxBoxSizer *boxSizerV = new wxBoxSizer(wxVERTICAL);
  boxSizerV->Add(communicationPanel, 0, wxEXPAND, 0);
  boxSizerV->Add(exportPanel, 0, wxEXPAND, 0);
  postCalcPanel->SetSizerAndFit(boxSizerV);
}

/////////////////////////
// COMMUNICATION PANEL //
/////////////////////////

void MainFrame::InitCommunicationPanel(){
  progressGauge = new wxGauge(communicationPanel,GAUGE_Progress, 100);
  outputPanel = new wxPanel(communicationPanel,PANEL_Output);
  InitOutputPanel();

  wxStaticBoxSizer *boxSizerV = new wxStaticBoxSizer(wxVERTICAL,communicationPanel);
  boxSizerV->Add(progressGauge, 0, wxBOTTOM | wxLEFT | wxRIGHT | wxEXPAND, 5);
  boxSizerV->Add(outputPanel, 0, wxTOP | wxEXPAND, 5);
  communicationPanel->SetSizerAndFit(boxSizerV);
}

void MainFrame::InitOutputPanel(){
  outputText = new wxTextCtrl(outputPanel, TEXT_Output, _("Output"), wxDefaultPosition, wxSize(-1,100), wxTE_MULTILINE | wxTE_READONLY);
  outputText->SetBackgroundColour(col_output);

  outputGrid = new wxGrid(outputPanel, GRID_Output);
  outputGrid->SetRowLabelSize(0);
  outputGrid->CreateGrid(2, 4, wxGrid::wxGridSelectCells);
  outputGrid->SetDefaultCellAlignment (wxALIGN_CENTRE, wxALIGN_CENTRE);

  // columns
  outputGrid->SetColLabelValue(0, "Cavity");
  outputGrid->SetColFormatNumber(0);

  outputGrid->SetColLabelValue(1, "Volume");
  outputGrid->SetColFormatFloat(1);

  outputGrid->SetColLabelValue(2, "Core Surface");
  outputGrid->SetColFormatFloat(2);
  
  outputGrid->SetColLabelValue(3, "Core Surface");
  outputGrid->SetColFormatFloat(3);

  outputGrid->SetColFormatFloat(3, 5, 3); // 2nd argument is width, last argument is precision

  wxBoxSizer *boxSizerH = new wxBoxSizer(wxHORIZONTAL);
  boxSizerH->Add(outputText, 1, wxRIGHT | wxEXPAND, 2);
  boxSizerH->Add(outputGrid, 1, wxLEFT | wxEXPAND, 2);
  outputPanel->SetSizerAndFit(boxSizerH);
}

//////////////////
// EXPORT PANEL //
//////////////////

void MainFrame::InitExportPanel(){
  reportExportPanel = new wxPanel(exportPanel, PANEL_ReportExport);
  totalMapExportPanel = new wxPanel(exportPanel, PANEL_TotalMapExport);
  cavityMapExportPanel = new wxPanel(exportPanel, PANEL_CavityMapExport);
  autoExportPanel = new wxPanel(exportPanel, PANEL_AutoExport);

  InitReportExportPanel();
  InitTotalMapExportPanel();
  InitCavityMapExportPanel();
  InitAutoExportPanel();

  wxStaticBoxSizer* boxSizerV = new wxStaticBoxSizer(wxVERTICAL,exportPanel);
  boxSizerV->Add(reportExportPanel, 1, wxBOTTOM | wxEXPAND, 2);
  boxSizerV->Add(totalMapExportPanel, 1, wxBOTTOM | wxEXPAND, 2);
  boxSizerV->Add(cavityMapExportPanel, 1, wxBOTTOM | wxEXPAND, 2);
  boxSizerV->Add(autoExportPanel, 1, wxEXPAND, 2);
  exportPanel->SetSizerAndFit(boxSizerV);
}

void MainFrame::InitReportExportPanel(){
  reportButton = new wxButton(reportExportPanel, BUTTON_Report, "Export report");
  reportCheckbox = new wxCheckBox(reportExportPanel, CHECKBOX_Report,"Auto export");

  SetSizerExportSubPanel(reportExportPanel, reportButton, reportCheckbox);
}

void MainFrame::InitTotalMapExportPanel(){
  totalMapButton = new wxButton(totalMapExportPanel, BUTTON_TotalMap, "Export total surface map");
  surfaceMapCheckbox = new wxCheckBox(totalMapExportPanel, CHECKBOX_SurfaceMap, "Auto export");
  
  SetSizerExportSubPanel(totalMapExportPanel, totalMapButton, surfaceMapCheckbox);
}

void MainFrame::InitCavityMapExportPanel(){
  cavityMapButton = new wxButton(cavityMapExportPanel, BUTTON_CavityMap, "Export cavity maps");
  cavityMapsCheckbox = new wxCheckBox(cavityMapExportPanel, CHECKBOX_CavityMaps, "Auto export");
  
  SetSizerExportSubPanel(cavityMapExportPanel, cavityMapButton, cavityMapsCheckbox);
}

void MainFrame::InitAutoExportPanel(){
  // these panels serve to align the widgets with the checkboxes
  wxPanel* emptyPanel = new wxPanel(autoExportPanel);
  wxPanel* subPanel = new wxPanel(autoExportPanel);

  {
    outputdirText = new wxStaticText(subPanel, TEXT_Outputdir, "Auto save directory:");
    outputdirPicker = new wxDirPickerCtrl(subPanel, BUTTON_Output, "", _("Select Output Directory"));

    wxBoxSizer *boxSizerH = new wxBoxSizer(wxHORIZONTAL);
    boxSizerH->Add(outputdirText,0, wxALIGN_CENTRE_VERTICAL);
    boxSizerH->Add(outputdirPicker,5, wxLEFT, 20);
    subPanel->SetSizerAndFit(boxSizerH);
  }

  wxBoxSizer* subSizer = new wxBoxSizer(wxHORIZONTAL);
  subSizer->Add(emptyPanel,1);
  subSizer->Add(subPanel,1);
  autoExportPanel->SetSizerAndFit(subSizer);
}

void MainFrame::SetSizerExportSubPanel(wxPanel* parentPanel, wxButton* button, wxCheckBox* checkbox){
  // button panel is used to stop buttons from expanding vertically
  wxPanel* buttonPanel = new wxPanel(parentPanel);
  button->Reparent(buttonPanel);
  button->SetSize(200,-1);

  wxBoxSizer* boxSizerH = new wxBoxSizer(wxHORIZONTAL);
  boxSizerH->Add(buttonPanel, 1);
  boxSizerH->Add(checkbox, 1);
  parentPanel->SetSizerAndFit(boxSizerH);
}

