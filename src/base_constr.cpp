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
#if defined(_WIN32)
  SetIcon(wxICON(aaaa));
#endif
  
  Ctrl::getInstance()->registerView(this);
  InitMessageQueue();
  InitTopLevel();

#ifdef MOLOVOL_RENDERER
  m_renderWin = new RenderFrame(this, wxT("Render Window"), wxPoint(50,50), wxSize(400,400));
  m_renderWin->Show(false);
#endif

  // Create a menu bar
  wxMenu *menuHelp = new wxMenu;
  menuHelp->Append(1001, "VTK License", "View VTK License Agreement");
  menuHelp->Append(wxID_ABOUT, "About");

  wxMenu *menuFile = new wxMenu;
  menuFile->Append(wxID_EXIT, "Exit\tCmd-Q", "Quit MoloVol");

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(menuHelp, "&Help");

  SetMenuBar(menuBar);

};

