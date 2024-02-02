#ifndef RENDER_FRAME_H

#define RENDER_FRAME_H

// Custom library
#include "wxVTKRenderWindowInteractor.h"

// wxWidgets
#include <wx/wx.h>

// VTK
#include <vtkSmartPointer.h>

// Standard library
#include <stdlib.h>

class vtkNamedColors;
class vtkImageData;
class vtkMarchingCubes;
class vtkRenderer;
class vtkRenderWindow;
class vtkPolyDataMapper;
class vtkActor;

class RenderFrame : public wxFrame {
  public:
    RenderFrame(const wxString&, const wxPoint&, const wxSize&);
    ~RenderFrame();

    void OnQuit(wxCommandEvent& event);
  
    vtkSmartPointer<vtkNamedColors> colors;
    vtkSmartPointer<vtkImageData> cylinder;
    vtkSmartPointer<vtkVoxelModeller> voxelModeller;
    vtkSmartPointer<vtkMarchingCubes> surface;
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkRenderWindow> renderWindow;
    vtkSmartPointer<vtkPolyDataMapper> mapper;
    vtkSmartPointer<vtkActor> actor;
  
  private:
    void ConstructVTK();
    void ConfigureVTK();
  
    wxVTKRenderWindowInteractor* m_pVTKWindow;
  
  private:
    DECLARE_EVENT_TABLE()
};

enum {
  Minimal_Quit = 1,
};

#endif
