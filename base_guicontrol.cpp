#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#  include <wx/wx.h>
#endif

#include "base.h"

//////////////////////////////////
// METHODS FOR MANIPULATING GUI //
//////////////////////////////////

void MainFrame::clearOutput(){
  outputText->SetValue("");
}

void MainFrame::printToOutput(std::string& text){
  outputText->SetValue(text);
}

void MainFrame::appendOutput(std::string& text){
  outputText->SetValue(outputText->GetValue() + text);
}
