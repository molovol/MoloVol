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

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include <chrono>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	unsigned char* colormatrix = createImageGPU("./renderkernel.cl", width, height);

	//directly copying does not work in runtime somehow (it should)
	imgbuffer = wxImage(width, height, colormatrix, false);//static_data	Indicates if the data should be free'd after use
	//wxBitmap bmp(wimgbuffer, 24);
	image = wxBitmap(imgbuffer, 24); // explicit depth important under MSW
	wxNativePixelData data(image);
	if ( !data ){
		// ... raw access to bitmap data unavailable, do something else ...
		return;
	}}

#define MAX_SOURCE_SIZE (0x100000)
unsigned char* wxVolumeRenderer::createImageGPU(std::string const& kernelpath, unsigned int width, unsigned int height){
	//tmp input matrix
	unsigned int size_inputmatrix = 100 * 100*100;
	unsigned int mem_size_A = sizeof(float) * size_inputmatrix;
	float* inputmatrix = (float*) malloc(size_inputmatrix * sizeof(float*));
	
	//create output matrix
	//unsigned int num_elements =width*height;
	unsigned int size_colormatrix = width * height;
	unsigned int mem_size_colorm = sizeof(float) * size_colormatrix;
	unsigned char* lightmatrix = (unsigned char*) malloc(size_colormatrix * sizeof(unsigned char*));//can not be indexed with 3d array notation
	
	// OpenCL
	// -------------------------------------------------------------------------
	cl_int err;                          //
	cl_platform_id* platforms = NULL;    //
	char platform_name[1024];            //
	cl_device_id device_id = NULL;       //
	cl_uint num_of_platforms = 0,        //
			num_of_devices = 0;          //
	cl_context context;                  //
	cl_kernel kernel;                    //
	cl_command_queue command_queue;      //
	cl_program program;                  //
	cl_mem inputA, output;       //

	// Kernel laden
	// -------------------------------------------------------------------------
	FILE *fp;
	char *cSourceCL;
	size_t source_size;

	fp = fopen(kernelpath.c_str(), "r");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		exit(1);
	}

	cSourceCL = (char *)malloc(MAX_SOURCE_SIZE);
	source_size = fread(cSourceCL, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);


	// -------------------------------------------------------------------------
	// 1) Platform, Context und Befehlswarteschlange vorbereiten
	// -------------------------------------------------------------------------
	auto t_start = std::chrono::high_resolution_clock::now();

	// verfügbare Geräte auslesen
	err = clGetPlatformIDs(0, NULL, &num_of_platforms);
	if (err != CL_SUCCESS) {
		printf("No platforms found. Error: %d\n", err);
		return NULL;
	}

	//
	platforms = (cl_platform_id *) malloc(num_of_platforms);
	err = clGetPlatformIDs(num_of_platforms, platforms, NULL);
	if (err != CL_SUCCESS) {
		printf("No platforms found. Error: %d\n", err);
		return NULL;
	} else {
		//wenn auf nvidia wird es hier gespeichert
		int nvidia_platform = 0;

		// jedes verfügbare Gerät überprüfen
		for (unsigned int i = 0; i < num_of_platforms; i++) {
			//
			clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof (platform_name), platform_name, NULL);
			if (err != CL_SUCCESS) {
				printf("Could not get information about platform. Error: %d\n", err);
				return NULL;
			}

			// wenn nvidia speicher es
			if (strstr(platform_name, "NVIDIA") != NULL) {
				nvidia_platform = i;
				break;
			}
		}

		// überprüfe das Gerät
		err = clGetDeviceIDs(platforms[nvidia_platform], CL_DEVICE_TYPE_GPU, 1, &device_id, &num_of_devices);
		if (err != CL_SUCCESS) {
			printf("Could not get device in platform. Error: %d\n", err);
			return NULL;
		}
	}

	// erzeugt einen OpenCL context
	context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
	if (err != CL_SUCCESS) {
		printf("Unable to create context. Error: %d\n", err);
		return NULL;
	}

	// erzeugt eine command queue zu erzeugen
	command_queue = clCreateCommandQueue(context, device_id, 0, &err);
	if (err != CL_SUCCESS) {
		printf("Unable to create command queue. Error: %d\n", err);
		return NULL;
	}


	// -------------------------------------------------------------------------
	// 2) Buffer und Gerätespeicher vorbereiten
	// -------------------------------------------------------------------------
	printf("Image size [%d][%d]", width, height);

	// erzeugt handels für in und output buffer, die hier erzeugt werden
	inputA = clCreateBuffer(context, CL_MEM_READ_ONLY, mem_size_A, NULL, &err);
	output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, mem_size_colorm, NULL, &err);
	if (!inputA || !output){
		printf("Error: Failed to allocate device memory!\n");
		exit(1);
	}

	// schreibt die Daten aus "data" in den input buffer, jetzt A und B
	clEnqueueWriteBuffer(command_queue, inputA, CL_TRUE, 0, mem_size_A, inputmatrix, 0, NULL, NULL);

	// -------------------------------------------------------------------------
	// 3) Programm linken und kompilieren, Kernel und Argumente einrichten
	// -------------------------------------------------------------------------

	// erzeugt ein Programm Objekt und lädt den code
	program = clCreateProgramWithSource(context, 1, (const char **)&cSourceCL, (const size_t *)&source_size, &err);
	if (err != CL_SUCCESS) {
		printf("Unable to create program. Error: %d\n", err);
		return NULL;
	}

	//compiliert und linkt den Kernel
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS) {
		printf("Error building program. Error: %d\n", err);
		return NULL;
	}

	kernel = clCreateKernel(program, "matMult", &err);
	if (err != CL_SUCCESS) {
		printf("Error setting kernel. Error: %d\n", err);
		return NULL;
	}

	// verlinkt input und output mit dem kernel über die Argumente
	err =  clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputA);
	err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &output);
	if (err != CL_SUCCESS) {
		printf("Error: Failed to set kernel arguments! %d\n", err);
		exit(1);
	}


	// -------------------------------------------------------------------------
	// 4) Kernel starten und Ergebnisse einsammeln
	// -------------------------------------------------------------------------
	size_t  global[2] = {width, height}; //TODO was for each entry in C, should now for each in
	err = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, global, NULL, 0, NULL, NULL);
	if (err != CL_SUCCESS){
		printf("Error: Failed to execute kernel! %d\n", err);
		exit(1);
	}

	// Barriere bis alle command_queue Befehle abgearbeitet worden
	clFinish(command_queue);

	// ließt von der command_queue über den output buffer in results
	err = clEnqueueReadBuffer(command_queue, output, CL_TRUE, 0, mem_size_colorm, lightmatrix, 0, NULL, NULL);
	if (err != CL_SUCCESS){
		printf("Error: Failed to read output array! %d\n", err);
		exit(1);
	}


	// -------------------------------------------------------------------------
	// 5) OpenCL Objekte freigeben
	// -------------------------------------------------------------------------
	clReleaseMemObject(inputA);
	clReleaseMemObject(output);
	clReleaseProgram(program);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(command_queue);
	clReleaseContext(context);
	
	auto t_end = std::chrono::high_resolution_clock::now();
	std::cout << "Parallel time using OpenCL"
			  << std::chrono::duration<double, std::milli>(t_end-t_start).count()
			  <<  " ms" << std::endl;
	
	//to rgb
	unsigned char* colormatrix = (unsigned char*) malloc(width * height*3 * sizeof(unsigned char*));//can not be indexed with 3d array notation
	for (auto i=0;i<size_colormatrix;++i){
		colormatrix[i*3] = lightmatrix[i];
		colormatrix[i*3+1] = lightmatrix[i];
		colormatrix[i*3+2] = lightmatrix[i];
	}
	return colormatrix;
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
