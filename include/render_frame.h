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

class wxPanel;
class wxTextCtrl;
class wxStaticText;
class wxCommandEvent;
class wxButton;

template <typename> class Container3D;
class Voxel;

class RenderFrame : public wxFrame {
  public:
    RenderFrame(const wxString&, const wxPoint&, const wxSize&);
    ~RenderFrame();

    void OnClose(wxCloseEvent& event);
    void UpdateSurface(const Container3D<Voxel>&, bool);
    void Render();
  
    vtkSmartPointer<vtkNamedColors> colors;
    vtkSmartPointer<vtkImageData> imagedata;
    vtkSmartPointer<vtkMarchingCubes> surface;
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkRenderWindow> renderWindow;
    vtkSmartPointer<vtkPolyDataMapper> mapper;
    vtkSmartPointer<vtkActor> actor;
  
  private:
    void InitPointerMembers();
    void ConfigureVTK();
    void AdjustControls(bool);
    void ChangeIso(double);

    // Events
    void OnChangeIso(wxCommandEvent&);
    void OnButtonIso(wxCommandEvent&);

    // Members
    bool m_twoProbeMode;
  
    wxVTKRenderWindowInteractor* m_pVTKWindow;
    wxPanel* m_controlPanel;
      wxButton* m_vdwBtn;
      wxButton* m_molBtn;
      wxButton* m_cavityBtn;
      wxButton* m_accessibleBtn;
      wxPanel* m_isoPanel;
        wxStaticText* m_isoText;
        wxTextCtrl* m_isoCtrl;
  
    DECLARE_EVENT_TABLE()
};

enum {
  Minimal_Quit = 1,
  WXVTK_Render,
  PANEL_Control,
  BUTTON_Vdw,
  BUTTON_Mol,
  BUTTON_Cavity,
  BUTTON_Accessible,
  PANEL_Iso,
  TEXT_Iso,
  TEXT_IsoCtrl
};

#endif
