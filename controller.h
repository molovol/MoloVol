#ifndef CONTROLLER_H

#define CONTROLLER_H

#include <iostream>
#include <wx/wx.h>

class Model;
class MainFrame;
class Ctrl{
  public:
    bool runCalculation();
    void registerView(MainFrame* inp_gui);
    static Ctrl* getInstance();
    void notifyUser(std::string str);
    void notifyUserUnicode(std::wstring wstr);

  private:
    Model* current_calculation;
    // static attributes to ensure there is only one of each
    static Ctrl* instance;
    static MainFrame* gui;
};


#endif
