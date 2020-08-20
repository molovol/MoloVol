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
	//unsigned int num_elements =width*height;
	unsigned int size_colormatrix = width * height*3;//RGB
	//unsigned int mem_size_colorm = sizeof(float) * size_colormatrix;
	unsigned char* colormatrix = (unsigned char*) malloc(size_colormatrix * sizeof(unsigned char*));//can not be indexed with 3d array notation
	for (auto i=0;i<size_colormatrix;++i){
		colormatrix[i] = rand()%50;
	}
	colormatrix[30] = 250;
	colormatrix[70] = 250;
	colormatrix[300] = 250;
	colormatrix[420] = 250;
	
	//directly copying does not work in runtime somehow (it should)
	imgbuffer = wxImage(width, height, colormatrix, false);//static_data	Indicates if the data should be free'd after use
	//wxBitmap bmp(wimgbuffer, 24);
	image = wxBitmap(imgbuffer, 24); // explicit depth important under MSW
	wxNativePixelData data(image);
	if ( !data ){
		// ... raw access to bitmap data unavailable, do something else ...
		return;
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
