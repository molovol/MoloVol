#ifndef BASE_H

#define BASE_H

#include <wx/filectrl.h>
#include <wx/wfstream.h>
#include <wx/spinctrl.h>
#include <wx/grid.h>
#include <wchar.h>
#include <string>
#include <iostream>
#include <vector>
#include <tuple>
#include <unordered_map>

class MainApp: public wxApp
{
  public:
    virtual bool OnInit();

  private:
    // colours
    wxColour col_win = wxColour(160,160,160);
};

class MainFrame: public wxFrame
{
  public:

    // methods for controller communication
    void clearOutput();
    void printToOutput(std::string& text);
    void appendOutput(std::string& text);
    std::string getAtomFilepath();
    std::string getRadiusFilepath();
    double getGridsize();
    int getDepth();
    double getProbeRadius();
    void enableGuiElements(bool inp); // method to turn on and off gui elements upon start and completion of calc
    //
    void displayAtomList(std::vector<std::tuple<std::string, int, double>>& symbol_number_radius);
    std::string generateChemicalFormulaFromGrid();
    std::unordered_map<std::string, double> generateRadiusMapFromView();
    MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size);

  private:

    double r_probe = 1.5; //* hard coded for testing purposes. eventually obtain from user input

    wxPanel* leftMainPanel;
    wxPanel* rightMainPanel;
     //
      wxPanel* browsePanel;
        wxPanel* atomfilePanel;
          wxButton* browseButton;
          wxTextCtrl* filepathText;
        wxPanel* radiusfilePanel;
          wxButton* radiusButton;
          wxTextCtrl* radiuspathText;
        wxPanel* fileOptionsPanel;
          wxButton* loadFilesButton;
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
      wxPanel* atomListPanel;
        wxGrid* atomListGrid;

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
    void InitFileOptionsPanel();
    void InitFilePanel(wxPanel* panel, wxButton* button, wxTextCtrl* text);
    void InitParametersPanel();
    void InitGridPanel();
    void InitGridinputPanel();
    void InitDepthPanel();
    void InitAtomListPanel();
    void FitSizes();
    // methods to handle events
    void OnExit(wxCommandEvent& event);
    void OnPrint(wxCommandEvent& event);
    void OnCalc(wxCommandEvent& event);
    void OnAtomBrowse(wxCommandEvent& event);
    void OnRadiusBrowse(wxCommandEvent& event);
    void OnLoadFiles(wxCommandEvent& event);
    void OnBrowse(std::string& filetype, wxTextCtrl* textbox);
    void GridChange(wxGridEvent& event);

    // colours
    wxColour col_panel = wxColour(160,160,160);
    wxColour col_output = wxColour(192,192,192);
    wxColour col_white = wxColour(255,255,255);
    wxColour col_grey_cell = wxColour(150,150,150);
    wxColour col_red_cell = wxColour(255,75,75);
    wxColour col_cyan_cell = wxColour(120,255,255);

    DECLARE_EVENT_TABLE()

    // other methods

};

enum
{
  PANEL_RightMain,
  PANEL_LeftMain,
  // assign an ID to the button Start
  TEXT_Output = wxID_HIGHEST + 1,
    BUTTON_Calc,
    PANEL_Sandr,
  PANEL_Browse,
    PANEL_Atomfile,
      BUTTON_Browse,
      TEXT_Filename,
    PANEL_Radiusfile,
      BUTTON_Radius,
      TEXT_Radius,
    PANEL_FileOptions,
      BUTTON_LoadFiles,
//
  PANEL_Parameters,
    PANEL_Grid,
      TEXT_Grid,
      PANEL_Gridinput,
        TEXT_Gridinput,
        TEXT_Gridunit,
    PANEL_Depth,
      TEXT_Depth,
      SPIN_Depthinput,
//
  PANEL_AtomList,
    GRID_AtomList,

};

//DECLARE_APP(MainApp)

#endif
