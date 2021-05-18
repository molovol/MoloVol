#ifndef BASE_H

#define BASE_H

#include <wx/filectrl.h>
#include <wx/filepicker.h>
#include <wx/wfstream.h>
#include <wx/spinctrl.h>
#include <wx/grid.h>
#include <wx/cmdline.h>
#include <wx/statusbr.h>
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
    wxStatusBar* statusBar;

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
        wxPanel* sandrPanel;
          wxButton* calcButton;
    
    wxPanel* postCalcPanel;
      wxPanel* communicationPanel;
        wxGauge* progressGauge;
        wxPanel* outputPanel;
          wxTextCtrl* outputText;
          wxGrid* outputGrid;
      wxPanel* exportPanel;
        wxPanel* reportExportPanel;
          wxButton* reportButton;
          wxCheckBox* reportCheckbox;
        wxPanel* totalMapExportPanel;
          wxButton* totalMapButton;
          wxCheckBox* surfaceMapCheckbox;
        wxPanel* cavityMapExportPanel;
          wxButton* cavityMapButton;
          wxCheckBox* cavityMapsCheckbox;
        wxPanel* autoExportPanel;
          wxStaticText* outputdirText;
          wxDirPickerCtrl* outputdirPicker;

    // set and manipulate gui interactivity
    void InitDefaultStates();
    std::map<wxWindow*, bool> default_states;
    void setDefaultState(wxWindow*, bool);
    void toggleOptionsPdb();
    void toggleButtons();

    // methods to initialise gui
    void InitTopLevel();
    
    void InitPreCalcPanel();
    void InitLeftMainPanel();
    void InitBrowsePanel();
    void InitAtomfilePanel();
    void InitRadiusfilePanel();
    void SetSizerFilePanel(wxPanel*, wxStaticText*, wxButton*, wxTextCtrl*);
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
    void InitSandr();
    
    void InitPostCalcPanel();
    void InitCommunicationPanel();
    void InitOutputPanel();

    void InitExportPanel();
    void InitReportExportPanel();
    void InitTotalMapExportPanel();
    void InitCavityMapExportPanel();
    void InitAutoExportPanel();
    void SetSizerExportSubPanel(wxPanel*, wxButton*, wxCheckBox*);

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
  STATUSBAR = wxID_HIGHEST+1,

  PANEL_PreCalc,
    PANEL_LeftMain,
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
      PANEL_Sandr,
        BUTTON_Calc,
   
  PANEL_PostCalc,
    PANEL_Communication,
      GAUGE_Progress,
      PANEL_Output,
        TEXT_Output,
        GRID_Output,
    PANEL_Export,
      PANEL_ReportExport,
        BUTTON_Report,
        CHECKBOX_Report,
      PANEL_TotalMapExport,
        BUTTON_TotalMap,
        CHECKBOX_SurfaceMap,
      PANEL_CavityMapExport,
        BUTTON_CavityMap,
        CHECKBOX_CavityMaps,
      PANEL_AutoExport,
        TEXT_Outputdir,
        BUTTON_Output
};

//DECLARE_APP(MainApp)

#endif
