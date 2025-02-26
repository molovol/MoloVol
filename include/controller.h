#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "flags.h"
#include <iostream>
#include <unordered_map>
#include <vector>
#ifdef MOLOVOL_GUI
#include <wx/wx.h>
class MainFrame;
#endif

struct CalcReportBundle;
class Model;

class Ctrl {
  public:
    static Ctrl* getInstance();

    void hush(const bool);
    void version();

    void enableGUI();
    void disableGUI();
    bool isGUIEnabled();

    static std::string getDefaultElemPath();
    static std::string getVersion();

#ifdef MOLOVOL_GUI
    bool loadElementsFile();
    bool loadAtomFile();
    bool runCalculation();  // GUI version
    void registerView(MainFrame* inp_gui);
#endif

    // CLI version
    bool runCalculation(const double, const double, const double, 
                       const std::string&, const std::string&, const std::string&, 
                       const int, const bool, const bool, const bool, const bool, 
                       const bool, const bool, const bool, const unsigned);

    void clearOutput();
    void notifyUser(std::string);
    void notifyUser(std::wstring);
    void updateStatus(std::string);
    void updateProgressBar(const int);
    void prepareOutput(std::string);
    void exportReport();
    void exportReport(std::string);
    void exportSurfaceMap(bool);
    void exportSurfaceMap(const std::string, bool);

    void newCalculation();
    void calculationDone(const bool=true);
    bool isCalculationDone();
    void setAbortFlag(const bool=true);
    bool getAbortFlag();
    void updateCalculationStatus();

    void displayErrorMessage(const int, const std::vector<std::string>& =std::vector<std::string>());

  private:
    Model* _current_calculation;
    static Ctrl* s_instance;
#ifdef MOLOVOL_GUI
    static MainFrame* s_gui;
#endif

    bool _abort_calculation;
    bool _calculation_finished;
    bool _to_gui = true;
    bool _quiet = true;

    void displayInput(CalcReportBundle&, const unsigned=mvOUT_ALL);
    void displayResults(CalcReportBundle&, const unsigned=mvOUT_ALL);
    void displayCavityList(CalcReportBundle&, const unsigned=mvOUT_ALL);
    std::string getErrorMessage(const int);

    inline static const std::string s_version = "1.1.1";
    inline static const std::string s_elem_file = "elements.txt";
};

#endif