//=====================================================================================
// JPEGUtilities.c -- written by Aaron Giles
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

//=====================================================================================
// Generic includes for Macintosh headers
//=====================================================================================

#include <Errors.h>

//=====================================================================================
// Includes specific to this module
//=====================================================================================

#include "JPEGUtilities.h"

//=====================================================================================
// Global variables local to this module
//=====================================================================================

static Handle gJPEGDestHandle;
static StdPixUPP gUnwrapPixProc = nil;
static QDBitsUPP gDummyBitsProc;
static QDTextUPP gDummyTextProc;
static QDLineUPP gDummyLineProc;
static QDRectUPP gDummyRectProc;
static QDRRectUPP gDummyRRectProc;
static QDOvalUPP gDummyOvalProc;
static QDArcUPP gDummyArcProc;
static QDPolyUPP gDummyPolyProc;
static QDRgnUPP gDummyRgnProc;

//=====================================================================================
// Prototypes for functions local to this module
//=====================================================================================

static void InitJPEGUPPs(void);
static unsigned char *GetJPEGData(unsigned char *theAdr, long theLen, unsigned char theCode);
static OSErr MakeJPEGImageDesc(Handle theHandle, ImageDescriptionHandle theDesc);
static pascal void UnwrapPixProc(PixMap *src, Rect *srcRect, MatrixRecord *matrix,
			short mode, RgnHandle mask, PixMap *matte, Rect *matteRect, 
			short callOldBits);
static pascal void DummyBitsProc(PixMap *src, Rect *srcRect, Rect *dstRect, short mode, 
			RgnHandle mask);
static pascal void DummyTextProc(short count, const void *textAddr, Point numer,
			Point denom);
static pascal void DummyLineProc(Point newPt);
static pascal void DummyRectProc(GrafVerb verb, const Rect *r);
static pascal void DummyRRectProc(GrafVerb verb, const Rect *r, short ovalWidth, 
			short ovalHeight);
static pascal void DummyOvalProc(GrafVerb verb, const Rect *r);
static pascal void DummyArcProc(GrafVerb verb, const Rect *r, short startAngle,
			short arcAngle);
static pascal void DummyPolyProc(GrafVerb verb, PolyHandle poly);
static pascal void DummyRgnProc(GrafVerb verb, RgnHandle rgn);

//=====================================================================================
// OSErr GetJPEGBounds(Handle theHandle, Rect *theBounds)
//=====================================================================================
// Scans the JPEG image data in theHandle for the bounding rectangle, and returns the
// result in theBounds.  If the code specifying the bounds of the image was not found,
// this function returns codecBadDataErr.
//=====================================================================================

extern OSErr GetJPEGBounds(Handle theHandle, Rect *theBounds)
{
	long theLen = GetHandleSize(theHandle);
	unsigned char *theData;
	short theCode;
	
	for (theCode = 0xc0; theCode < 0xd0; theCode++)
		if (theData = GetJPEGData((unsigned char *)*theHandle, theLen, theCode)) break;
	if (theCode == 0xd0) return codecBadDataErr;
	theBounds->top = theBounds->left = 0;
	theBounds->right = (theData[3] << 8) + theData[4];
	theBounds->bottom = (theData[1] << 8) + theData[2];
	return noErr;
}

//=====================================================================================
// OSErr DrawJPEG(Handle theHandle, Rect *srcRect, Rect *dstRect, short tMode)
//=====================================================================================
// Draws a JPEG image, whose data is in theHandle, to the current port.  The portion of
// the image to be drawn is specified in srcRect, and any scaling or translation can be
// specified through dstRect.  The transfer mode for drawing is given in the tMode
// parameter.
//=====================================================================================

extern OSErr DrawJPEG(Handle theHandle, Rect *srcRect, Rect *dstRect, short tMode)
{
	PixMapHandle dPixMap = GetGWorldPixMap((CGrafPtr)qd.thePort);
	long theSize = GetHandleSize(theHandle);
	char hState = HGetState(theHandle);
	ImageDescriptionHandle theDesc;
	MatrixRecord theMatrix;
	OSErr theErr;

	if (theDesc = (ImageDescriptionHandle)NewHandleClear(sizeof(ImageDescription))) {
		HLock((Handle)theDesc);
		theErr = MakeJPEGImageDesc(theHandle, theDesc);
		if (theErr == noErr) {
			SetIdentityMatrix(&theMatrix);
			MapMatrix(&theMatrix, srcRect, dstRect);
			HLock(theHandle);
			theErr = FDecompressImage(StripAddress(*theHandle), theDesc, dPixMap,
						srcRect, &theMatrix, tMode, qd.thePort->visRgn, nil, nil,
						codecMaxQuality, anyCodec, theSize, nil, nil);
			HSetState(theHandle, hState);
		}
		DisposeHandle((Handle)theDesc);
	} else theErr = memFullErr;
	return theErr;
}

//=====================================================================================
// Handle UnwrapJPEG(Handle srcHandle)
//=====================================================================================
// Extracts (unwraps) the original JPEG data from a JPEG PICT image specified in
// srcHandle.  If this function fails, it returns a nil handle.
//=====================================================================================

extern Handle UnwrapJPEG(PicHandle srcHandle)
{
	long theSize = GetHandleSize((Handle)srcHandle);
	char hState = HGetState((Handle)srcHandle);
	Rect bounds = (*srcHandle)->picFrame;
	CGrafPort tempPort;
	CQDProcs theProcs;
	GrafPtr oldPort;

	gJPEGDestHandle = nil;
	HLockHi((Handle)srcHandle);
	GetPort(&oldPort);
	OpenCPort(&tempPort);
	SetPort((GrafPtr)&tempPort);
	SetStdCProcs(&theProcs);
	if (!gUnwrapPixProc) InitJPEGUPPs();
	theProcs.newProc1 = (UniversalProcPtr)gUnwrapPixProc;
	theProcs.bitsProc = gDummyBitsProc;
	theProcs.textProc = gDummyTextProc;
	theProcs.lineProc = gDummyLineProc;
	theProcs.rectProc = gDummyRectProc;
	theProcs.rRectProc = gDummyRRectProc;
	theProcs.ovalProc = gDummyOvalProc;
	theProcs.arcProc = gDummyArcProc;
	theProcs.polyProc = gDummyPolyProc;
	theProcs.rgnProc = gDummyRgnProc;
	tempPort.grafProcs = &theProcs;
	DrawPicture(srcHandle, &bounds);
	SetPort(oldPort);
	CloseCPort(&tempPort);
	HSetState((Handle)srcHandle, hState);
	return gJPEGDestHandle;
}

//=====================================================================================
// Handle WrapJPEG(Handle srcHandle)
//=====================================================================================
// Wraps a JPEG image into a PICT.  If this function fails, it returns a nil handle.
//=====================================================================================

extern PicHandle WrapJPEG(Handle srcHandle)
{
	long theSize = GetHandleSize(srcHandle);
	char hState = HGetState(srcHandle);
	ImageDescriptionHandle theDesc;
	OpenCPicParams theParams;
	PicHandle theHandle;
	CGrafPort tempPort;
	GrafPtr oldPort;
	OSErr theErr;

	HLock(srcHandle);
	if (theDesc = (ImageDescriptionHandle)NewHandleClear(sizeof(ImageDescription))) {
		theErr = MakeJPEGImageDesc(srcHandle, theDesc);
		if (theErr == noErr) {
			theParams.srcRect.top = theParams.srcRect.left = 0;
			theParams.srcRect.right = (*theDesc)->width;
			theParams.srcRect.bottom = (*theDesc)->height;
			theParams.hRes = (*theDesc)->hRes;
			theParams.vRes = (*theDesc)->vRes;
			theParams.version = -2;
			theParams.reserved1 = theParams.reserved2 = 0;
			GetPort(&oldPort);
			OpenCPort(&tempPort);
			SetPort((GrafPtr)&tempPort);
			ClipRect(&theParams.srcRect);
			if (theHandle = OpenCPicture(&theParams)) {
				theErr = FDecompressImage(StripAddress(*srcHandle), theDesc, 
							GetGWorldPixMap(&tempPort), &theParams.srcRect, nil, ditherCopy,
							nil, nil, nil, codecMaxQuality, anyCodec, theSize, nil, nil);
				ClosePicture();
				if (theErr == noErr && !EmptyRect(&(*(PicHandle)theHandle)->picFrame)) {
					CloseCPort(&tempPort);
					SetPort(oldPort);
					DisposeHandle((Handle)theDesc);
					HSetState(srcHandle, hState);
					return theHandle;
				}
				DisposeHandle((Handle)theHandle);
			} else theErr = memFullErr;
			CloseCPort(&tempPort);
			SetPort(oldPort);
		}
		DisposeHandle((Handle)theDesc);
	} else theErr = memFullErr;
	HSetState(srcHandle, hState);
	return nil;
}

//=====================================================================================
// void InitJPEGUPPs(void)
//=====================================================================================
// Initializes all the local UPPs for the JPEG callbacks.
//=====================================================================================

static void InitJPEGUPPs(void)
{
	gUnwrapPixProc = NewStdPixProc((ProcPtr)UnwrapPixProc);
	gDummyBitsProc = NewQDBitsProc((ProcPtr)DummyBitsProc);
	gDummyTextProc = NewQDTextProc((ProcPtr)DummyTextProc);
	gDummyLineProc = NewQDLineProc((ProcPtr)DummyLineProc);
	gDummyRectProc = NewQDRectProc((ProcPtr)DummyRectProc);
	gDummyRRectProc = NewQDRRectProc((ProcPtr)DummyRRectProc);
	gDummyOvalProc = NewQDOvalProc((ProcPtr)DummyOvalProc);
	gDummyArcProc = NewQDArcProc((ProcPtr)DummyArcProc);
	gDummyPolyProc = NewQDPolyProc((ProcPtr)DummyPolyProc);
	gDummyRgnProc = NewQDRgnProc((ProcPtr)DummyRgnProc);
}

//=====================================================================================
// unsigned char *GetJPEGData(unsigned char *theAdr, long theLen, unsigned char theCode)
//=====================================================================================
// Returns the address of the specified marker within the JPEG data stream.
//=====================================================================================

static unsigned char *GetJPEGData(unsigned char *theAdr, long theLen, unsigned char theCode) 
{
	unsigned char *theEnd = theAdr + theLen;
	long theSize;
	
	while (true) {
		while (*theAdr++ != 0xff && theAdr < theEnd);
		if (theAdr >= theEnd) return nil;
		while (*theAdr == 0xff) theAdr++;
		if (theAdr >= theEnd) return nil;
		if (*theAdr == theCode) break;
		else if (*theAdr == 0xd9) return nil;
		else if (*theAdr <= 0x01 || (*theAdr >= 0xd0 && *theAdr <= 0xd8)) {
			theAdr++;
			continue;
		}
		theAdr++;
		if ((theSize = (*theAdr << 8) + *(theAdr + 1)) < 0) return nil;
		if ((theAdr + theSize) >= theEnd) {
			*(theAdr - 1) = 0xd9;
			return nil;
		} else theAdr += theSize;
	}
	theAdr += 3;
	return theAdr;
}

//=====================================================================================
// OSErr MakeJPEGImageDesc(Handle theHandle, ImageDescriptionHandle theDesc)
//=====================================================================================
// Creates a QuickTime image description record based on JPEG data.
//=====================================================================================

static OSErr MakeJPEGImageDesc(Handle theHandle, ImageDescriptionHandle theDesc) 
{
	CodecInfo theInfo;
	OSErr theErr;
	Rect bounds;
	
	theErr = GetCodecInfo(&theInfo, 'jpeg', anyCodec);
	if (theErr != noErr) return theErr;
	theErr = GetJPEGBounds(theHandle, &bounds);
	if (theErr != noErr) return theErr;
	(*theDesc)->idSize = sizeof(ImageDescription);
	(*theDesc)->cType = 'jpeg';
	(*theDesc)->resvd1 = (*theDesc)->resvd2 = (*theDesc)->dataRefIndex = 0;
	BlockMove(theInfo.typeName, (*theDesc)->name, 32);
	(*theDesc)->version = theInfo.version;
	(*theDesc)->revisionLevel = theInfo.revisionLevel;
	(*theDesc)->vendor = theInfo.vendor;
	(*theDesc)->temporalQuality = 0;
	(*theDesc)->spatialQuality = 0x200L;
	(*theDesc)->width = bounds.right;
	(*theDesc)->height = bounds.bottom;
	(*theDesc)->hRes = (*theDesc)->vRes = 0x480000L;
	(*theDesc)->dataSize = GetHandleSize(theHandle);
	(*theDesc)->frameCount = 1;
	(*theDesc)->depth = 32;
	(*theDesc)->clutID = -1;
	return theErr;
}

//=====================================================================================
// pascal void UnwrapPixProc(PixMap *src, Rect *srcRect, MatrixRecord *matrix,
//			short mode, RgnHandle mask, PixMap *matte, Rect *matteRect,
//			short callOldBits)
//=====================================================================================
// Bottleneck function called to extract raw JPEG data from an image.
//=====================================================================================

static pascal void UnwrapPixProc(PixMap *src, Rect *srcRect, MatrixRecord *matrix,
			short mode, RgnHandle mask, PixMap *matte, Rect *matteRect,
			short callOldBits)
{
#if applec
#pragma unused(srcRect, matrix, mode, mask, matte, matteRect, callOldBits)
#endif
	long bufSize, left, offset = 0;
	ImageDescriptionHandle theDesc;
	ICMProgressProcRecord progProc;
	ICMDataProcRecord dataProc;
	Ptr theData, destAdr;
	
	if (!GetCompressedPixMapInfo(src, &theDesc, &theData, &bufSize, &dataProc, &progProc)) {
		if ((*theDesc)->cType == 'jpeg' && !gJPEGDestHandle) {
			if (gJPEGDestHandle = NewHandle((*theDesc)->dataSize)) {
				HLock(gJPEGDestHandle);
				destAdr = (Ptr)*gJPEGDestHandle;
				left = (*theDesc)->dataSize;
				if (dataProc.dataProc) {
					while (left > bufSize) {
						BlockMove(theData, &destAdr[offset], bufSize);
						offset += bufSize;
						left -= bufSize;
						theData += bufSize;
						if (CallICMDataProc(dataProc.dataProc, &theData, 
									(left > bufSize) ? bufSize : left, 
									dataProc.dataRefCon)) return;
					}
				}
				BlockMove(theData, &destAdr[offset], left);
				HUnlock(gJPEGDestHandle);
			}
		}
	}
}

//=====================================================================================
// Dummy QuickDraw bottlenecks to filter out non-PixMaps within PICTs
//=====================================================================================

static pascal void DummyBitsProc(PixMap *src, Rect *srcRect, Rect *dstRect, short mode, 
			RgnHandle mask) 
{
#if applec
#pragma unused(src, srcRect, dstRect, mode, mask)
#endif
}

static pascal void DummyTextProc(short count, const void *textAddr, Point numer,
			Point denom) 
{
#if applec
#pragma unused(count, textAddr, numer, denom)
#endif
}

static pascal void DummyLineProc(Point newPt)
{
#if applec
#pragma unused(newPt)
#endif
}

static pascal void DummyRectProc(GrafVerb verb, const Rect *r)
{
#if applec
#pragma unused(verb, r)
#endif
}

static pascal void DummyRRectProc(GrafVerb verb, const Rect *r, short ovalWidth, 
	short ovalHeight)
{
#if applec
#pragma unused(verb, r, ovalWidth, ovalHeight)
#endif
}

static pascal void DummyOvalProc(GrafVerb verb, const Rect *r)
{
#if applec
#pragma unused(verb, r)
#endif
}

static pascal void DummyArcProc(GrafVerb verb, const Rect *r, short startAngle,
			short arcAngle)
{
#if applec
#pragma unused(verb, r, startAngle, arcAngle)
#endif
}

static pascal void DummyPolyProc(GrafVerb verb, PolyHandle poly)
{
#if applec
#pragma unused(verb, poly)
#endif
}

static pascal void DummyRgnProc(GrafVerb verb, RgnHandle rgn)
{
#if applec
#pragma unused(verb, rgn)
#endif
}
