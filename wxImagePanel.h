//
//  wxImagePanel.h
//  ballpit
//
//  Created by Benedikt Stefan Vogler on 19.08.20.
//

#ifndef wxImagePanel_h
#define wxImagePanel_h

#include <wx/rawbmp.h>
class wxImagePanel : public wxPanel
{
	wxBitmap image;
	
public:
	wxImagePanel(wxFrame* parent, wxBitmapType format);
	
	void paintEvent(wxPaintEvent & evt);
	void paintNow();
	
	void render(wxDC& dc);
	
	// some useful events
	/*
	 void mouseMoved(wxMouseEvent& event);
	 void mouseDown(wxMouseEvent& event);
	 void mouseWheelMoved(wxMouseEvent& event);
	 void mouseReleased(wxMouseEvent& event);
	 void rightClick(wxMouseEvent& event);
	 void mouseLeftWindow(wxMouseEvent& event);
	 void keyPressed(wxKeyEvent& event);
	 void keyReleased(wxKeyEvent& event);
	 */
	
	DECLARE_EVENT_TABLE()
};

#endif /* wxImagePanel_h */
