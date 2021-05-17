#ifndef BASE_H

#define BASE_H

#include <wx/filectrl.h>
#include <wx/filepicker.h>
#include <wx/wfstream.h>
#include <wx/spinctrl.h>
#include <wx/grid.h>
#include <wx/cmdline.h>
#include <wchar.h>
#include <string>
#include <iostream>
#include <vector>
#include <tuple>
#include <unordered_map>
#include <map>

class MainApp: public wxApp
{
  public:
    virtual bool OnInit();
    virtual int OnRun();

  private:
    void silenceGUI(bool);
    bool isSilent();
    bool _silent = false;
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
    void appendOutput(std::wstring& text);
    std::string getAtomFilepath();
    std::string getRadiusFilepath();
    bool getIncludeHetatm();
    bool getAnalyzeUnitCell();
    bool getCalcSurfaceAreas();
    bool getProbeMode();
    double getProbe1Radius();
    double getProbe2Radius();
    double getGridsize();
    int getDepth();
    bool getMakeReport();
    bool getMakeSurfaceMap();
    bool getMakeCavityMaps();
    std::string getOutputDir();
    void enableGuiElements(bool inp); // method to turn on and off interactable gui elements

    void displayAtomList(std::vector<std::tuple<std::string, int, double>> symbol_number_radius);
    std::unordered_map<std::string, double> generateRadiusMap();
    double getMaxRad();
    std::vector<std::string> getIncludedElements();
    MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size);

  private:
    wxPanel* preCalcPanel;
      wxPanel* leftMainPanel;
        wxPanel* browsePanel;
          wxPanel* atomfilePanel;
            wxStaticText* atomText;
            wxButton* browseButton;
            wxTextCtrl* filepathText;
          wxPanel* radiusfilePanel;
            wxStaticText* radiusText;
            wxButton* radiusButton;
            wxTextCtrl* radiuspathText;
          wxPanel* fileOptionsPanel;
            wxCheckBox* pdbHetatmCheckbox;
            wxButton* loadFilesButton;
        wxPanel* atomListPanel;
          wxGrid* atomListGrid;

      wxPanel* rightMainPanel;
        wxPanel* parameterPanel;
          wxCheckBox* unitCellCheckbox;
          wxCheckBox* surfaceAreaCheckbox;
          wxCheckBox* twoProbesCheckbox;
          wxPanel* probe1Panel;
            wxStaticText* probe1Text;
            wxPanel* probe1InputPanel;
              wxTextCtrl* probe1InputText;
              wxStaticText* probe1UnitText; // possibly usable universally?
          wxPanel* probe2Panel;
            wxStaticText* probe2Text;
            wxPanel* probe2InputPanel;
              wxTextCtrl* probe2InputText;
              wxStaticText* probe2UnitText; // possibly usable universally?
          wxPanel* gridsizePanel;
            wxStaticText* gridsizeText;
            wxPanel* gridsizeInputPanel;
              wxTextCtrl* gridsizeInputText;
              wxStaticText* gridsizeUnitText; // possibly usable universally?
          wxPanel* depthPanel;
            wxStaticText* depthText;
            wxSpinCtrl* depthInput;
          wxCheckBox* reportCheckbox;
          wxCheckBox* surfaceMapCheckbox;
          wxCheckBox* cavityMapsCheckbox;
          wxPanel* outputpathPanel;
            wxStaticText* outputdirText;
            wxDirPickerCtrl* outputdirPicker;
        wxPanel* sandrPanel;
          wxTextCtrl* outputText;
          wxButton* calcButton;
    
    wxPanel* postCalcPanel;
      wxPanel* communicationPanel;
        //wxGauge*
        wxPanel* outputPanel;
          //outputText from above
          //wxGrid*
      wxPanel* exportPanel;
        wxPanel* reportExportPanel;
          //wxButton*
          //wxCheckBox*
        wxPanel* totalMapExportPanel;
          //wxButton*
          //wxCheckBox*
        wxPanel* cavityMapExportPanel;
          //wxButton*
          //wxCheckBox*
        wxPanel* autoExportPanel;

    // set and manipulate gui interactivity
    void InitDefaultStates();
    std::map<wxWindow*, bool> default_states;
    void setDefaultState(wxWindow*, bool);
    void toggleOptionsPdb();
    void toggleButtons();

    // methods to initialise gui
    void InitTopLevel();
    void InitLeftMainPanel();
    void InitBrowsePanel();
    void InitAtomfilePanel();
    void InitRadiusfilePanel();
    void SetSizerFilePanel(wxPanel* panel, wxStaticText* text, wxButton* button, wxTextCtrl* path);
    void InitFileOptionsPanel();
    void InitAtomListPanel();
    void InitRightMainPanel();
    void InitParametersPanel();
    void InitProbe1Panel();
    void InitProbe1InputPanel();
    void InitProbe2Panel();
    void InitProbe2InputPanel();
    void InitGridPanel();
    void InitGridinputPanel();
    void InitDepthPanel();
    void InitOutputpathPanel();
    void InitSandr();

    // methods to handle events
    void OnExit(wxCommandEvent& event);
    void OnPrint(wxCommandEvent& event);
    void OnCalc(wxCommandEvent& event);
    void OnAtomBrowse(wxCommandEvent& event);
    void OnRadiusBrowse(wxCommandEvent& event);
    void OnLoadFiles(wxCommandEvent& event);
    void OnBrowse(wxCommandEvent& event, std::string& filetype, wxTextCtrl* textbox);
    void ProbeModeChange(wxCommandEvent& event);
    void GridChange(wxGridEvent& event);

    // colours
    wxColour col_panel = wxColour(160,160,160);
    wxColour col_output = wxColour(192,192,192);
    wxColour col_white = wxColour(255,255,255);
    wxColour col_grey_cell = wxColour(150,150,150);
    wxColour col_red_cell = wxColour(255,75,75);
    wxColour col_cyan_cell = wxColour(120,255,255);

    DECLARE_EVENT_TABLE()
};

enum
{
  // assign an ID
  PANEL_PreCalc,
    PANEL_LeftMain = wxID_HIGHEST + 1,
      PANEL_Browse,
        PANEL_Atomfile,
          TEXT_Atom,
          BUTTON_Browse,
          TEXT_Filename,
        PANEL_Radiusfile,
          TEXT_Radius,
          BUTTON_Radius,
          TEXT_Radiuspath,
        PANEL_FileOptions,
          CHECKBOX_Hetatm,
          BUTTON_LoadFiles,
      PANEL_AtomList,
        GRID_AtomList,
  
    PANEL_RightMain,
      PANEL_Parameters,
        CHECKBOX_UnitCell,
        CHECKBOX_SurfaceArea,
        CHECKBOX_TwoProbes,
        PANEL_Probe1,
          TEXT_Probe1,
          PANEL_Probe1Input,
            TEXT_Probe1Input,
            TEXT_Probe1Unit,
        PANEL_Probe2,
          TEXT_Probe2,
          PANEL_Probe2Input,
            TEXT_Probe2Input,
            TEXT_Probe2Unit,
        PANEL_Grid,
          TEXT_Grid,
          PANEL_Gridinput,
            TEXT_Gridinput,
            TEXT_Gridunit,
        PANEL_Depth,
          TEXT_Depth,
          SPIN_Depthinput,
        CHECKBOX_Report,
        CHECKBOX_SurfaceMap,
        CHECKBOX_CavityMaps,
        PANEL_Outputpath,
          TEXT_Outputdir,
          BUTTON_Output,
      PANEL_Sandr,
        TEXT_Output,
        BUTTON_Calc,
   
  PANEL_PostCalc,
    PANEL_Communication,
      //wxGauge*
      PANEL_Output,
        //outputText from above
        //wxGrid*
    PANEL_Export,
      PANEL_ReportExport,
        //wxButton*
        //wxCheckBox*
      PANEL_TotalMapExport,
        //wxButton*
        //wxCheckBox*
      PANEL_CavityMapExport,
        //wxButton*
        //wxCheckBox*
      PANEL_AutoExport

};

//DECLARE_APP(MainApp)

#endif
