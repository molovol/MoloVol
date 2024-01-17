#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#  include <wx/wx.h>
#endif

#include "base.h"
#include "special_chars.h"
#include "controller.h"
#include "misc.h"
#include <cassert>

#include <wx/sizer.h>
#include <wx/glcanvas.h>

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
    MainFrame* MainWin = new MainFrame(_("MoloVol " + Ctrl::getVersion()), wxDefaultPosition, wxDefaultSize);
    MainWin->Show(true);
    SetTopWindow(MainWin);


    // Graphics
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    frame = new wxFrame((wxFrame *)NULL, -1,  wxT("Hello GL World"), wxPoint(50,50), wxSize(400,200));

    int args[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0};

    canvas = new VoxelCanvas( (wxFrame*) frame, args);
    sizer->Add(canvas, 1, wxEXPAND);

    frame->SetSizer(sizer);
    frame->SetAutoLayout(true);

    frame->Show();
    return true;

  }
  return true;
};

// EVENT TABLE FOR CANVAS
BEGIN_EVENT_TABLE(VoxelCanvas, wxGLCanvas)
EVT_MOTION(VoxelCanvas::mouseMoved)
EVT_LEFT_DOWN(VoxelCanvas::mouseDown)
EVT_LEFT_UP(VoxelCanvas::mouseReleased)
EVT_RIGHT_DOWN(VoxelCanvas::rightClick)
EVT_LEAVE_WINDOW(VoxelCanvas::mouseLeftWindow)
EVT_SIZE(VoxelCanvas::resized)
EVT_KEY_DOWN(VoxelCanvas::keyPressed)
EVT_KEY_UP(VoxelCanvas::keyReleased)
EVT_MOUSEWHEEL(VoxelCanvas::mouseWheelMoved)
EVT_PAINT(VoxelCanvas::render)
END_EVENT_TABLE()

// CALLBACK FUNCTIONS FOR CANVAS
void VoxelCanvas::mouseMoved(wxMouseEvent& event) {}
void VoxelCanvas::mouseDown(wxMouseEvent& event) {}
void VoxelCanvas::mouseWheelMoved(wxMouseEvent& event) {}
void VoxelCanvas::mouseReleased(wxMouseEvent& event) {}
void VoxelCanvas::rightClick(wxMouseEvent& event) {}
void VoxelCanvas::mouseLeftWindow(wxMouseEvent& event) {}
void VoxelCanvas::keyPressed(wxKeyEvent& event) {}
void VoxelCanvas::keyReleased(wxKeyEvent& event) {}
 
// CANVAS CLASS DEFINITIONS
VoxelCanvas::VoxelCanvas(wxFrame* parent, int* args) :
    wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
  m_context = new wxGLContext(this);

  // prepare a simple cube to demonstrate 3D render
  // source: http://www.opengl.org/resources/code/samples/glut_examples/examples/cube.c
  m_v[0][0] = m_v[1][0] = m_v[2][0] = m_v[3][0] = -1;
  m_v[4][0] = m_v[5][0] = m_v[6][0] = m_v[7][0] = 1;
  m_v[0][1] = m_v[1][1] = m_v[4][1] = m_v[5][1] = -1;
  m_v[2][1] = m_v[3][1] = m_v[6][1] = m_v[7][1] = 1;
  m_v[0][2] = m_v[3][2] = m_v[4][2] = m_v[7][2] = 1;
  m_v[1][2] = m_v[2][2] = m_v[5][2] = m_v[6][2] = -1;    

  // To avoid flashing on MSW
  SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}
 
VoxelCanvas::~VoxelCanvas()
{
	delete m_context;
}

void VoxelCanvas::resized(wxSizeEvent& evt)
{
//	wxGLCanvas::OnSize(evt);
	
    Refresh();
}
 
/** Inits the OpenGL viewport for drawing in 3D. */
void VoxelCanvas::prepare3DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y)
{
	
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glClearDepth(1.0f);	// Depth Buffer Setup
    glEnable(GL_DEPTH_TEST); // Enables Depth Testing
    glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	
    glEnable(GL_COLOR_MATERIAL);
	
    glViewport(topleft_x, topleft_y, bottomrigth_x-topleft_x, bottomrigth_y-topleft_y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	
    float ratio_w_h = (float)(bottomrigth_x-topleft_x)/(float)(bottomrigth_y-topleft_y);
    gluPerspective(45 /*view angle*/, ratio_w_h, 0.1 /*clip close*/, 200 /*clip far*/);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	
}
 
/** Inits the OpenGL viewport for drawing in 2D. */
void VoxelCanvas::prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glEnable(GL_TEXTURE_2D);   // textures
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	
    glViewport(topleft_x, topleft_y, bottomrigth_x-topleft_x, bottomrigth_y-topleft_y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluOrtho2D(topleft_x, bottomrigth_x, bottomrigth_y, topleft_y);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
 
int VoxelCanvas::getWidth()
{
    return GetSize().x;
}
 
int VoxelCanvas::getHeight()
{
    return GetSize().y;
}
 
 
void VoxelCanvas::render( wxPaintEvent& evt )
{
    if(!IsShown()) return;
    
    wxGLCanvas::SetCurrent(*m_context);
    wxPaintDC(this); // only to be used in paint events. use wxClientDC to paint outside the paint event
	
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
    // ------------- draw some 2D ----------------
    prepare2DViewport(0,0,getWidth()/2, getHeight());
    glLoadIdentity();
	
    // white background
    glColor4f(1, 1, 1, 1);
    glBegin(GL_QUADS);
    glVertex3f(0,0,0);
    glVertex3f(getWidth(),0,0);
    glVertex3f(getWidth(),getHeight(),0);
    glVertex3f(0,getHeight(),0);
    glEnd();
	
    // red square
    glColor4f(1, 0, 0, 1);
    glBegin(GL_QUADS);
    glVertex3f(getWidth()/8, getHeight()/3, 0);
    glVertex3f(getWidth()*3/8, getHeight()/3, 0);
    glVertex3f(getWidth()*3/8, getHeight()*2/3, 0);
    glVertex3f(getWidth()/8, getHeight()*2/3, 0);
    glEnd();
    
    // ------------- draw some 3D ----------------
    prepare3DViewport(getWidth()/2,0,getWidth(), getHeight());
    glLoadIdentity();
	
    glColor4f(0,0,1,1);
    glTranslatef(0,0,-5);
    glRotatef(50.0f, 0.0f, 1.0f, 0.0f);
    
    glColor4f(1, 0, 0, 1);
    for (int i = 0; i < 6; i++)
    {
        glBegin(GL_LINE_STRIP);
        glVertex3fv(&m_v[m_faces[i][0]][0]);
        glVertex3fv(&m_v[m_faces[i][1]][0]);
        glVertex3fv(&m_v[m_faces[i][2]][0]);
        glVertex3fv(&m_v[m_faces[i][3]][0]);
        glVertex3fv(&m_v[m_faces[i][0]][0]);
        glEnd();
    }
    
    glFlush();
    SwapBuffers();
}

// MAIN APP DEFINITIONS
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
    elementsButton,
    filepathText,
    elementspathText,
    atomListGrid,
    surfaceAreaCheckbox,
    twoProbesCheckbox,
    probe1DropDown,
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

  elementsfilePanel = new wxPanel(browsePanel, PANEL_Elementsfile);
  atomfilePanel = new wxPanel(browsePanel, PANEL_Atomfile);
  fileOptionsPanel = new wxPanel(browsePanel, PANEL_FileOptions, wxDefaultPosition, wxDefaultSize, 0);

  InitElementsfilePanel();
  InitAtomfilePanel();
  InitFileOptionsPanel();

  wxStaticBoxSizer *browserSizer = new wxStaticBoxSizer(wxVERTICAL,browsePanel);
  browserSizer->Add(elementsfilePanel,0,wxEXPAND | wxTOP,6);
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

// function used in InitAtomfilePanel and InitElementsfilePanel to create and set the sizer
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
  filepathText = new wxTextCtrl(atomfilePanel, TEXT_Filename, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);

  SetSizerFilePanel(atomfilePanel, atomText, browseButton, filepathText);
}

void MainFrame::InitElementsfilePanel(){
  elementsText = new wxStaticText(elementsfilePanel, TEXT_Elements, "Elements file:");
  elementsButton = new wxButton(elementsfilePanel, BUTTON_Elements, "Browse");

  std::string default_path = Ctrl::getDefaultElemPath();
  elementspathText = new wxTextCtrl(elementsfilePanel, TEXT_Elementspath, default_path, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
  SetSizerFilePanel(elementsfilePanel, elementsText, elementsButton, elementspathText);
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
    unitCellCheckbox = new wxCheckBox(framePanel, CHECKBOX_UnitCell, "Analyze crystal unit cell (PDB or CIF file required)");
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

  const wxString drop_down_options[] = {
    "1.2 (Hydrogen)",
    "1.4 (Water)",
    "1.66 (Nitrogen)",
    "1.83 (Argon)",
    "1.86 (N2 sphere rad.)"
  };

  probe1Text = new wxStaticText(probe1Panel, TEXT_Probe1, "Small Probe radius:");
  probe1DropDown = new wxComboBox(probe1Panel, TEXT_Probe1Input, "1.2", wxDefaultPosition, wxDefaultSize,
      sizeof(drop_down_options)/sizeof(drop_down_options[0]), drop_down_options);
  probe1UnitText = new wxStaticText(probe1Panel, TEXT_Probe1Unit, L" \u212B"); // unicode for angstrom

  wxBoxSizer *probe1Sizer = new wxBoxSizer(wxHORIZONTAL);
  probe1Sizer->Add(probe1Text,1,wxALIGN_CENTRE_VERTICAL);
  probe1Sizer->Add(probe1DropDown, 1, wxALIGN_CENTRE_VERTICAL);
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
  gridsizeInputText = new wxTextCtrl(gridsizePanel, TEXT_Gridinput, "0.2");
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
  // the width of 420 pixels gives enough room for the vertical scroll bar when multiple cavities are listed
  // with default width, the vertical scroll bar hides a part of the grid thus generating a horizontal scroll bar
  outputText = new wxTextCtrl(outputPanel, TEXT_Output, _("Output Dialog"), wxDefaultPosition, wxSize(420,100), wxTE_MULTILINE | wxTE_READONLY);

  outputGrid = new wxGrid(outputPanel, GRID_Output);
  outputGrid->CreateGrid(0, 0);
  outputGrid->SetRowLabelSize(0);
  outputGrid->SetDefaultCellAlignment (wxALIGN_CENTRE, wxALIGN_CENTRE);

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

