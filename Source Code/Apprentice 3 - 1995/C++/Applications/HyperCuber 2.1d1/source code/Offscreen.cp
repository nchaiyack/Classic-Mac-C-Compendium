//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the routines which manipulate the off-screen pixmap
//|________________________________________________________________________________

#include "HyperCuber Errors.h"

#include <Exceptions.h>
#include <TCLUtilities.h>



//============================ Globals ============================\\

CGrafPort			left_offscreen_cport;		//  The offscreen grafport for left eye view,
												//    or for the only view if viewing in mono
CGrafPtr			left_offscreen_cptr = &left_offscreen_cport;
CGrafPtr			offscreen_cptr = &left_offscreen_cport;

CGrafPort			right_offscreen_cport;		//  The offscreen grafport for right eye view
CGrafPtr			right_offscreen_cptr = &right_offscreen_cport;

extern Boolean		drawing_disabled;


//============================ Prototypes ============================\\

void CreateOffscreenPixmap(CGrafPort *color_port, Rect *bounds);
void DisposeOffscreenPixmap(CGrafPort *color_port);
void AdjustOffscreenPixmap(CGrafPort *color_port, Rect *bounds);

extern void GeneralError(short error);



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure: CreateOffscreenPixmap
//|
//| Purpose: create a new offscreen pixmap
//|
//| Parameters: color_port: points to CGrafPort to act as port
//|             bounds:     boundaries to use for pixmap
//|______________________________________________________________

void CreateOffscreenPixmap(CGrafPort *color_port, Rect *bounds)
{

	GrafPtr temp_port;	
	GetPort (&temp_port);										//  Save the current GrafPort

	OpenCPort(color_port);										//  Create a new CGrafPort

	SetPort((GrafPtr) color_port);								//  Make this the current port
	TextFont(applFont);											//  Change the font to application

	short width = ((bounds->right - bounds->left + 3) / 4) * 4;	//  Round width to longword
	short height = bounds->bottom - bounds->top;

	Ptr base_address = NewPtr(width * height);					//  Allocate space for bitmap

	if (!base_address)
		ExitToShell();											//  Should never happen, since
																//    bitmap starts at 1x1

	PixMap *pixmap = *(*color_port).portPixMap;					//  Get pointer to the PixMap
	pixmap->baseAddr = base_address;
	pixmap->rowBytes = width;
	pixmap->rowBytes |= 32768;									//  Make it a PixMap
	pixmap->bounds = *bounds;
	pixmap->pixelSize = 8;
	pixmap->cmpCount = 1;
	pixmap->cmpSize = 8;

	SetPort(temp_port);											//  Restore the old port

}	//==== CreateOffscreenPixmap() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure: AdjustOffscreenPixmap
//|
//| Purpose: adjust an existing offscreen pixmap so it is as large as bounds
//|
//| Parameters: color_port: the CGrafPort containing the pixmap
//|             bounds:     new boundaries to use for pixmap
//|___________________________________________________________________________________

void AdjustOffscreenPixmap(CGrafPort *color_port, Rect *bounds)
{

	short new_width = bounds->right - bounds->left;		//  Find new size
	short new_row_bytes = ((new_width + 3) / 4) * 4;
	short new_height = bounds->bottom - bounds->top;
		
	DisposPtr((**(*color_port).portPixMap).baseAddr);		//  Dispose of old bitmap
	
	
	Handle base_handle =
			NewHandleCanFail(new_row_bytes * new_height);	//  Check if there's enough memory
	if (base_handle == NULL)
		{
		GeneralError(bitmap_too_large);						//  Tell user about error
		drawing_disabled = TRUE;							//  Disable drawing
		return;
		}
		
	DisposHandle(base_handle);
	Ptr base_address = NewPtr(new_row_bytes * new_height);	//  Allocate space for bitmap

	Rect new_bounds = {0, 0, new_height, new_width};

	(**(*color_port).portPixMap).baseAddr = base_address;		//  Set up new pixmap bounds
	(**(*color_port).portPixMap).rowBytes = new_row_bytes + 0x8000;
	(**(*color_port).portPixMap).bounds = new_bounds;
	
}	//==== AdjustOffscreenPixmap() ====\\


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure: DisposeOffscreenPixmap
//|
//| Purpose: dispose of an offscreen pixmap
//|
//| Parameters: color_port: the offscreen CGrafPort
//|______________________________________________________________

void DisposeOffscreenPixmap(CGrafPort *color_port)
{

	DisposPtr((**(*color_port).portPixMap).baseAddr);
	CloseCPort(color_port);

}	//==== DisposeOffscreenPixmap() ====\\


