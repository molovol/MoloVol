#ifndef CONTROLLER_H

#define CONTROLLER_H

#include <iostream>
#include <unordered_map>
#include <wx/wx.h>

struct CalcResultBundle;
class Model;
class MainFrame;
class Ctrl{
  public:
    bool loadRadiusFile();
    bool loadAtomFile();
    bool runCalculation();
    CalcResultBundle runCalculation(
        std::string,
        double,
        int,
        std::unordered_map<std::string, double>,
        std::vector<std::string>,
        double = 0,
        double = 0,
        bool = false,
        bool = false,
        double = 0,
        std::string = "0");
    void registerView(MainFrame* inp_gui);
    static Ctrl* getInstance();
    std::vector<std::string> getGuiParameters(); // confusing name, this is not a getter function
    void notifyUser(std::string str, bool = true);
    bool unittestExcluded();
    bool unittestProtein();

  private:
    // consider making static pointer for model
    Model* current_calculation;
    // static attributes to ensure there is only one of each
    static Ctrl* instance;
    static MainFrame* gui;

    bool to_gui = true; // determines whether to print to console or to GUI
};


#endif
