// PICT Object Handler for the WASTE Text Engine
// by Michael F. Kamprath, kamprath@earthlink.net
//
// v1.0, 12 March 1995
// v1.1, 5 June 1995, 	Added InstallPICTObject() to install the PICT handler
//						by itself.
//

#include "WASTE.h"
#include "WE_PICT_Handler.h"
#include "WASTE_Objects.h"

//
// Hanlder UPPs
//

static WENewObjectUPP           newPICTUPP = NULL;
static WEDisposeObjectUPP       disposePICTUPP = NULL;
static WEDrawObjectUPP          drawPICTUPP = NULL;

//
// InstallPICTObject()
//		Installs the PICT handler into WASTE.
//
OSErr	InstallPICTObject( WEHandle theWE )
{
OSErr	iErr;

	if (newPICTUPP == NULL)
	{
		newPICTUPP = NewWENewObjectProc(HandleNewPicture);
		disposePICTUPP = NewWEDisposeObjectProc(HandleDisposePicture);
		drawPICTUPP = NewWEDrawObjectProc(HandleDrawPicture);
	
		iErr = WEInstallObjectHandler(kTypePicture, weNewHandler, (UniversalProcPtr)newPICTUPP, theWE);
		if (iErr) return(iErr);
		iErr = WEInstallObjectHandler(kTypePicture, weDisposeHandler, (UniversalProcPtr)disposePICTUPP, theWE);
		if (iErr) return(iErr);
		iErr = WEInstallObjectHandler(kTypePicture, weDrawHandler, (UniversalProcPtr)drawPICTUPP, theWE);
		if (iErr) return(iErr);
	}
	
	return(noErr);
}

//
// New Object Handler for PICTs
//
pascal OSErr	HandleNewPicture(Point *defaultObjectSize,WEObjectReference objectRef)
{
PicHandle	thePic;
Rect		theFrame;

	thePic = (PicHandle)WEGetObjectDataHandle(objectRef);
	
	theFrame = (*thePic)->picFrame;
	
	OffsetRect(&theFrame, -theFrame.left, -theFrame.top);
	
	*defaultObjectSize = botRight(theFrame);
		
	return(noErr);
}

//
// Dispose Object Handler for PICTS
//
pascal OSErr	HandleDisposePicture(WEObjectReference objectRef )
{
PicHandle	thePic;

	thePic = (PicHandle)WEGetObjectDataHandle(objectRef);
	
	if (thePic)
		KillPicture(thePic);

	return(MemError());
}
//
// Draw Object Handler for PICTs
//
pascal OSErr	HandleDrawPicture (Rect *destRect, WEObjectReference objectRef )
{
PicHandle	thePic;
	
	thePic = (PicHandle)WEGetObjectDataHandle(objectRef);

	DrawPicture(thePic, destRect);
			
	return( noErr );
}