#include "render_frame.h"
#include "container3d.h"
#include "voxel.h"

// VTK
#include <vtkActor.h>
#include <vtkImageData.h>
#include <vtkNamedColors.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkMarchingCubes.h>
#include <vtkImageMask.h>

// wxWidgets
#include <wx/spinctrl.h>
#include <wx/event.h>
#include <wx/button.h>
#include <wx/listbox.h>
#include <wx/arrstr.h>

#include <array>
#include <unordered_map>
#include <string> 

// EVENT TABLE
BEGIN_EVENT_TABLE(RenderFrame, wxFrame)
  EVT_CLOSE(RenderFrame::OnClose)
  EVT_TEXT_ENTER(TEXT_IsoCtrl, RenderFrame::OnChangeIso)
  EVT_BUTTON(wxID_ANY, RenderFrame::OnButtonClick)
END_EVENT_TABLE()

// DEFINITIONS
// Constructor is called when the render window is initiated by the main window.
// Initialises the render frame and control widgets.
RenderFrame::RenderFrame(const wxString& title, const wxPoint& pos, const wxSize& size) 
  : wxFrame((wxFrame *)NULL, -1, title, pos, size) {

  // Create wxVTK window interactor
  m_pVTKWindow = new wxVTKRenderWindowInteractor(this, WXVTK_Render, wxDefaultPosition, wxSize(400,400));
  m_pVTKWindow->UseCaptureMouseOn(); // TODO: Not sure what this does
  
  InitControlPanel();

  // Top level horizontal box sizer, contains render frame and the control panel
  {
    wxBoxSizer* topLvlSizer = new wxBoxSizer(wxHORIZONTAL);
    topLvlSizer->Add(m_pVTKWindow, 1, wxEXPAND);
    topLvlSizer->Add(m_controlPanel, 0, wxEXPAND);
    this->SetSizerAndFit(topLvlSizer);
  }

  // Render window
  InitPointerMembers();
  InitRenderWindow();

}

RenderFrame::~RenderFrame()
{
  if(m_pVTKWindow) m_pVTKWindow->Delete();
}

////////////////////
// PUBLIC METHODS //
////////////////////

void RenderFrame::UpdateSurface(const Container3D<Voxel>& surf_data, const bool probe_mode, const unsigned char n_cavities){
  // Set up image
  std::array<size_t,3> dims = surf_data.getNumElements();

  imagedata->Initialize();
  imagedata->SetDimensions(dims[0],dims[1],dims[2]);
  // Sets the type of the scalar
  imagedata->AllocateScalars(VTK_INT,1);

  maskdata->Initialize();
  maskdata->SetDimensions(dims[0],dims[1],dims[2]);
  // Sets the type of the scalar
  maskdata->AllocateScalars(VTK_UNSIGNED_CHAR,1);

  // TODO: Make this a static member of this class
  const std::unordered_map<char,int> typeToNum =
    {{0b00000011, 0},
     {0b00000101, 1},
     {0b00001001, 6},
     {0b00010001, 4},
     {0b00100001, 2},
     {0b01000001, 2}};
  // Copy image data
  // TODO: Parallelise
  for (size_t i = 0; i < dims[0]; ++i) {
    for (size_t j = 0; j < dims[1]; ++j) {
      for (size_t k = 0; k < dims[2]; ++k) {
        int* voxel = static_cast<int*>(imagedata->GetScalarPointer(i,j,k));
        *voxel = (int)typeToNum.find(surf_data.getElement(i,j,k).getType())->second;

        voxel = static_cast<int*>(maskdata->GetScalarPointer(i,j,k));
        *voxel = (int)(surf_data.getElement(i,j,k).getID() == 2 ? 1 : 0);
      }
    }
  }

  // Apply mask to image data
  imagemask->SetImageInputData(imagedata);
  imagemask->SetMaskInputData(maskdata);
  imagemask->SetMaskedOutputValue(1);

  // Set initial iso value and set iso value in text field
  surface->SetValue(0, 0.5);
  m_isoCtrl->SetValue("0.5");
  
  renderer->ResetCamera();

  AdjustControls(probe_mode);

  wxArrayString list_items;
  list_items.Add("Full Map");
  for (size_t i = 0; i < n_cavities; ++i) {
    list_items.Add("Cavity #" + std::to_string(i+1));
  }
  m_cavityList->InsertItems(list_items, 0);

}

void RenderFrame::Render() {
  renderWindow->Render();  
}

/////////////////////
// PRIVATE METHODS //
/////////////////////

// Adjust GUI elements based on the probe mode
void RenderFrame::AdjustControls(bool probe_mode) {
  m_twoProbeMode = probe_mode;
  if (probe_mode) {
    m_cavityBtn->Show();
  }
  else {
    m_cavityBtn->Hide();
  }
  Layout();
}

void RenderFrame::ChangeIso(double value) {
  // Set value to UI
  std::string str = std::to_string(value);
  // Remove trailing zeros
  str = str.substr(0, str.find_last_not_of('0')+1); 
  if(str.find('.') == str.size()-1) {
    // Remove decimal point
    str = str.substr(0, str.size()-1);
  }
  m_isoCtrl->SetValue(str);
  // Set iso in renderer
  surface->SetValue(0,value);
  Render();
}

//////////
// INIT //
//////////
void RenderFrame::InitIsoInputPanel() {
  m_isoInputPanel = new wxPanel(m_isoCtrlPanel, PANEL_IsoInput);

  m_isoText = new wxStaticText(m_isoInputPanel, TEXT_Iso, "Iso Value");
  m_isoCtrl = new wxTextCtrl(m_isoInputPanel, TEXT_IsoCtrl, "0.5", wxDefaultPosition, wxDefaultSize,
      wxTE_PROCESS_ENTER);

  {
    wxBoxSizer* isoSizer = new wxBoxSizer(wxHORIZONTAL);
    isoSizer->Add(m_isoText, 1);
    isoSizer->Add(m_isoCtrl, 1);
    m_isoInputPanel->SetSizerAndFit(isoSizer);
  }
}

void RenderFrame::InitIsoControlPanel() {
  m_isoCtrlPanel = new wxPanel(m_controlPanel, PANEL_IsoCtrl);

  m_vdwBtn = new wxButton(m_isoCtrlPanel, BUTTON_Vdw, "Van der Waals-Surface");
  m_molBtn = new wxButton(m_isoCtrlPanel, BUTTON_Mol, "Molecular Surface");
  m_cavityBtn = new wxButton(m_isoCtrlPanel, BUTTON_Cavity, "Cavity Surfaces");
  m_accessibleBtn = new wxButton(m_isoCtrlPanel, BUTTON_Accessible, "Probe Accessible Surface");

  InitIsoInputPanel();

  {
    wxBoxSizer* controlSizer = new wxBoxSizer(wxVERTICAL);
    controlSizer->Add(m_vdwBtn, 0, wxEXPAND);
    controlSizer->Add(m_molBtn, 0, wxEXPAND);
    controlSizer->Add(m_cavityBtn, 0, wxEXPAND);
    controlSizer->Add(m_accessibleBtn, 0, wxEXPAND);
    controlSizer->Add(m_isoInputPanel, 0, wxEXPAND);
    m_isoCtrlPanel->SetSizerAndFit(controlSizer);
  }
}

void RenderFrame::InitControlPanel() {
  m_controlPanel = new wxPanel(this, PANEL_Control, wxDefaultPosition, wxSize(200,-1));
 
  InitIsoControlPanel();

  m_resetCameraBtn = new wxButton(m_controlPanel, BUTTON_ResetCamera, "Center Camera");

  m_cavityList = new wxListBox(m_controlPanel, LIST_Cavity, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_EXTENDED);
  
  {
    wxBoxSizer* controlSizer = new wxBoxSizer(wxVERTICAL);
    controlSizer->Add(m_isoCtrlPanel, 0, wxEXPAND);
    controlSizer->Add(m_resetCameraBtn, 0, wxEXPAND);
    controlSizer->Add(m_cavityList, 0, wxEXPAND);
    m_controlPanel->SetSizerAndFit(controlSizer);
  }
}

void RenderFrame::InitPointerMembers()
{
  colors = vtkSmartPointer<vtkNamedColors>::New(); 
  imagedata = vtkSmartPointer<vtkImageData>::New();
  maskdata = vtkSmartPointer<vtkImageData>::New();
  imagemask = vtkSmartPointer<vtkImageMask>::New();
  surface = vtkSmartPointer<vtkMarchingCubes>::New();
  renderer = vtkSmartPointer<vtkRenderer>::New();
  mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  actor = vtkSmartPointer<vtkActor>::New();
}

void RenderFrame::InitRenderWindow() {

  // Here we get the renderer window from wxVTK then add the renderer
  renderWindow = m_pVTKWindow->GetRenderWindow();
  renderWindow->AddRenderer(renderer);

  // Adding the actor to the renderer
  renderer->SetBackground(colors->GetColor3d("DarkSlateGray").GetData());
  renderer->AddActor(actor);

  // Adding the mapper to the renderer
  actor->GetProperty()->SetColor(colors->GetColor3d("MistyRose").GetData());
  actor->SetMapper(mapper);
 
  // Adding marching cubes data to mapper
  mapper->SetInputConnection(surface->GetOutputPort());
  mapper->ScalarVisibilityOff();

  // Add image data to marching cube object
  surface->SetInputConnection(imagemask->GetOutputPort());
  surface->ComputeNormalsOn();
}

////////////////////
// EVENT HANDLERS //
////////////////////
void RenderFrame::OnChangeIso(wxCommandEvent& e) {
  auto number = e.GetString();
  double value;
  if(!number.ToDouble(&value)){
    ChangeIso(0);
  }
  else {
    ChangeIso(value);
  }
}

void RenderFrame::OnButtonClick(wxCommandEvent& event) {

  switch (event.GetId()) {
    case BUTTON_Vdw:
      ChangeIso(0.5);
      break;
    case BUTTON_Mol:
      ChangeIso(m_twoProbeMode? 1.5 : 2.0);
      break;
    case BUTTON_Cavity:
      ChangeIso(3.0);
      break;
    case BUTTON_Accessible:
      ChangeIso(5.0);
      break;
    case BUTTON_ResetCamera:
      renderer->ResetCamera();
      Render();
      break;
  }

}

void RenderFrame::OnClose(wxCloseEvent& WXUNUSED(event)){
  Show(false);
}

