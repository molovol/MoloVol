#ifndef BASE_H

#define BASE_H

#include "cavity.h"
#include <wx/filectrl.h>
#include <wx/filepicker.h>
#include <wx/wfstream.h>
#include <wx/spinctrl.h>
#include <wx/grid.h>
#include <wx/cmdline.h>
#include <wx/statusbr.h>
#include <wx/thread.h>
#include <wchar.h>
#include <string>
#include <iostream>
#include <vector>
#include <tuple>
#include <unordered_map>
#include <map>
#include <utility>

class MainApp: public wxApp
{
  public:
    virtual bool OnInit();
    virtual int OnRun();

  private:
    void silenceGUI(bool);
    bool isSilent();
    // colours
    wxColour col_win = wxColour(160,160,160);
};

wxDECLARE_EVENT(wxEVT_COMMAND_WORKERTHREAD_COMPLETED, wxThreadEvent);

class MainFrame: public wxFrame, public wxThreadHelper
{
  public:
    MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size);

    // use these functions when changing the GUI externally, for instance from Ctrl
    // these methods are thread safe
    void extClearOutputText();
    void extClearOutputGrid();
    void extAppendOutput(const std::string);
    void extAppendOutputW(const std::wstring);
    
    void extSetStatus(const std::string);
    void extSetProgressBar(const int);
    void extDisplayCavityList(const std::vector<Cavity>&);

    void extOpenErrorDialog(const int, const std::string&);
    
    void printToOutput(const std::string text);
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
  protected:
    virtual wxThread::ExitCode Entry();
    bool m_data;
    wxCriticalSection m_dataCS;
  private:
    // gui control methods that may be called directly from the main thread
    void clearOutputText();
    void clearOutputGrid();
    void appendOutput(const std::string text);
    void appendOutputW(const std::wstring text);
    
    void setStatus(const std::string);
    void setProgressBar(const int);
    void displayCavityList(const std::vector<Cavity>&);
    
    void openErrorDialog(const std::pair<int,std::string>&);

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
          wxButton* abortButton;
    
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
          wxTextCtrl* dirpickerText;
          wxButton* dirpickerButton;

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
    void OnCalc(wxCommandEvent& event);
    void OnAbort(wxCommandEvent& event);
    void OnCalculationFinished(wxCommandEvent& event);
    void OnAtomBrowse(wxCommandEvent& event);
    void OnRadiusBrowse(wxCommandEvent& event);
    void OnLoadFiles(wxCommandEvent& event);
    void OnBrowse(wxCommandEvent& event, std::string& filetype, wxTextCtrl* textbox);
    void OnTextInput(wxCommandEvent&);
    void OnBrowseOutput(wxCommandEvent&);

    std::string OpenExportFileDialog(const std::string, const std::string);
    void OnExportReport(wxCommandEvent& event);
    void OnExportTotalMap(wxCommandEvent& event);
    void OnExportCavityMap(wxCommandEvent& event);
    void OnToggleAutoExport(wxCommandEvent& event);

    void ProbeModeChange(wxCommandEvent& event);
    void GridChange(wxGridEvent& event);

    // access functions
    std::vector<wxCheckBox*> getAutoExportCheckBoxes(){return {reportCheckbox, surfaceMapCheckbox, cavityMapsCheckbox};}

    // colours
    wxColour col_panel = wxColour(160,160,160);
    wxColour col_output = wxColour(192,192,192);
    wxColour col_white = wxColour(255,255,255);
    wxColour col_grey_cell = wxColour(150,150,150);
    wxColour col_red_cell = wxColour(255,75,75);
    wxColour col_cyan_cell = wxColour(120,255,255);
    wxColour col_dark_blue = wxColour(35,128,190);
    wxColour col_light_blue = wxColour(226,255,250);
    wxColour col_cream = wxColour(255,235,169);

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
        BUTTON_Abort,
   
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
        TEXT_Dirpicker,
        BUTTON_Dirpicker
};

//DECLARE_APP(MainApp)

#endif
