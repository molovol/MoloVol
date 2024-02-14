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
    void AdjustControls(bool);
    void ChangeIso(double);

    // Events
    void OnChangeIso(wxCommandEvent&);
    void OnButtonClick(wxCommandEvent&);

    // Init
    void InitIsoInputPanel();
    void InitIsoControlPanel();
    void InitControlPanel();
    void InitPointerMembers();
    void InitRenderWindow();

    // Members
    bool m_twoProbeMode;
  
    wxVTKRenderWindowInteractor* m_pVTKWindow;
    wxPanel* m_controlPanel;
      wxPanel* m_isoCtrlPanel;
        wxButton* m_vdwBtn;
        wxButton* m_molBtn;
        wxButton* m_cavityBtn;
        wxButton* m_accessibleBtn;
        wxPanel* m_isoInputPanel;
          wxStaticText* m_isoText;
          wxTextCtrl* m_isoCtrl;
        wxButton* m_resetCameraBtn;
  
    DECLARE_EVENT_TABLE()
};

enum {
  Minimal_Quit = wxID_HIGHEST+1,
  WXVTK_Render,
  PANEL_Control,
  PANEL_IsoCtrl,
  BUTTON_Vdw,
  BUTTON_Mol,
  BUTTON_Cavity,
  BUTTON_Accessible,
  PANEL_IsoInput,
  TEXT_Iso,
  TEXT_IsoCtrl,
  BUTTON_ResetCamera
};

#endif
