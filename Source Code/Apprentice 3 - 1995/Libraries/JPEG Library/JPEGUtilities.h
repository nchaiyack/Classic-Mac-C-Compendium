//=====================================================================================
// JPEGUtilities.h -- written by Aaron Giles
// Last update: 7/7/94
//=====================================================================================
// A source code library for performing very simple operations (drawing, embedding into
// PICTs, and extracting from PICTs) on JPEG-compressed images using QuickTime.
//=====================================================================================
// This code has been successfully compiled under Metrowerks C/C++ 1.0a4, under
// THINK C 7.0, and under MPW C 3.3.
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

#ifndef __JPEGUTILITIES__
#define __JPEGUTILITIES__

//=====================================================================================
// Generic includes for Macintosh headers
//=====================================================================================

#include <ImageCompression.h>
#include <Memory.h>
#include <QuickDraw.h>
#include <Types.h>

//=====================================================================================
// Public interface function prototypes
//=====================================================================================

#ifdef __cplusplus
extern "C" {
#endif

// GetJPEGBounds -- returns the bounding rectangle for the JPEG image in theHandle
extern OSErr GetJPEGBounds(Handle theHandle, Rect *theBounds);

// DrawJPEG -- draws all or part of the JPEG image in theHandle to the current port
extern OSErr DrawJPEG(Handle theHandle, Rect *srcRect, Rect *dstRect, short tMode); 

// UnwrapJPEG -- returns a handle to the first JPEG image from the PICT in srcHandle
extern Handle UnwrapJPEG(PicHandle srcHandle);

// WrapJPEG -- creates a JPEG-compressed PICT from the JPEG in srcHandle
extern PicHandle WrapJPEG(Handle srcHandle);

#ifdef __cplusplus
}
#endif

//=====================================================================================
// Kludges to make this code work for non-Universal Header types.  Note that we rely
// on USESROUTINEDESCRIPTORS being defined always when using the Universal Headers.
// This seems to be a good assumption -- until Apple changes things again. :-)
//=====================================================================================

#ifndef USESROUTINEDESCRIPTORS
typedef Ptr UniversalProcPtr;
typedef Ptr StdPixUPP;
typedef Ptr QDBitsUPP;
typedef Ptr QDTextUPP;
typedef Ptr QDLineUPP;
typedef Ptr QDRectUPP;
typedef Ptr QDRRectUPP;
typedef Ptr QDOvalUPP;
typedef Ptr QDArcUPP;
typedef Ptr QDPolyUPP;
typedef Ptr QDRgnUPP;
#define NewStdPixProc(proc) ((Ptr)proc)
#define NewQDBitsProc(proc) ((Ptr)proc)
#define NewQDTextProc(proc) ((Ptr)proc)
#define NewQDLineProc(proc) ((Ptr)proc)
#define NewQDRectProc(proc) ((Ptr)proc)
#define NewQDRRectProc(proc) ((Ptr)proc)
#define NewQDOvalProc(proc) ((Ptr)proc)
#define NewQDArcProc(proc) ((Ptr)proc)
#define NewQDPolyProc(proc) ((Ptr)proc)
#define NewQDRgnProc(proc) ((Ptr)proc)
typedef ProgressProcRecord ICMProgressProcRecord;
typedef ProgressProcRecordPtr ICMProgressProcRecordPtr;
typedef DataProcRecord ICMDataProcRecord;
typedef DataProcRecordPtr ICMDataProcRecordPtr;
#endif

#endif
