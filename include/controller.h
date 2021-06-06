#ifndef CONTROLLER_H

#define CONTROLLER_H

#include <iostream>
#include <unordered_map>
#include <wx/wx.h>

class Model;
class MainFrame;
class Ctrl{
  public:
    static Ctrl* getInstance();

    void enableGUI();
    void disableGUI();
    bool isGUIEnabled();

    bool loadRadiusFile();
    bool loadAtomFile();
    bool runCalculation();
    void registerView(MainFrame* inp_gui);
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

    void displayErrorMessage(const int);
    void printErrorMessage(const int);
    // unit tests
    bool unittestExcluded();
    bool unittestProtein();
    bool unittestRadius();
    bool unittest2Probe();
    bool unittestSurface();
    bool unittestFloodfill();

    inline static const std::string s_version = "beta 1";
  private:
    // consider making static pointer for model
    Model* _current_calculation;
    // static attributes to ensure there is only one of each
    static Ctrl* s_instance;
    static MainFrame* s_gui;

    bool _abort_calculation; // variable for main thread to signal stopping the calculation
    bool _calculation_finished;
    bool _to_gui = true; // determines whether to print to console or to GUI
    
    std::string getErrorMessage(const int);
};


#endif
