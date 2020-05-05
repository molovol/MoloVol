#ifndef BASE_H

#define BASE_H

#include <wx/filectrl.h>
#include <wx/wfstream.h>

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
    wxButton* browseButton;
    wxTextCtrl* filepathText;

    // methods to initialise gui
    void InitTopLevel();
    void InitSandr();
    void InitBrowsePanel();

    // methods to handle events
    void OnExit(wxCommandEvent& event);
    void OnPrint(wxCommandEvent& event);
    void OnCalc(wxCommandEvent& event);
    void OnBrowse(wxCommandEvent& event);
    
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
  BUTTON_Browse,
  TEXT_Filename,
  PANEL_Browse
};

//DECLARE_APP(MainApp)



#endif
