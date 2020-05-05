#ifndef CONTROLLER_H

#define CONTROLLER_H

#include <iostream>
#include <wx/wx.h>

class MainFrame;
class Ctrl{
  public:
    void runCalculation(std::string& filepath);
    void registerView(MainFrame* inp_gui);
    static Ctrl* getInstance();

  private:
//    Ctrl() = default;
    static Ctrl* instance;
    static MainFrame* gui;
};
    

#endif
