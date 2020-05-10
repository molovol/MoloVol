#ifndef BASE_H

#define BASE_H

#include <wx/filectrl.h>
#include <wx/wfstream.h>
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
    wxPanel* sandrPanel;
    wxTextCtrl* outputText;
    wxButton* calcButton;

    wxPanel* browsePanel;
      wxPanel* atomfilePanel;
        wxButton* browseButton;
        wxTextCtrl* filepathText;
      wxPanel* radiusfilePanel;
        wxButton* radiusButton;
        wxTextCtrl* radiuspathText;

    // methods to initialise gui
    void InitTopLevel();
    void InitSandr();
    void InitBrowsePanel();
    void InitFilePanel(wxPanel* panel, wxButton* button, wxTextCtrl* text);

    // methods to handle events
    void OnExit(wxCommandEvent& event);
    void OnPrint(wxCommandEvent& event);
    void OnCalc(wxCommandEvent& event);
    //void OnBrowse(wxCommandEvent& event);
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
      TEXT_Radius
};

//DECLARE_APP(MainApp)



#endif
