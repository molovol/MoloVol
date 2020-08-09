#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#  include <wx/wx.h>
#endif

#include "base.h"
#include <string>

//////////////////////////////////
// METHODS FOR MANIPULATING GUI //
//////////////////////////////////

void MainFrame::clearOutput(){
  outputText->SetValue("");
}

void MainFrame::printToOutput(std::string& text){
  outputText->SetValue(text);
}

void MainFrame::printToOutputUnicode(std::wstring& text){
  outputText->SetValue(text);
}

void MainFrame::appendOutput(std::string& text){
  outputText->SetValue(outputText->GetValue() + text);
}

void MainFrame::appendOutputUnicode(std::wstring& text){
  outputText->SetValue(outputText->GetValue() + text);
}

std::string MainFrame::getAtomFilepath(){
  return filepathText->GetValue().ToStdString();
}

std::string MainFrame::getRadiusFilepath(){
  return radiuspathText->GetValue().ToStdString();
}

double MainFrame::getGridsize(){
  return std::stod(gridsizeInputText->GetValue().ToStdString());
}

int MainFrame::getDepth(){
  return depthInput->GetValue();
}

//* get from user input
double MainFrame::getProbeRadius(){
  return r_probe;
}
