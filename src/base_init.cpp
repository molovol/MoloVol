#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#  include <wx/wx.h>
#endif

#include "base.h"
#include "special_chars.h"
#include "controller.h"
#include "misc.h"
#include <cassert>

// wxWidgets macro that contains the entry point, initialised the app, and calls wxApp::OnInit()
IMPLEMENT_APP(MainApp)

/////////////////////////////
// MAIN APP IS INITIALISED //
/////////////////////////////

// first custom code that is run
bool MainApp::OnInit()
{
  if (argc != 1){
    // command line interface
    evalCmdLine();
  }
  else {
    // initialise the GUI
    MainFrame* MainWin = new MainFrame(_("MoloVol " + Ctrl::s_version), wxDefaultPosition, wxDefaultSize);
    MainWin->Show(true);
    SetTopWindow(MainWin);
  }
  return true;
};

// contains all command line options
static const wxCmdLineEntryDesc cmd_line_desc[] =
{
  // required
  { wxCMD_LINE_OPTION, "r", "radius", "Probe radius", wxCMD_LINE_VAL_DOUBLE},
  { wxCMD_LINE_OPTION, "g", "grid", "Spatial resolution of the underlying grid", wxCMD_LINE_VAL_DOUBLE},
  { wxCMD_LINE_OPTION, "fs", "file-structure", "Path to the structure file", wxCMD_LINE_VAL_STRING},
  // optional
  { wxCMD_LINE_OPTION, "fr", "file-radius", "Path to the radius file", wxCMD_LINE_VAL_STRING},
  { wxCMD_LINE_OPTION, "d", "depth", "Octree depth", wxCMD_LINE_VAL_NUMBER},
  { wxCMD_LINE_SWITCH, "s", "silent", "Silence progress reporting", wxCMD_LINE_VAL_NONE, 0},
  { wxCMD_LINE_OPTION, "u", "unittest", "Run a programmed unit test", wxCMD_LINE_VAL_STRING},
  { wxCMD_LINE_NONE }
};

static const std::vector<std::string> required_args = {"r", "g", "fs"};

// return true to supress GUI, return false to open GUI
void MainApp::evalCmdLine(){
  // if there are no cmd line arguments, open app normally
  silenceGUI(true);
  wxCmdLineParser parser = wxCmdLineParser(argc,argv);
  parser.SetDesc(cmd_line_desc);
  // if something is wrong with the cmd line args, stop
  if(parser.Parse() != 0){return;}
  // unit tests
  wxString unittest_id;
  if (parser.Found("u",&unittest_id)){
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
      std::cout << "Invalid selection" << std::endl;
    }
    return;
  }
  // check if all required arguments are available
  for (auto& arg_name : required_args){
    if (!parser.Found(arg_name)){
      Ctrl::getInstance()->displayErrorMessage(901);
      return;
    }
  }

  // minimum required arguments for calculation
  double probe_radius_s;
  double grid_resolution;
  wxString structure_file_path;

  parser.Found("r",&probe_radius_s);
  parser.Found("g",&grid_resolution);
  parser.Found("fs",&structure_file_path);
 
  // optional arguments with default values
  wxString radius_file_path = getResourcesDir() + "/radii.txt";
  wxString output_dir_path = "";
  wxString output = "vol";
  double probe_radius_l = 0;
  long tree_depth = 4;
  bool opt_include_hetatm = false;
  bool opt_unit_cell = false;
  bool opt_surface_area = false;
  bool opt_probe_mode = false;
  bool exp_report = false;
  bool exp_total_map = false;
  bool exp_cavity_maps = false;

  parser.Found("fr",&radius_file_path);
  parser.Found("d",&tree_depth);

  // run calculation
  Ctrl::getInstance()->runCalculation(
      probe_radius_s, 
      probe_radius_l, 
      grid_resolution, 
      structure_file_path.ToStdString(), 
      radius_file_path.ToStdString(),
      output_dir_path.ToStdString(),
      (int)tree_depth,
      opt_include_hetatm,
      opt_unit_cell,
      opt_surface_area,
      opt_probe_mode,
      exp_report,
      exp_total_map,
      exp_cavity_maps);
}

// OnRun() is called after OnInit() returns true. In order to suppress the GUI, the attribute "silent" has to
// be toggled. this can be done by opening the app from the command line
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
    cavityMapsCheckbox,
    dirpickerButton,
    dirpickerText};
  wxWindow* widgets_disabled[] = {
    pdbHetatmCheckbox,
    loadFilesButton,
    unitCellCheckbox,
    probe2InputText,
    calcButton,
    reportButton,
    totalMapButton,
    cavityMapButton,
    abortButton};

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

void MainApp::silenceGUI(bool set){Ctrl::getInstance()->disableGUI();}
bool MainApp::isSilent(){return !Ctrl::getInstance()->isGUIEnabled();}

void MainFrame::InitMessageQueue(){
  _abort_q = new wxMessageQueue<bool>();
  _abort_q->Clear();
}

////////////////////////////////////
// INITIALISATION OF GUI ELEMENTS //
////////////////////////////////////

/////////////////////
// TOP LEVEL FRAME //
/////////////////////
void MainFrame::InitTopLevel(){
  // contains import panel (left main) and options panel (right main)
  preCalcPanel = new wxPanel(this,PANEL_PreCalc);
  postCalcPanel = new wxPanel(this,PANEL_PostCalc, wxDefaultPosition, wxDefaultSize, 0);

  InitPreCalcPanel();
  InitPostCalcPanel();

  statusBar = new wxStatusBar(this, STATUSBAR);
  statusBar->SetFieldsCount(1);
  statusBar->SetStatusText("Welcome!");

  wxBoxSizer *boxSizerV = new wxBoxSizer(wxVERTICAL);
  boxSizerV->Add(preCalcPanel, 1, wxEXPAND);
  boxSizerV->Add(postCalcPanel, 1, wxEXPAND);
  boxSizerV->Add(statusBar, 0, wxEXPAND);
  SetSizerAndFit(boxSizerV);

  InitDefaultStates();
}

////////////////////
// PRE CALC PANEL //
////////////////////

void MainFrame::InitPreCalcPanel(){
  // contains browse panel and atom list panel
  leftMainPanel = new wxPanel(preCalcPanel,PANEL_LeftMain);
  // contains parameter panel and send-and-receive panel
  rightMainPanel = new wxPanel(preCalcPanel,PANEL_RightMain);

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
  browsePanel->SetMaxSize(wxSize(-1,160));

  // contains a grid widget that displays a table of atoms
  atomListPanel = new wxPanel(leftMainPanel,PANEL_AtomList, wxDefaultPosition, wxDefaultSize, 0);

  InitBrowsePanel();
  InitAtomListPanel();

  wxBoxSizer *leftSizerV = new wxBoxSizer(wxVERTICAL);
  leftSizerV->Add(browsePanel,0,wxEXPAND);
  leftSizerV->Add(atomListPanel,1,wxEXPAND);
  leftMainPanel->SetSizerAndFit(leftSizerV);
}

void MainFrame::InitRightMainPanel(){
  // contains panels for user input
  parameterPanel = new wxPanel(rightMainPanel,PANEL_Parameters);

  // contains calculate button
  sandrPanel = new wxPanel(rightMainPanel,PANEL_Sandr, wxDefaultPosition, wxDefaultSize, 0);

  InitParametersPanel();
  InitSandr();

  wxBoxSizer *rightSizerV = new wxBoxSizer(wxVERTICAL);
  rightSizerV->Add(parameterPanel, 1, wxEXPAND);
  rightSizerV->Add(sandrPanel, 0, wxEXPAND);
  rightMainPanel->SetSizerAndFit(rightSizerV);
}

//////////////////
// BROWSE PANEL //
//////////////////
void MainFrame::InitBrowsePanel(){

  radiusfilePanel = new wxPanel(browsePanel, PANEL_Radiusfile);
  atomfilePanel = new wxPanel(browsePanel, PANEL_Atomfile);
  fileOptionsPanel = new wxPanel(browsePanel, PANEL_FileOptions, wxDefaultPosition, wxDefaultSize, 0);

  InitRadiusfilePanel();
  InitAtomfilePanel();
  InitFileOptionsPanel();

  wxStaticBoxSizer *browserSizer = new wxStaticBoxSizer(wxVERTICAL,browsePanel);
  browserSizer->Add(radiusfilePanel,0,wxEXPAND | wxTOP,6);
  browserSizer->Add(atomfilePanel,0,wxEXPAND | wxTOP,10);
  browserSizer->Add(fileOptionsPanel,0,wxEXPAND,0);
  browsePanel->SetSizerAndFit(browserSizer);
}

////////////////////////////
// SEND AND RECEIVE PANEL //
////////////////////////////
void MainFrame::InitSandr(){

  calcButton = new wxButton(sandrPanel,BUTTON_Calc,"Calculate");
  abortButton = new wxButton(sandrPanel,BUTTON_Abort,"Abort");

  wxStaticBoxSizer *sandrSizer = new wxStaticBoxSizer(wxHORIZONTAL,sandrPanel);
  sandrSizer->Add(calcButton,3,wxALIGN_CENTRE_VERTICAL);
  sandrSizer->Add(abortButton,1,wxALIGN_CENTRE_VERTICAL);
  sandrPanel->SetSizerAndFit(sandrSizer);
}

////////////////////
// FILE SELECTION //
////////////////////

// function used in InitAtomfilePanel and InitRadiusfilePanel to create and set the sizer
void MainFrame::SetSizerFilePanel(wxPanel* panel, wxStaticText* text, wxButton* button, wxTextCtrl* path){

  wxBoxSizer *fileSizer = new wxBoxSizer(wxHORIZONTAL);
  fileSizer->Add(text, 0, wxALIGN_CENTRE_VERTICAL | wxRIGHT, 5);
  fileSizer->Add(path, 1, wxRIGHT, 5);
  fileSizer->Add(button, 0,0);
  panel->SetSizerAndFit(fileSizer);
}

void MainFrame::InitAtomfilePanel(){
  atomText = new wxStaticText(atomfilePanel, TEXT_Atom, "Structure file:");
  browseButton = new wxButton(atomfilePanel, BUTTON_Browse, "Browse");
  filepathText = new wxTextCtrl(atomfilePanel, TEXT_Filename, wxEmptyString);

  SetSizerFilePanel(atomfilePanel, atomText, browseButton, filepathText);
}

void MainFrame::InitRadiusfilePanel(){
  radiusText = new wxStaticText(radiusfilePanel, TEXT_Radius, "Radius file:");
  radiusButton = new wxButton(radiusfilePanel, BUTTON_Radius, "Browse");
  
  std::string default_path = getResourcesDir() + "/radii.txt";
  radiuspathText = new wxTextCtrl(radiusfilePanel, TEXT_Radiuspath, default_path);
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
    // contains input controls for probe 2 (large) radius
    probe2Panel = new wxPanel(framePanel, PANEL_Probe2);
    // contains input controls for grid size
    gridsizePanel = new wxPanel(framePanel, PANEL_Grid);
    // contains input controls for tree depth
    depthPanel = new wxPanel(framePanel, PANEL_Depth);

    InitProbe1Panel();
    InitProbe2Panel();
    InitGridPanel();
    InitDepthPanel();

    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    int dist_inbetween = 4;
    boxSizer->Add(unitCellCheckbox, -1, wxALIGN_LEFT | wxBOTTOM, dist_inbetween);
    boxSizer->Add(surfaceAreaCheckbox, -1, wxALIGN_LEFT | wxTOP | wxBOTTOM, dist_inbetween);
    boxSizer->Add(twoProbesCheckbox, -1, wxALIGN_LEFT | wxTOP | wxBOTTOM, dist_inbetween);
    boxSizer->Add(probe1Panel, -1, wxEXPAND | wxTOP | wxBOTTOM, dist_inbetween);
    boxSizer->Add(probe2Panel, -1, wxEXPAND | wxTOP | wxBOTTOM, dist_inbetween);
    boxSizer->Add(gridsizePanel, -1, wxEXPAND | wxTOP | wxBOTTOM, dist_inbetween);
    boxSizer->Add(depthPanel, -1, wxEXPAND | wxTOP, dist_inbetween);
    framePanel->SetSizerAndFit(boxSizer);
  }

  wxStaticBoxSizer *parameterSizer = new wxStaticBoxSizer(wxVERTICAL,parameterPanel);
  parameterSizer->Add(framePanel, 1, wxEXPAND | wxALL, 5);
  parameterPanel->SetSizerAndFit(parameterSizer);

  return;
}

void MainFrame::InitProbe1Panel(){

  probe1Text = new wxStaticText(probe1Panel, TEXT_Probe1, "Small Probe radius:");
  probe1InputText = new wxTextCtrl(probe1Panel, TEXT_Probe1Input, "1.2");
  probe1UnitText = new wxStaticText(probe1Panel, TEXT_Probe1Unit, L" \u212B"); // unicode for angstrom

  wxBoxSizer *probe1Sizer = new wxBoxSizer(wxHORIZONTAL);
  probe1Sizer->Add(probe1Text,1,wxALIGN_CENTRE_VERTICAL);
  probe1Sizer->Add(probe1InputText, 1, wxALIGN_CENTRE_VERTICAL);
  probe1Sizer->Add(probe1UnitText, 1, wxALIGN_CENTRE_VERTICAL);
  probe1Panel->SetSizerAndFit(probe1Sizer);

  return;
}

void MainFrame::InitProbe2Panel(){

  probe2Text = new wxStaticText(probe2Panel, TEXT_Probe2, "Large Probe radius:");
  probe2InputText = new wxTextCtrl(probe2Panel, TEXT_Probe2Input, "3");
  probe2UnitText = new wxStaticText(probe2Panel, TEXT_Probe2Unit, L" \u212B"); // unicode for angstrom

  wxBoxSizer *probe2Sizer = new wxBoxSizer(wxHORIZONTAL);
  probe2Sizer->Add(probe2Text, 1, wxALIGN_CENTRE_VERTICAL);
  probe2Sizer->Add(probe2InputText, 1, wxALIGN_CENTRE_VERTICAL);
  probe2Sizer->Add(probe2UnitText, 1, wxALIGN_CENTRE_VERTICAL);
  probe2Panel->SetSizerAndFit(probe2Sizer);

  return;
}

void MainFrame::InitGridPanel(){

  gridsizeText = new wxStaticText(gridsizePanel, TEXT_Grid, "Grid resolution:");
  gridsizeInputText = new wxTextCtrl(gridsizePanel, TEXT_Gridinput, "0.1");
  gridsizeUnitText = new wxStaticText(gridsizePanel, TEXT_Gridunit, L" \u212B"); // unicode for angstrom

  wxBoxSizer *gridsizeSizer = new wxBoxSizer(wxHORIZONTAL);
  gridsizeSizer->Add(gridsizeText, 1, wxALIGN_CENTRE_VERTICAL);
  gridsizeSizer->Add(gridsizeInputText, 1, wxALIGN_CENTRE_VERTICAL);
  gridsizeSizer->Add(gridsizeUnitText, 1, wxALIGN_CENTRE_VERTICAL);
  gridsizePanel->SetSizerAndFit(gridsizeSizer);

  return;
}

void MainFrame::InitDepthPanel(){

  depthText = new wxStaticText(depthPanel, TEXT_Depth, "Optimization depth:");
  depthInput = new wxSpinCtrl(depthPanel, SPIN_Depthinput, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTAB_TRAVERSAL, 0, 20, 4);
  depthDummyPanel = new wxPanel(depthPanel, PANEL_DepthDummy, wxDefaultPosition, wxDefaultSize, 0); // to align with above panels

  wxBoxSizer *depthSizer = new wxBoxSizer(wxHORIZONTAL);
  depthSizer->Add(depthText,1,wxALIGN_CENTRE_VERTICAL);
  depthSizer->Add(depthInput,1,wxALIGN_CENTRE_VERTICAL);
  depthSizer->Add(depthDummyPanel,1,wxALIGN_CENTRE_VERTICAL);
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
  atomListSizer->Add(atomListGrid,1,wxEXPAND);
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
  boxSizerV->Add(communicationPanel, 1, wxEXPAND, 0);
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
  boxSizerV->Add(outputPanel, 1, wxTOP | wxEXPAND, 5);
  communicationPanel->SetSizerAndFit(boxSizerV);
}

void MainFrame::InitOutputPanel(){
  outputText = new wxTextCtrl(outputPanel, TEXT_Output, _("Output Dialog"), wxDefaultPosition, wxSize(-1,100), wxTE_MULTILINE | wxTE_READONLY);

  outputGrid = new wxGrid(outputPanel, GRID_Output);
  outputGrid->SetRowLabelSize(0);
  outputGrid->CreateGrid(0, 5, wxGrid::wxGridSelectCells);
  outputGrid->SetDefaultCellAlignment (wxALIGN_CENTRE, wxALIGN_CENTRE);

  // columns
  const std::vector<wxString> col_headers =
    {"Cavity ID",
      "Volume (" + Symbol::angstrom() + Symbol::cubed() + ")",
      "Core Surface\n(" + Symbol::angstrom() + Symbol::squared() + ")",
      "Shell Surface\n(" + Symbol::angstrom() + Symbol::squared() + ")",
      "Position\n("+Symbol::angstrom()+","+Symbol::angstrom()+","+Symbol::angstrom()+")"};
  for (size_t row = 0; row < col_headers.size(); ++row){
    outputGrid->SetColLabelValue(row, col_headers[row]);
    if (row == 0){
      outputGrid->SetColFormatNumber(row);
    }
    else if (row != 4) {
      outputGrid->SetColFormatFloat(row);
      outputGrid->SetColFormatFloat(row, 5, 3);
    }
  }

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
  reportCheckbox = new wxCheckBox(reportExportPanel, CHECKBOX_Report,"Auto export report");

  SetSizerExportSubPanel(reportExportPanel, reportButton, reportCheckbox);
}

void MainFrame::InitTotalMapExportPanel(){
  totalMapButton = new wxButton(totalMapExportPanel, BUTTON_TotalMap, "Export total surface map");
  surfaceMapCheckbox = new wxCheckBox(totalMapExportPanel, CHECKBOX_SurfaceMap, "Auto export total surface map");

  SetSizerExportSubPanel(totalMapExportPanel, totalMapButton, surfaceMapCheckbox);
}

void MainFrame::InitCavityMapExportPanel(){
  cavityMapButton = new wxButton(cavityMapExportPanel, BUTTON_CavityMap, "Export cavity maps");
  cavityMapsCheckbox = new wxCheckBox(cavityMapExportPanel, CHECKBOX_CavityMaps, "Auto export cavity maps");

  SetSizerExportSubPanel(cavityMapExportPanel, cavityMapButton, cavityMapsCheckbox);
}

void MainFrame::InitAutoExportPanel(){
  // these panels serve to align the widgets with the checkboxes
  wxPanel* emptyPanel = new wxPanel(autoExportPanel);
  wxPanel* subPanel = new wxPanel(autoExportPanel);

  {
    outputdirText = new wxStaticText(subPanel, TEXT_Outputdir, "Auto save directory:");
    dirpickerText = new wxTextCtrl(subPanel, TEXT_Dirpicker, "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    dirpickerButton = new wxButton(subPanel, BUTTON_Dirpicker, "Browse");

    SetSizerFilePanel(subPanel, outputdirText, dirpickerButton, dirpickerText);
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

