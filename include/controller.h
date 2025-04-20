#ifndef CONTROLLER_H

#define CONTROLLER_H

#include "flags.h"
#include <iostream>
#include <unordered_map>
#include <wx/wx.h>

struct CalcReportBundle;
class Model;
class MainFrame;
class AtomTree;
struct Atom;

template <typename> class Container3D;
class Voxel;

class Ctrl{
  public:
    static Ctrl* getInstance();

    void hush(const bool);
    void version();

    void enableGUI();
    void disableGUI();
    bool isGUIEnabled();

    static std::string getDefaultElemPath();
    static std::string getVersion();

    bool loadElementsFile();
    bool loadAtomFile();
    bool runCalculation();
    bool runCalculation(const double, const double, const double, const std::string&,
        const std::string&, const std::string&, const int, const bool, const bool,
        const bool, const bool, const bool, const bool, const bool, const unsigned);
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
    void renderSurface(const Container3D<Voxel>&, const std::array<double,3>, 
        const double, const bool, const unsigned char, const std::vector<Atom>&);
    const Container3D<Voxel>& getSurfaceData() const;

    void newCalculation();
    void calculationDone(const bool=true);
    bool isCalculationDone();
    void setAbortFlag(const bool=true);
    bool getAbortFlag();
    void updateCalculationStatus();

    void displayErrorMessage(const int, const std::vector<std::string>& =std::vector<std::string>());

  private:
    // consider making static pointer for model
    Model* _current_calculation;
    // static attributes to ensure there is only one of each
    static Ctrl* s_instance;
    static MainFrame* s_gui;

    bool _abort_calculation; // variable for main thread to signal stopping the calculation
    bool _calculation_finished;
    bool _to_gui = true; // determines whether to print to console or to GUI
    bool _quiet = true; // silences all non-result command line outputs

    void displayInput(CalcReportBundle&, const unsigned=mvOUT_ALL);
    void displayResults(CalcReportBundle&, const unsigned=mvOUT_ALL);
    void displayCavityList(CalcReportBundle&, const unsigned=mvOUT_ALL);
    std::string getErrorMessage(const int);

    inline static const std::string s_version = "1.2.0";
    inline static const std::string s_elem_file = "elements.txt";
};

#endif
