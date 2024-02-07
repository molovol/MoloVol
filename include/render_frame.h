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
template <typename> class Container3D;
class Voxel;

class RenderFrame : public wxFrame {
  public:
    RenderFrame(const wxString&, const wxPoint&, const wxSize&);
    ~RenderFrame();

    void OnClose(wxCloseEvent& event);
    void UpdateSurface(const Container3D<Voxel>&);
  
    vtkSmartPointer<vtkNamedColors> colors;
    vtkSmartPointer<vtkImageData> imagedata;
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
