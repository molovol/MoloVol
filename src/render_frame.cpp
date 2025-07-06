#include "render_frame.h"
#include "container3d.h"
#include "voxel.h"
#include "base.h"
#include "atomtree.h"

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
#include <vtkMolecule.h>
#include <vtkMoleculeMapper.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>

// wxWidgets
#include <wx/spinctrl.h>
#include <wx/event.h>
#include <wx/button.h>
#include <wx/listbox.h>
#include <wx/arrstr.h>
#include <wx/dynarray.h>
#include <wx/sizer.h>
#include <wx/slider.h>

#include <array>
#include <unordered_map>
#include <string> 

#define HIDEMOLLABEL "Hide Molecule"
#define SHOWMOLLABEL "Show Molecule"
#define BORDER 2

// EVENT TABLE
BEGIN_EVENT_TABLE(RenderFrame, wxFrame)
  EVT_CLOSE(RenderFrame::OnClose)
  EVT_TEXT_ENTER(TEXT_IsoCtrl, RenderFrame::OnChangeIso)
  EVT_BUTTON(wxID_ANY, RenderFrame::OnButtonClick)
  EVT_LISTBOX(LIST_Cavity, RenderFrame::OnCavitySelect)
  EVT_SCROLL(RenderFrame::OnOpacitySlide)
END_EVENT_TABLE()

// DEFINITIONS
// Constructor is called when the render window is initiated by the main window.
// Initialises the render frame and control widgets.
RenderFrame::RenderFrame(const MainFrame* parent, const wxString& title, const wxPoint& pos, const wxSize& size) 
  : wxFrame((wxFrame *)NULL, -1, title, pos, size), m_parentWindow(parent) {

#if defined(_WIN32)
  SetIcon(wxICON(aaaa));
#endif

  // Create wxVTK window interactor
  m_pVTKWindow = new wxVTKRenderWindowInteractor(this, WXVTK_Render, wxDefaultPosition, wxSize(400,400));
  m_pVTKWindow->UseCaptureMouseOn(); // Mouse motion is captured outside of window
  
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

void RenderFrame::UpdateSurface(const Container3D<Voxel>& surf_data, const std::array<double,3> origin, 
    const double grid_step, const bool probe_mode, const unsigned char n_cavities){
  // Set up image
  std::array<unsigned long,3> dims = surf_data.getNumElements();

  imagedata->PrepareForNewData();
  imagedata->SetDimensions(dims[0],dims[1],dims[2]);
  imagedata->SetOrigin(origin[0]+grid_step/2, origin[1]+grid_step/2, origin[2]+grid_step/2);
  imagedata->SetSpacing(grid_step, grid_step, grid_step);
  // Sets the type of the scalar
  imagedata->AllocateScalars(VTK_CHAR,1);

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
        unsigned char* voxel = static_cast<unsigned char*>(imagedata->GetScalarPointer(i,j,k));
        *voxel = (unsigned char)typeToNum.find(surf_data.getElement(i,j,k).getType())->second;
      }
    }
  }
  ClearMask();

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
  m_cavityList->Clear();
  m_cavityList->InsertItems(list_items, 0);

}

void RenderFrame::UpdateMolecule(const std::vector<Atom>& atomlist) {
  
  const AtomTree atomtree(atomlist);
  const std::vector<Atom>& all_atoms = atomtree.getAtomList();

  molecule->Initialize();

  vtkSmartPointer<vtkFloatArray> radii = vtkSmartPointer<vtkFloatArray>::New();
  radii->SetName("radii");
  radii->SetNumberOfComponents(1);
  radii->SetNumberOfTuples(all_atoms.size());

  std::vector<vtkAtom> atom_objs;
  for (size_t i = 0; i < all_atoms.size(); ++i) {
    const Atom& at = all_atoms[i];
    atom_objs.push_back(molecule->AppendAtom(at.number, at.pos_x, at.pos_y, at.pos_z));
    radii->SetValue(i, at.rad);
  }

  for (size_t at_id = 0; at_id < all_atoms.size(); ++at_id) {
    const Atom& at = all_atoms[at_id];
    std::vector<size_t> closest = atomtree.listAllWithin(at.getPos(), 0);

    for (const size_t nb_id : closest) {
      if (nb_id != at_id) {
        molecule->AppendBond(atom_objs[at_id], atom_objs[nb_id], 1);
      }
    }
  }
  
  molecule->GetVertexData()->AddArray(radii);

  molactor->SetVisibility(true);
  m_hideMolBtn->SetLabel(HIDEMOLLABEL);
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

  m_vdwBtn->Enable();
  m_cavityBtn->Enable();
  m_molBtn->SetLabel("Molecular Surface");
  m_accessibleBtn->SetLabel("Probe Accessible Surface");

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

void RenderFrame::ClearMask() {
  int dims[3];
  imagedata->GetDimensions(dims);

  maskdata->PrepareForNewData();
  maskdata->SetDimensions(dims[0],dims[1],dims[2]);
  // Sets the type of the scalar
  maskdata->AllocateScalars(VTK_UNSIGNED_CHAR,1);

  // TODO: Parallelise
  for (size_t i = 0; i < dims[0]; ++i) {
    for (size_t j = 0; j < dims[1]; ++j) {
      for (size_t k = 0; k < dims[2]; ++k) {
        unsigned char* voxel = static_cast<unsigned char*>(maskdata->GetScalarPointer(i,j,k));
        *voxel = 1; 
      }
    }
  }
}

//////////
// INIT //
//////////
void RenderFrame::InitIsoControlPanel() {
  m_isoCtrlPanel = new wxPanel(m_controlPanel, PANEL_IsoCtrl);

  m_vdwBtn = new wxButton(m_isoCtrlPanel, BUTTON_Vdw, "Van der Waals-Surface");
  m_molBtn = new wxButton(m_isoCtrlPanel, BUTTON_Mol, "Molecular Surface");
  m_cavityBtn = new wxButton(m_isoCtrlPanel, BUTTON_Cavity, "Cavity Surfaces");
  m_accessibleBtn = new wxButton(m_isoCtrlPanel, BUTTON_Accessible, "Probe Accessible Surface");

  InitIsoInputPanel();

  {
    wxStaticBoxSizer* controlSizer = new wxStaticBoxSizer(wxVERTICAL, m_isoCtrlPanel, "Surface Select");
    controlSizer->Add(m_vdwBtn, 0, wxEXPAND | wxBOTTOM, BORDER);
    controlSizer->Add(m_molBtn, 0, wxEXPAND | wxBOTTOM, BORDER);
    controlSizer->Add(m_cavityBtn, 0, wxEXPAND | wxBOTTOM, BORDER);
    controlSizer->Add(m_accessibleBtn, 0, wxEXPAND | wxBOTTOM, BORDER);
    controlSizer->Add(m_isoInputPanel, 0, wxEXPAND);
    m_isoCtrlPanel->SetSizerAndFit(controlSizer);
  }
}

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

void RenderFrame::InitControlPanel() {
  m_controlPanel = new wxPanel(this, PANEL_Control, wxDefaultPosition, wxSize(200,-1));
 
  InitIsoControlPanel();

  wxPanel* camera_panel = new wxPanel(m_controlPanel);
  m_resetCameraBtn = new wxButton(camera_panel, BUTTON_ResetCamera, "Center Camera");
  wxStaticText* opacity_label = new wxStaticText(camera_panel, wxID_ANY, "Opacity [%]");
  m_opacitySlider = new wxSlider(camera_panel, SLIDER_Opacity, 60, 0, 100);
  wxStaticBoxSizer* camera_sizer = new wxStaticBoxSizer(wxVERTICAL, camera_panel, "View Control");
  camera_sizer->Add(opacity_label, 0, wxEXPAND);
  camera_sizer->Add(m_opacitySlider, 0, wxEXPAND | wxBOTTOM, BORDER);
  camera_sizer->Add(m_resetCameraBtn, 0, wxEXPAND);
  camera_panel->SetSizerAndFit(camera_sizer);
  
  InitMolPanel();

  wxPanel* cavity_panel = new wxPanel(m_controlPanel);
  m_cavityList = new wxListBox(cavity_panel, LIST_Cavity, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_EXTENDED);
  wxStaticBoxSizer* cavity_sizer = new wxStaticBoxSizer(wxVERTICAL, cavity_panel, "Cavity Select");
  cavity_sizer->Add(m_cavityList, 0, wxEXPAND);
  cavity_panel->SetSizerAndFit(cavity_sizer);
  
  {
    wxBoxSizer* controlSizer = new wxBoxSizer(wxVERTICAL);
    controlSizer->Add(m_isoCtrlPanel, 0, wxEXPAND);
    controlSizer->Add(camera_panel, 0, wxEXPAND);
    controlSizer->Add(m_molPanel, 0, wxEXPAND);
    controlSizer->Add(cavity_panel, 0, wxEXPAND);
    m_controlPanel->SetSizerAndFit(controlSizer);
  }
}

void RenderFrame::InitMolPanel() {
  m_molPanel = new wxPanel(m_controlPanel, PANEL_Mol);
  m_hideMolBtn = new wxButton(m_molPanel, BUTTON_HideMol, HIDEMOLLABEL);
  m_liquoriceModelBtn = new wxButton(m_molPanel, BUTTON_LiquoriceModel, "Liquorice Stick");
  m_sticksnballsModelBtn = new wxButton(m_molPanel, BUTTON_SticksNBallsModel, "Sticks and Balls");
  m_vdwModelBtn = new wxButton(m_molPanel, BUTTON_VdwModel, "Van der Waals-Radii");

  wxStaticBoxSizer* hSizer = new wxStaticBoxSizer(wxVERTICAL, m_molPanel, "Molecule Render");
  hSizer->Add(m_hideMolBtn, 0, wxEXPAND | wxBOTTOM, BORDER);
  hSizer->Add(m_liquoriceModelBtn, 0, wxEXPAND | wxBOTTOM, BORDER);
  hSizer->Add(m_sticksnballsModelBtn, 0, wxEXPAND | wxBOTTOM, BORDER);
  hSizer->Add(m_vdwModelBtn, 0, wxEXPAND);
  m_molPanel->SetSizerAndFit(hSizer);
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
  molecule = vtkSmartPointer<vtkMolecule>::New();
  molmapper = vtkSmartPointer<vtkMoleculeMapper>::New();
  molactor = vtkSmartPointer<vtkActor>::New();
}

void RenderFrame::InitRenderWindow() {

  // Here we get the renderer window from wxVTK then add the renderer
  renderWindow = m_pVTKWindow->GetRenderWindow();
  renderWindow->AddRenderer(renderer);

  // Adding the actor to the renderer
  renderer->SetBackground(0,0,0);
  renderer->AddActor(actor);

  // Adding the mapper to the renderer
  actor->GetProperty()->SetColor(250/(double)255, 224/(double)255, 135/(double)255);
  actor->GetProperty()->SetOpacity(0.6);
  actor->SetMapper(mapper);
 
  // Adding marching cubes data to mapper
  mapper->SetInputConnection(surface->GetOutputPort());
  mapper->ScalarVisibilityOff();

  // Add image data to marching cube object
  surface->SetInputConnection(imagemask->GetOutputPort());
  surface->ComputeNormalsOn();

  molecule->Initialize();
  molmapper->SetInputData(molecule);
  molmapper->UseLiquoriceStickSettings();
  molactor->SetMapper(molmapper);
  renderer->AddActor(molactor);
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

void RenderFrame::OnOpacitySlide(wxScrollEvent& e) {
  double op = (double)m_opacitySlider->GetValue()/(double)m_opacitySlider->GetMax();
  actor->GetProperty()->SetOpacity(op);
  Render();
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
    case BUTTON_HideMol:
      molactor->SetVisibility(m_hideMolBtn->GetLabel() != HIDEMOLLABEL);
      Render();
      m_hideMolBtn->SetLabel(m_hideMolBtn->GetLabel() == HIDEMOLLABEL? SHOWMOLLABEL : HIDEMOLLABEL);
      break;
    case BUTTON_LiquoriceModel:
      molmapper->UseLiquoriceStickSettings();
      molactor->SetVisibility(true);
      m_hideMolBtn->SetLabel(HIDEMOLLABEL);
      Render();
      break;
    case BUTTON_SticksNBallsModel:
      molmapper->UseBallAndStickSettings();
      molactor->SetVisibility(true);
      m_hideMolBtn->SetLabel(HIDEMOLLABEL);
      Render();
      break;
    case BUTTON_VdwModel:
      molmapper->SetAtomicRadiusTypeToCustomArrayRadius();
      molactor->SetVisibility(true);
      m_hideMolBtn->SetLabel(HIDEMOLLABEL);
      Render();
      break;
  }
}

void RenderFrame::OnCavitySelect(wxCommandEvent& event) {
  auto surf_data = m_parentWindow->getSurfaceData();
  maskdata->PrepareForNewData();
  
  // Get a vector of selected cavity IDs
  wxArrayInt selection;
  int n_selections = m_cavityList->GetSelections(selection);
  std::vector<unsigned char> idx_list;
  for (size_t i = 0; i < n_selections; ++i) {
    idx_list.push_back(selection.Item(i));
  }
  // Return if no item has been selected
  if (idx_list.empty()) {return;}

  // If "Full Map" was selected, show entire surface map
  if (idx_list[0] == 0) {
    for (size_t i = 0; i<n_selections; ++i) {
      m_cavityList->Deselect(idx_list[i]);
    }
    m_cavityList->SetSelection(0);
    ClearMask();
    
    m_vdwBtn->Enable();
    m_cavityBtn->Enable();
    m_molBtn->SetLabel("Molecular Surface");
    m_accessibleBtn->SetLabel("Probe Accessible Surface");
  }
  else {
    // Get size of image data
    std::array<unsigned long,3> dims = surf_data.getNumElements();
    maskdata->SetDimensions(dims[0],dims[1],dims[2]);
    maskdata->AllocateScalars(VTK_UNSIGNED_CHAR,1);

    for (size_t i = 0; i < dims[0]; ++i) {
      for (size_t j = 0; j < dims[1]; ++j) {
        for (size_t k = 0; k < dims[2]; ++k) {
          unsigned char* voxel = static_cast<unsigned char*>(maskdata->GetScalarPointer(i,j,k));
          *voxel = (unsigned char)(
              std::binary_search(idx_list.begin(), idx_list.end(), surf_data.getElement(i,j,k).getID())? 1 : 0
            );
        }
      }
    }

    m_vdwBtn->Disable();
    m_cavityBtn->Disable();
    m_molBtn->SetLabel("Cavity Shell Surface");
    m_accessibleBtn->SetLabel("Cavity Core Surface");
  }

  // Apply mask to image data
  imagemask->SetImageInputData(imagedata);
  imagemask->SetMaskInputData(maskdata);
  imagemask->SetMaskedOutputValue(1);
  Render();
}

void RenderFrame::OnClose(wxCloseEvent& WXUNUSED(event)){
  Show(false);
}

