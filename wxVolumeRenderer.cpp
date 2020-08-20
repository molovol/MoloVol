//
//  wxVolumeRenderer.cpp
//  ballpit
//
//  Created by Benedikt Stefan Vogler on 20.08.20.
//

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#  include <wx/wx.h>
#endif

#include "wxVolumeRenderer.h"
#include <wx/rawbmp.h>

BEGIN_EVENT_TABLE(wxVolumeRenderer, wxPanel)

// catch paint events
EVT_PAINT(wxVolumeRenderer::paintEvent)

END_EVENT_TABLE()


wxVolumeRenderer::wxVolumeRenderer(wxFrame* parent,
						   wxBitmapType format) : wxPanel(parent){
	auto width = 512;
	auto height = 512;
	image = wxBitmap(width, height, 24); // explicit depth important under MSW
	wxNativePixelData data(image);
	if ( !data ){
		// ... raw access to bitmap data unavailable, do something else ...
		return;
	}
	wxNativePixelData::Iterator p(data);
	// we draw a (10, 10)-(20, 20) rect manually using the given r, g, b
	p.Offset(data, 10, 10);
	for ( int y = 0; y < 10; ++y ){
		wxNativePixelData::Iterator rowStart = p;
		for ( int x = 0; x < 10; ++x, ++p ){
			p.Red() = 0;
			p.Green() = 250;
			p.Blue() = 30;
		}
		p = rowStart;
		p.OffsetY(data, 1);
	}
}

/*
 * Called by the system of by wxWidgets when the panel needs
 * to be redrawn. You can also trigger this call by
 * calling Refresh()/Update().
 */
void wxVolumeRenderer::paintEvent(wxPaintEvent & evt){
    // depending on your system you may need to look at double-buffered dcs
    wxPaintDC dc(this);
    render(dc);
}

/*
 * Alternatively, you can use a clientDC to paint on the panel
 * at any time. Using this generally does not free you from
 * catching paint events, since it is possible that e.g. the window
 * manager throws away your drawing when the window comes to the
 * background, and expects you will redraw it when the window comes
 * back (by sending a paint event).
 */
void wxVolumeRenderer::paintNow(){
    // depending on your system you may need to look at double-buffered dcs
    wxClientDC dc(this);
    render(dc);
}

/*
 * Here we do the actual rendering. I put it in a separate
 * method so that it can work no matter what type of DC
 * (e.g. wxPaintDC or wxClientDC) is used.
 */
void wxVolumeRenderer::render(wxDC&  dc){
    dc.DrawBitmap( image, 0, 0, false );
}
