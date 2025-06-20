// File Object Handler for the WASTE Text Engine
// by Michael F. Kamprath, kamprath@earthlink.net
//
// v1.0, 12 March 1995
//
// v1.1, 1 April.  Fixed a a bug with draging hard drives to a WASTE instance
//                 and a bix with insert file objects via InsertFileRefFromFSSpec()
//

#include <Icons.h>
#include <Drag.h>
#include <LowMem.h>
#include "Waste.h"

#include "WE_hfs_Handler.h"
#include "WASTE_Objects.h"
#include "GetFileIcon.h"
#include "SendFinderOpen.h"

// Local Functions
static void		SizeHFSObject( Point *objectSize, HFSFlavor **theHFS );

//
// Handler UPPs
//

static WENewObjectUPP           newHFSUPP = NULL;
static WEDisposeObjectUPP       disposeHFSUPP = NULL;
static WEDrawObjectUPP          drawHFSUPP = NULL;
static WEClickObjectUPP         clickHFSUPP = NULL;

//
// InstallHFSObject()
//		Installs the HFS Object handler into WASTE.
//

OSErr	InstallHFSObject( WEHandle theWE )
{
OSErr	iErr;

	if (newHFSUPP == NULL)
	{
		newHFSUPP = NewWENewObjectProc(HandleNewHFS);
		disposeHFSUPP = NewWEDisposeObjectProc(HandleDisposeHFS);
		drawHFSUPP = NewWEDrawObjectProc(HandleDrawHFS);
		clickHFSUPP = NewWEClickObjectProc(HandleClickHFS);
	
		iErr = WEInstallObjectHandler(flavorTypeHFS, weNewHandler, (UniversalProcPtr)newHFSUPP, theWE);
		if (iErr) return(iErr);
		iErr = WEInstallObjectHandler(flavorTypeHFS, weDisposeHandler, (UniversalProcPtr)disposeHFSUPP, theWE);
		if (iErr) return(iErr);
		iErr = WEInstallObjectHandler(flavorTypeHFS, weDrawHandler, (UniversalProcPtr)drawHFSUPP, theWE);
		if (iErr) return(iErr);
		iErr = WEInstallObjectHandler(flavorTypeHFS, weClickHandler, (UniversalProcPtr)clickHFSUPP, theWE);
		if (iErr) return(iErr);
	
	}
	
	return(noErr);
}
//
// New Object Handler for HFS Objects
//
pascal OSErr	HandleNewHFS(Point *defaultObjectSize,WEObjectReference objectRef)
{
HFSFlavor	**theHFS = (HFSFlavor**)WEGetObjectDataHandle(objectRef);

	// First size the object as WASTE requires
	SizeHFSObject( defaultObjectSize, theHFS);
	
	// Set the object refcon to 0.  This is for safety
	// as later we use the refcon to store the icon handle
	// for this object.
	WESetObjectRefCon( objectRef, 0 );
	
	return(noErr);
}

//
// Dispose Object handler for HFS Objects
//
pascal OSErr	HandleDisposeHFS(WEObjectReference objectRef )
{
Handle	theHFS = WEGetObjectDataHandle(objectRef);
Handle	iconCache = (Handle)WEGetObjectRefCon( objectRef );

	// If the object has an icon handle loaded, dispose of it.
	if (iconCache != 0)
	{
		DisposeIconSuite( iconCache, true );
	}
	
	// Dispos of the HFS data
	DisposeHandle(theHFS);

	return(MemError());
}

//
// Draw Object Handler for HFS objects
//

pascal OSErr	HandleDrawHFS(Rect *destRect, WEObjectReference objectRef )
{
HFSFlavor	**theHFS = (HFSFlavor**)WEGetObjectDataHandle(objectRef);
Handle		iconCache = (Handle)WEGetObjectRefCon( objectRef );
FontInfo	fInfo;
GrafPtr		thePort;
Rect		iconRect;
OSErr		iErr;
short		oldTextFont, oldTextSize, oldTextFace;
short		sysTextFont = LMGetSysFontFam();
short		sysTextSize = LMGetSysFontSize();

	// if the object's refcon did not contain an icon handle, load one.
	if (iconCache == 0)
	{
		HLockHi( (Handle)theHFS );
		if ( ((*theHFS)->fileCreator=='MACS')&&((*theHFS)->fileType=='fold') )
			iErr = GetIconSuite(&iconCache,genericFolderIconResource,svAllAvailableData);
		else if ( ((*theHFS)->fileCreator=='MACS')&&((*theHFS)->fileType=='disk') )
		{
			iErr = GetIconSuite(&iconCache,genericHardDiskIconResource,svAllAvailableData);
		}
		else
		{
			iErr = GetFileIcon( &(*theHFS)->fileSpec, svAllAvailableData, &iconCache );
			
			if (iErr)
				iErr = GetIconSuite(&iconCache,genericDocumentIconResource,svAllAvailableData);
		}	
		HUnlock( (Handle)theHFS );
		if (iErr)
			return(iErr);
		WESetObjectRefCon( objectRef, (long)iconCache );
	}
	
	// Determine the icon's rectangle and plot it
	SetRect( &iconRect, (destRect->right + destRect->left)/2 - 16, destRect->top,
						(destRect->right + destRect->left)/2 + 16, destRect->top + 32 );
	iErr = PlotIconSuite(&iconRect,(IconAlignmentType)(atVerticalCenter + atHorizontalCenter),
											(IconTransformType)ttNone,	iconCache);
											
	// Draw the file's title.  First save old font info for port.
	GetPort(&thePort);
	oldTextFont = thePort->txFont;
	oldTextSize = thePort->txSize;
	oldTextFace = thePort->txFace;
	
	TextFont( sysTextFont );
	TextSize( sysTextSize );
	TextFace( 0 );
	GetFontInfo(&fInfo);
	
	MoveTo( destRect->left + 1, destRect->bottom - fInfo.descent - fInfo.leading );
	DrawString( (*theHFS)->fileSpec.name );
	
	TextFont( oldTextFont );
	TextSize( oldTextSize );
	TextFace( oldTextFace );
	
	return( iErr );
}

//
// Click Handler for HFS Objects.  
//		Will send the finder an open selection apple event when object is clicked.
//


pascal Boolean	HandleClickHFS(	Point hitPt, 
									short modifiers, 
									long clickTime, 
									WEObjectReference objectRef)
{
#pragma unused (hitPt, clickTime)

HFSFlavor	**theHFS;

	if (modifiers & 0x0001)         // look for double-clicks
	{
		theHFS = (HFSFlavor**)WEGetObjectDataHandle(objectRef);
		
		HLockHi( (Handle)theHFS );
		SendFinderOpenAE(&(*theHFS)->fileSpec);
		HUnlock( (Handle)theHFS );
		
		return(true);
	}
	else
		return(false);
}

//
// SizeHFSObject()
//		Used to return the display size of an HFS object
//
static void	SizeHFSObject( Point *objectSize, HFSFlavor **theHFS )
{
FontInfo	fInfo;
GrafPtr		thePort;
short		oldTextFont, oldTextSize, oldTextFace;
short		strWidth, strHeight;
short		sysTextFont = LMGetSysFontFam();
short		sysTextSize = LMGetSysFontSize();

	GetPort(&thePort);
	oldTextFont = thePort->txFont;
	oldTextSize = thePort->txSize;
	oldTextFace = thePort->txFace;
	
	TextFont( sysTextFont );
	TextSize( sysTextSize );
	TextFace( 0 );
	
	strWidth = StringWidth( (*theHFS)->fileSpec.name ) + 4;
	GetFontInfo(&fInfo);
	strHeight = fInfo.ascent + fInfo.descent + fInfo.leading;
	
	TextFont( oldTextFont );
	TextSize( oldTextSize );
	TextFace( oldTextFace );

	objectSize->h = ( strWidth > 32 ) ? strWidth : 32 ;
	objectSize->v = 32 + strHeight + 1;	
}

//
// InsertFileRefFromFSSpec()
//		Inserts a file object into a WASTE instance.
//

OSErr	InsertFileRefFromFSSpec( FSSpec *theFile, WEHandle theWE )
{
FInfo		fndrInfo;
HFSFlavor	**theHFS;
Point		theSize;
OSErr		iErr;
	
	theHFS = (HFSFlavor**)NewHandleClear( sizeof(HFSFlavor) );
	if (theHFS)
	{
		// First construct the HFSFlavor object from the FSSpec
		iErr = FSpGetFInfo(theFile,&fndrInfo);
		if (iErr)
		{
			DisposeHandle( (Handle)theHFS );
			return(iErr);
		}
		HLock( (Handle)theHFS );
		(*theHFS)->fileType = fndrInfo.fdType;
		(*theHFS)->fileCreator = fndrInfo.fdCreator;
		(*theHFS)->fdFlags = fndrInfo.fdFlags;
		(*theHFS)->fileSpec = (*theFile);
		HUnlock( (Handle)theHFS );
		
		SizeHFSObject( &theSize, theHFS );
		
		// Now insert it into the WEHandle
		WEInsertObject( flavorTypeHFS, (Handle)theHFS, theSize, theWE );
	}
	return(noErr);
}
