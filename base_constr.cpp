#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#  include <wx/wx.h>
#endif

#include "base.h"
#include "controller.h"

////////////////////////////
// MAIN FRAME CONSTRUCTOR //
////////////////////////////

MainFrame::MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size)
  : wxFrame((wxFrame*) NULL, -1, title, pos, size)
{
  Ctrl::getInstance()->registerView(this);
   
  InitTopLevel();
  
  InitBrowsePanel();

  InitAtomfilePanel();

  InitRadiusfilePanel();

  //
  InitParametersPanel();
  
  InitGridPanel();

  InitGridinputPanel();

  InitDepthPanel();

  //

  InitSandr();  
};


