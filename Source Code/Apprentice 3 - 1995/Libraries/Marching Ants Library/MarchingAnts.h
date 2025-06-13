//=====================================================================================
// MarchingAnts.h -- written by Aaron Giles
// Last update: 7/7/94 (version 1.1)
//=====================================================================================
// A source code library for handling rectangular graphical selections involving the
// famous "marching ants".  This header file contains a brief description of each
// function; see the prelude to each function in the definition file MarchingAnts.c
// for a full description of its operation and expected use.
//=====================================================================================
// This code has been compiled successfully under MPW C, MPW PPCC, THINK C, and
// Metrowerks C/C++, both 68k and PowerPC.  This code has been compiled under both the
// Universal Headers and the old 7.1 headers; if it works earlier than that I cannot
// say for sure.
//=====================================================================================
// If you find any bugs in this source code, please email me and I will attempt to fix
// them.  If you have any additions/modifications that you think would be generally
// useful, email those to me as well, and I will consider incorporating them into the
// next release.  My email address is giles@med.cornell.edu.
//=====================================================================================
// This source code is copyright � 1994, Aaron Giles.  Permission to use this code in
// your product is freely granted, provided that you credit me appropriately in your
// application's About box/credits *and* documentation.  If you ship an application
// which uses this code, I would also like to request that you provide me with one
// complimentary copy of the application.
//=====================================================================================

#ifndef __MARCHINGANTS__
#define __MARCHINGANTS__

//=====================================================================================
// Mac headers needed for the headers
//=====================================================================================

#include <QDOffscreen.h>

//=====================================================================================
// If you are using old headers that don't know about the new Pattern definitions you
// may need to uncomment this line.
//=====================================================================================

//#define dangerousPattern 1

//=====================================================================================
// Definition of the AntsRecord; note that there is no need for other functions to know
// anything about the internals here.  Everything that you will need is provided
// through accessor functions which accept parameters of type AntsReference, which is
// intended to be an opaque data type.
//=====================================================================================

typedef struct AntsRecord {
	GWorldPtr verticalWorld;
	GWorldPtr horizontalWorld;
	Rect bounds, limitRect;
	GrafPtr port;
	Boolean active;
	Boolean visible;
	Boolean drawn;
} AntsRecord, *AntsPtr, **AntsHandle;

//=====================================================================================
// Here is the AntsReference typedef.  All ants functions accept AntsReference
// parameters instead of AntsHandles.  This makes the AntsReference a semi-opaque data
// type.
//=====================================================================================

typedef AntsHandle AntsReference;

//=====================================================================================
// The AntsScrollProc is a callback function used for autoscrolling.  Basically it gets
// called with a requested dx,dy combination, and your autoscroll function will scroll
// as much as it can and return the actual amount scrolled as dx,dy.  The refCon
// parameter is a user-defined long variable which can be used to store information
// needed by the callback.
//=====================================================================================

typedef void (*AntsScrollProc)(short *dx, short *dy, long refCon);

//=====================================================================================
// Minimum values in each dimension for a "valid" selection.
//=====================================================================================

enum {
	antsMinimumX = 8,
	antsMinimumY = 8
};

//=====================================================================================
// The public ants interface.
//=====================================================================================

#ifdef __cplusplus
extern "C" {
#endif

// NewAnts: allocate and initialize a new set of ants
extern AntsReference NewAnts(void);

// DisposeAnts: dispose of all memory associated with an ants record
extern void DisposeAnts(AntsReference theAnts);

// AntsHaveSelection: returns true if there is a non-nil selection
extern Boolean AntsHaveSelection(AntsReference theAnts);

// ResetAntsSelection: resets the selection to nil
extern void ResetAntsSelection(AntsReference theAnts);

// GetAntsSelection: returns the bounding rectangle of the current selection
extern void GetAntsSelection(AntsReference theAnts, Rect *theBounds);

// SetAntsSelection: change the current selection's bounding rectangle
extern void SetAntsSelection(AntsReference theAnts, Rect *newBounds);

// GetAntsLimitRect: returns the limit rectangle for clipping and mouse tracking
extern void GetAntsLimitRect(AntsReference theAnts, Rect *limitRect);

// SetAntsLimitRect: sets the limit rectangle for clipping and mouse tracking
extern void SetAntsLimitRect(AntsReference theAnts, Rect *newLimit);

// GetAntsPort: returns the port used for drawing the ants
extern GrafPtr GetAntsPort(AntsReference theAnts);

// SetAntsPort: sets the port for ant drawing
extern void SetAntsPort(AntsReference theAnts, GrafPtr thePort);

// AreAntsVisible: returns true if the ants are currently being shown
extern Boolean AreAntsVisible(AntsReference theAnts);

// TrackAntsSelection: tracks the mouse as a new selection is made
extern void TrackAntsSelection(AntsReference theAnts, Point localStart, 
			AntsScrollProc scroll, long refCon);

// ShowAnts: makes the current ants selection visible
extern void ShowAnts(AntsReference theAnts);

// HideAnts: hides the current selection, restoring the original background
extern void HideAnts(AntsReference theAnts);

// AnimateAnts: rotates the dashed rectangle by one notch and redraws it
extern void AnimateAnts(AntsReference theAnts);

// ActivateAnts: sets up the ants so they display an animated dashed rectangle
extern void ActivateAnts(AntsReference theAnts);

// DeactivateAnts: sets the ants to display a stable dotted rectangle
extern void DeactivateAnts(AntsReference theAnts);

#ifdef __cplusplus
}
#endif

#endif
