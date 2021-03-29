#ifndef CONTROLLER_H

#define CONTROLLER_H

#include <iostream>
#include <wx/wx.h>

class Model;
class MainFrame;
class Ctrl{
  public:
    bool loadRadiusFile();
    bool loadAtomFile();
    bool runCalculation();
    void registerView(MainFrame* inp_gui);
    static Ctrl* getInstance();
    void getGuiParameters(std::vector<std::string>&);
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
