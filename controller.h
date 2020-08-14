#ifndef CONTROLLER_H

#define CONTROLLER_H

#include <iostream>
#include <wx/wx.h>
#include <vector>

#include <tuple>

class Model;
class MainFrame;
class Ctrl{
  public:
    std::vector<std::tuple<std::string, int, double>> loadInputFiles();
    bool runCalculation();
    void registerView(MainFrame* inp_gui);
    static Ctrl* getInstance();
    void notifyUser(std::string str);
    void notifyUser(std::wstring wstr);
    std::wstring generateChemicalFormulaUnicode(std::string chemical_formula);

  private:
    Model* current_calculation;
    Model* current_file_loading;
    // static attributes to ensure there is only one of each
    static Ctrl* instance;
    static MainFrame* gui;
};


#endif
