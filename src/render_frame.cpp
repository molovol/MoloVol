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

#include <array>
#include <unordered_map>

// EVENT TABLE
#define MY_FRAME    101
#define MY_VTK_WINDOW 102

BEGIN_EVENT_TABLE(RenderFrame, wxFrame)
  EVT_MENU(Minimal_Quit,  RenderFrame::OnQuit)
END_EVENT_TABLE()

// DEFINITIONS
RenderFrame::RenderFrame(const wxString& title, const wxPoint& pos, const wxSize& size) : wxFrame((wxFrame *)NULL, -1, title, pos, size)
{

  CreateStatusBar(2);
  wxString mystring;
  mystring << "WxWidgets Version: ";
  mystring << wxMAJOR_VERSION;
  mystring << ".";
  mystring << wxMINOR_VERSION;
  mystring << ".";
  mystring << wxRELEASE_NUMBER;
  mystring << ".";
  mystring << wxSUBRELEASE_NUMBER;
  SetStatusText(mystring,1);
  
  m_pVTKWindow = new wxVTKRenderWindowInteractor(this, MY_VTK_WINDOW);
  m_pVTKWindow->UseCaptureMouseOn(); // TODO: Not sure what this does
  
  ConstructVTK();
  ConfigureVTK();
}

RenderFrame::~RenderFrame()
{
  if(m_pVTKWindow) m_pVTKWindow->Delete();
}

void RenderFrame::ConstructVTK()
{
  colors = vtkSmartPointer<vtkNamedColors>::New(); 
  cylinder = vtkSmartPointer<vtkImageData>::New();
  surface = vtkSmartPointer<vtkMarchingCubes>::New();
  renderer = vtkSmartPointer<vtkRenderer>::New();
  mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  actor = vtkSmartPointer<vtkActor>::New();

}

void RenderFrame::ConfigureVTK()
{
  double isoValue = 0.5;

  // Here we get the renderer window from wxVTK
  renderWindow = m_pVTKWindow->GetRenderWindow();
  renderWindow->AddRenderer(renderer);

  renderer->SetBackground(colors->GetColor3d("DarkSlateGray").GetData());
  renderer->AddActor(actor);

  actor->GetProperty()->SetColor(colors->GetColor3d("MistyRose").GetData());
  actor->SetMapper(mapper);
  
  // Alternative image data
  int lim = 200;
  cylinder->SetDimensions(lim,lim,lim);
  cylinder->AllocateScalars(VTK_INT,1);
  for (size_t i = 0; i < lim; ++i) {
    for (size_t j = 0; j < lim; ++j) {
      // Constructing a cylinder
      bool writeLine = (i-100)*(i-100) + (j-100)*(j-100) < 50*50;
      for (size_t k = 0; k < lim; ++k) {
        bool cap = k > 2 && k < lim-2;

        int* voxel = static_cast<int*>(cylinder->GetScalarPointer(i, j, k));
        *voxel = writeLine && cap? 1 : 0;
      }
    }
  }

  surface->SetInputData(cylinder);
  surface->ComputeNormalsOn();
  surface->SetValue(0, isoValue);

  // The mapper requires our vtkMarchingCubes object
  mapper->SetInputConnection(surface->GetOutputPort());
  mapper->ScalarVisibilityOff();

}

void RenderFrame::UpdateSurface(const Container3D<Voxel>& surf_data){
  std::array<size_t,3> dims = surf_data.getNumElements();
  cylinder->SetDimensions(dims[0],dims[1],dims[2]);
  const std::unordered_map<char,int> typeToNum =
    {{0b00000011, 0},
     {0b00000101, 1},
     {0b00001001, 6},
     {0b00010001, 4},
     {0b00100001, 2},
     {0b01000001, 2}};
  
  for (size_t i = 0; i < dims[0]; ++i) {
    for (size_t j = 0; j < dims[1]; ++j) {
      for (size_t k = 0; k < dims[2]; ++k) {
        int* voxel = static_cast<int*>(cylinder->GetScalarPointer(i,j,k));
        *voxel = typeToNum.find(surf_data.getElement(i,j,k).getType())->second;
      }
    }
  }

  m_pVTKWindow->GetRenderWindow()->Render();
}

void RenderFrame::OnQuit(wxCommandEvent& WXUNUSED(event)) {
  // Only hide window when this window is closed
  Show(false);
}

