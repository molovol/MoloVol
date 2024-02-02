#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#  include <wx/wx.h>
#endif

#include "base.h"
#include "controller.h"

#ifdef MOLOVOL_RENDERER
#include "render_frame.h"
#endif

////////////////////////////
// MAIN FRAME CONSTRUCTOR //
////////////////////////////

MainFrame::MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size)
  : wxFrame((wxFrame*) NULL, -1, title, pos, size)
{
  Ctrl::getInstance()->registerView(this);
  InitMessageQueue();
  InitTopLevel();

#ifdef MOLOVOL_RENDERER
  RenderWin = new RenderFrame(wxT("Hello World"), wxPoint(50,50), wxSize(400,200));
  RenderWin->Show(true);
#endif
};

