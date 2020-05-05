#ifndef CONTROLLER_H

#define CONTROLLER_H

#include <iostream>
#include <wx/wx.h>

class Model;
class MainFrame;
class Ctrl{
  public:
    bool runCalculation(std::string& filepath);
    void registerView(MainFrame* inp_gui);
    static Ctrl* getInstance();

  private:
    Model* current_calculation;
    // static attributes to ensure there is only one of each
    static Ctrl* instance;
    static MainFrame* gui;
};
    

#endif
