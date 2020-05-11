#ifndef BASE_H

#define BASE_H

#include <wx/filectrl.h>
#include <wx/wfstream.h>
#include <wx/spinctrl.h>
#include <iostream>

class MainApp: public wxApp
{
  public:
    virtual bool OnInit();
};

class MainFrame: public wxFrame
{
  public:
    // methods for controller communication
    void clearOutput();
    void printToOutput(std::string& text);
    void appendOutput(std::string& text);

    MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size);
      wxPanel* browsePanel;
        wxPanel* atomfilePanel;
          wxButton* browseButton;
          wxTextCtrl* filepathText;
        wxPanel* radiusfilePanel;
          wxButton* radiusButton;
          wxTextCtrl* radiuspathText;
     // 
      wxPanel* parameterPanel;
        wxPanel* gridsizePanel;
          wxStaticText* gridsizeText;
          wxPanel* gridsizeInputPanel;
            wxTextCtrl* gridsizeInputText;
            wxStaticText* gridsizeUnitText; // possibly usable universally?
        wxPanel* depthPanel;
          wxStaticText* depthText;
          wxSpinCtrl* depthInput;
     //
      wxPanel* sandrPanel;
        wxTextCtrl* outputText;
        wxButton* calcButton;

    // methods to initialise gui
    void InitTopLevel();
    void InitSandr();
    void InitBrowsePanel();
    void InitAtomfilePanel(); 
    void InitRadiusfilePanel();  
    void InitFilePanel(wxPanel* panel, wxButton* button, wxTextCtrl* text);
    //
    void InitParametersPanel();
    void InitGridPanel();
    void InitGridinputPanel();
    void InitDepthPanel();
    //
    // methods to handle events
    void OnExit(wxCommandEvent& event);
    void OnPrint(wxCommandEvent& event);
    void OnCalc(wxCommandEvent& event);
    void OnAtomBrowse(wxCommandEvent& event);
    void OnRadiusBrowse(wxCommandEvent& event);
    void OnBrowse(std::string& filetype, wxTextCtrl* textbox);
    
    DECLARE_EVENT_TABLE()

    // other methods

};

enum
{
  // assign an ID to the button Hello
  TEXT_Output = wxID_HIGHEST + 1,
    BUTTON_Calc,
    PANEL_Sandr,
  //FILE_Browse,
  PANEL_Browse,
    PANEL_Atomfile,
      BUTTON_Browse,
      TEXT_Filename,
    PANEL_Radiusfile,
      BUTTON_Radius,
      TEXT_Radius,
//
  PANEL_Parameters,
    PANEL_Grid,
      TEXT_Grid,
      PANEL_Gridinput,
        TEXT_Gridinput,
        TEXT_Gridunit,
    PANEL_Depth,
      TEXT_Depth,
      SPIN_Depthinput
//
};

//DECLARE_APP(MainApp)

#endif
