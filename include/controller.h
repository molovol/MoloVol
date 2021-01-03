#ifndef CONTROLLER_H

#define CONTROLLER_H

#include <iostream>
#include <unordered_map>
#include <wx/wx.h>

class Model;
class MainFrame;
class Ctrl{
  public:
    bool loadRadiusFile();
    bool loadAtomFile();
    bool runCalculation();
    bool runCalculation(std::string,double,int,std::unordered_map<std::string, double>,double,double,bool,bool,double,std::vector<std::string>,std::string);
    void registerView(MainFrame* inp_gui);
    static Ctrl* getInstance();
    std::vector<std::string> getGuiParameters(); // confusing name, this is not a getter function
    void notifyUser(std::string str);

  private:
    // consider making static pointer for model
    Model* current_calculation;
    // static attributes to ensure there is only one of each
    static Ctrl* instance;
    static MainFrame* gui;

//    saveLastWritten(std::string
};


#endif
