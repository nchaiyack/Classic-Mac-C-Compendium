/****
 * CBrowserDoc.c
 *
 *	Document methods for the Digest Browser application.
 *
 *  Copyright © 1990 Symantec Corporation.  All rights reserved.
 *  Copyright © 1991, 1992 Manuel A. PŽrez.  All rights reserved.
 *	Portions Copyright © 1992 J. Robert Boonstra, II.  All rights reserved.
 *
 ****/

#include "Global.h"
#include "Commands.h"
#include "CApplication.h"
#include "CBartender.h"
#include "CDecorator.h"
#include "CDesktop.h"
#include "CError.h"
#include "CPanorama.h"
#include "CBrowserScrollPane.h"	// JRB change from CScrollPane.h
#include "CBrowserDoc.h"
#include "TBUtilities.h"
#include "CWindow.h"
#include <Packages.h>
#include <string.h>

#include "BrowserCmds.h"

#define	WINDBrowser		500		/* Resource ID for WIND template */


extern	CApplication *gApplication;	/* The application */
extern	CBartender	*gBartender;	/* The menu handling object */
extern	CDecorator	*gDecorator;	/* Window dressing object	*/
extern	CDesktop	*gDesktop;		/* The enclosure for all windows */
extern	CBureaucrat	*gGopher;		/* The current boss in the chain of command */
extern	OSType		gSignature;		/* The application's signature */
extern	CError		*gError;		/* The global error handler */


/***
 * IBrowserDoc
 *
 *	Does nothing, just calls the CDocument IDocument method
 *
 ***/

void CBrowserDoc::IBrowserDoc(CApplication *aSupervisor, Boolean printable)

{
	CDocument::IDocument(aSupervisor, printable);

	/* Initialize the default index displayed when the document is first
	 * created.  Ideally, we should read this from a resource.
	 */
	index_displayed = cmdIndexSubject;
	itsListIndex = NULL;
	itsMessage = NULL;
}


/***
 * NewFile
 *
 * Empty implementation because we don't allow creation of new documents
 ***/
void CBrowserDoc::NewFile(void)
{ }    // This is correct, see note above.


/***
 * OpenFile
 *
 ***/

void CBrowserDoc::OpenFile(SFReply *macSFReply)

{
Str63		theName;
OSErr		theError;
BrowserDirPtr	dir;


	dir = Allocate(BrowserDir);
	brInitDir(dir);

	// Copy pascal name to a c name
	PtoCstr(macSFReply->fName);
	strcpy(dir->fname, (char *)&macSFReply->fName);
	CtoPstr(macSFReply->fName);

	// Copy the vref too
	dir->vRefNum = macSFReply->vRefNum;

	// Then let this program do the work
	BuildBrowserIndex(dir);		// this could fail
	BuildWindow(dir);
	itsWindow->SetTitle(macSFReply->fName);
	itsWindow->Select();			// Don't forget to make the window active
}

/***
 *
 * Dispose
 *
 ***/

void CBrowserDoc::Dispose()
{

	itsListIndex->Dispose();
	itsMessage->Dispose();

	inherited::Dispose();
}

/***
 * BuildWindow
 *
 *	This is the auxiliary window-building method that the
 *	NewFile() and OpenFile() methods use to create a window.
 *
 *	In this implementation, the argument is the data to display.
 *
 ***/

void CBrowserDoc::BuildWindow (BrowserDirPtr dir)
{
// JRB change - replace CScrollPane with CBrowserScrollPane and add new parameter
CBrowserScrollPane		*theScrollPane;
CBrowserScrollPane		*the2ndScrollPane;
LongRect		theAperture;
short			w;

			// const to help us understand the code
#define scrollerHeight		120
#define listHeight			(scrollerHeight-16)
#define contentsVLoc		(scrollerHeight+1)

	/**	First create the window and initialize it. **/

	itsWindow = new(CWindow);
	itsWindow->IWindow(WINDBrowser, FALSE, gDesktop, this);
	itsWindow->GetAperture(&theAperture);
	w = theAperture.bottom - theAperture.top - scrollerHeight;

	/** Create a scroll pane for the index field **/
// JRB change - replace ScrollPane with BrowserScrollPane and add new parameter
    theScrollPane = new(CBrowserScrollPane);
	theScrollPane->IBrowserScrollPane(itsWindow, this, 10, scrollerHeight, 0, 0,
								sizELASTIC, sizFIXEDSTICKY,
								TRUE, TRUE, TRUE, 	// JRB - last arg was FALSE
								TRUE);				// JRB, SICN size box option
	theScrollPane->FitToEnclFrame(TRUE, FALSE);


	/**
	 **	itsMainPane is the document's focus
	 **	of attention. Some of the standard
	 **	classes (particularly CPrinter) rely
	 **	on itsMainPane pointing to the main
	 **	pane of your window.
	 **
	 **	itsGopher specifies which object
     ** should become the gopher when the document
     ** becomes active. By default
	 **	the document becomes the gopher. ItÕs
	 **	likely that your main pane handles commands
	 **	so youÕll almost always want to set itsGopher
	 **	to point to the same object as itsMainPane.
	 **
	 **/

	/** Create a DisplayIndex (top of the window) */
	itsListIndex = new(CDisplayIndex);
	itsGopher = itsListIndex;

	itsListIndex->IDisplayIndex(theScrollPane, this, 0, listHeight, dir, index_displayed);
	theScrollPane->InstallPanorama(itsListIndex);


	//**
	// Build the contents of the message display
	//**

    the2ndScrollPane = new(CBrowserScrollPane);
	the2ndScrollPane->IBrowserScrollPane(itsWindow, this,
			10, w,					// sizes
			0, contentsVLoc,		// locations
			sizELASTIC, sizELASTIC,	// sizes types
			TRUE, TRUE, TRUE,		// horiz, vert, and size box 
			FALSE);				// JRB, no SICN size box
	the2ndScrollPane->FitToEnclFrame(TRUE, FALSE);		// horiz, vert

	itsMessage = new(CDisplayText);
	itsMessage->IDisplayText(the2ndScrollPane, this, contentsVLoc, 1);
	the2ndScrollPane->InstallPanorama(itsMessage);

	// itsMainPane is used for printing, so set the message (not the index)
	// field as the main pane.
	itsMainPane = itsMessage;


	//
	// Establish dependency between the two displays, by making
	// the message display be dependent upon the selection on
	// the index display
	//
	itsMessage->DependUpon(itsListIndex);

// JRB addition - following line guarantees first index is selected when doc is opened
	itsListIndex->SetSelectedLine(0, true);

	gDecorator->PlaceNewWindow(itsWindow);
}

/***
 *
 * WriteTagged
 *
 ***/

OSErr CBrowserDoc::WriteTagged(short fileRef)
{
OSErr myErr;
BrowserDirPtr	dir;
BrowserItemPtr	p;
Handle text;
long len;
long offset;

	myErr = noErr;
	dir = itsListIndex->GetDirectory();
	for (p = dir->topItem; p != NULL; p = brGetNext(p)) {

		if (brGetMark(p)) {

			// Write this item to the file

			if (myErr == noErr) {
				len = brGetEnd(p) - brGetStart(p);
				FailNIL(text = NewHandle(len));		// fail if no memory
				HLock(text);
				fseek(brGetFP(p), brGetStart(p), 0);		// read in text
				fread(*text, 1, len, brGetFP(p));

				offset = 0;							// replace '\n' with '\r'
				do {
					offset = Munger(text, offset, "\n", 1, "\r", 1);
				} while (offset > 0);

				myErr = FSWrite(fileRef, &len, *text);

				HUnlock(text);
				DisposHandle(text);					// release handle 
			}
		}
	}

	return myErr;
}

/***
 * DoCommand {OVERRIDE}
 *
 *	Execute a command
 *
 ***/

void	CBrowserDoc::DoCommand(long theCommand)
{

	switch (theCommand) {

		case cmdSaveTo:
			SaveTaggedTo();
			break;

		case cmdAppendTo:
			AppendTaggedTo();
			break;

		case cmdClearMarkedItems:
			ClearTagged();
			break;

		case cmdMarkItem:
			TagItem();
			break;

		case cmdIndexFrom:
		case cmdIndexDate:
		case cmdIndexSubject:
		case cmdIndexComposite:		// JRB addition
			if (theCommand != index_displayed) {
				// Store index in an instance variable
				index_displayed = theCommand;

				// Set the index in the index display
				itsListIndex->SetIndex(theCommand);
			}
			break;

		default:
			inherited::DoCommand(theCommand);
			break;
	}
}

/***
 * UpdateMenus {OVERRIDE}
 *
 *	Enable all Index items, and place a check mark next to the current
 *  one.  Enable the save and append commands.
 *
 ***/

void	CBrowserDoc::UpdateMenus()
{
BrowserDirPtr theDir;
BrowserItemPtr	selItem;

    inherited::UpdateMenus();

	// Enable saving only if the document is dirty
	if (dirty) {
		gBartender->EnableCmd(cmdSaveTo);
		gBartender->EnableCmd(cmdAppendTo);
	}

	theDir = itsListIndex->GetDirectory();
	if (brMarkCount(theDir) > 0)
		gBartender->EnableCmd(cmdClearMarkedItems);

	// Enable the Mark Item menu accordingly
	gBartender->SetCmdText(cmdMarkItem, "\pMark Item");
	if (itsListIndex->GetSelectedLine() >= 0) {
		gBartender->EnableCmd(cmdMarkItem);
		selItem = itsListIndex->GetSelectedItem();
		if (brGetMark(selItem))
			gBartender->SetCmdText(cmdMarkItem, "\pUnmark Item");
	}


	gBartender->EnableCmd(cmdIndexFrom);
	gBartender->EnableCmd(cmdIndexDate);
	gBartender->EnableCmd(cmdIndexSubject);
	gBartender->EnableCmd(cmdIndexComposite);	// JRB addition

	gBartender->CheckMarkCmd(cmdIndexFrom, index_displayed == cmdIndexFrom);
	gBartender->CheckMarkCmd(cmdIndexDate, index_displayed == cmdIndexDate);
	gBartender->CheckMarkCmd(cmdIndexSubject, index_displayed == cmdIndexSubject);
	// JRB addition
	gBartender->CheckMarkCmd(cmdIndexComposite, index_displayed == cmdIndexComposite);
}

/***
 * SaveTaggedTo
 ***/

void CBrowserDoc::SaveTaggedTo(void)
{
StandardFileReply reply;
OSErr myErr;
short fref;
short vol;
short source;
Str255 prompt, name;

	GetIndString(prompt, 1026, 1);
	GetIndString(name, 1026, 2);
	StandardPutFile(prompt, name, &reply);
	if (reply.sfGood) {
		myErr = noErr;

		source = CurResFile();		/* save current resource file */
		if (!reply.sfReplacing)
			myErr = FSpCreate(&reply.sfFile, gSignature, 'TEXT', smSystemScript);


#ifdef COPY_RSRC
		/* Create document's resource fork and copy Finder Resources to it. */
		if (myErr == noErr) {
			FSpCreateResFile(&reply.sfFile, gSignature, 'TEXT', smSystemScript);
			myErr = ResError();
		}
		
		if (myErr == noErr)
			fref = FSpOpenResFile(&reply.sfFile, fsWrPerm);

		if (fref > 0)
			myErr = DoCopyResource('STR ', -16396, source, fref);
		else
			myErr = ResError();

		if (myErr == noErr)
			myErr = FSClose(fref);
#endif

		/* Write our data to the data fork side */
		if (myErr == noErr)
			myErr = FSpOpenDF(&reply.sfFile, fsWrPerm, &fref);

		if (myErr == noErr)
			myErr = SetFPos(fref, fsFromStart, 0);

		if (myErr == noErr)
			myErr = WriteTagged(fref);

		if (myErr == noErr)
			myErr = GetVRefNum(fref, &vol);

		if (myErr == noErr)
			myErr = FlushVol(NULL, vol);

		if (myErr == noErr)
			myErr = FSClose(fref);


		if (myErr == noErr)
			dirty = false;
	}
}

OSErr CBrowserDoc::DoCopyResource(ResType theType, short theId,
	short source, short dest)
{
Handle myHandle;
Str255 myName;
ResType myType;
short myID;
OSErr err = noErr;

	UseResFile(source);
	myHandle = GetResource(theType, theId);
	if (myHandle != NULL) {
		GetResInfo(myHandle, &myID, &myType, myName);
		DetachResource(myHandle);
		UseResFile(dest);
		AddResource(myHandle, theType, theId, myName);
		if (ResError() == noErr)
			WriteResource(myHandle);			/* write resource data */
		err = ResError();
		ReleaseResource(myHandle);
	}
	return err;
}

/***
 * AppendTaggedTo
 ***/

void CBrowserDoc::AppendTaggedTo(void)
{
SFTypeList typeList;
StandardFileReply reply;
OSErr myErr;
short fref;
short vol;

	typeList[0] = 'TEXT';
	StandardGetFile(NULL, 1, typeList, &reply);
	if (reply.sfGood) {
		
		myErr = noErr;
		if (myErr == noErr)
			myErr = FSpOpenDF(&reply.sfFile, fsWrPerm, &fref);

		if (myErr == noErr)
			myErr = SetFPos(fref, fsFromLEOF, 0);

		if (myErr == noErr)
			myErr = WriteTagged(fref);

		if (myErr == noErr)
			myErr = GetVRefNum(fref, &vol);

		if (myErr == noErr)
			myErr = FlushVol(NULL, vol);

		if (myErr == noErr)
			myErr = FSClose(fref);

		if (myErr == noErr)
			dirty = false;
	}
}

/***
 * TagItem
 ***/

void CBrowserDoc::TagItem(void)
{
BrowserDirPtr theDir;

	itsListIndex->TagItem();
	theDir = itsListIndex->GetDirectory();

	// set the dirty flag
	dirty = (brMarkCount(theDir) > 0);		// dirty == marked items
}

/***
 * ClearTagged
 ***/
void CBrowserDoc::ClearTagged()
{
BrowserDirPtr theDir;
BrowserItemPtr p;
long i;

	theDir = itsListIndex->GetDirectory();

	for (i = 0, p = theDir->topItem; p != NULL; i++, p = brGetNext(p))
		if (brGetMark(p)) {	// turn them all off
			itsListIndex->SetSelectedLine(i, false);
			itsListIndex->TagItem();
		}

	// set the dirty flag
	dirty = false;		// no items tagged, document is clean
}

/***
 *
 * AdjustPaneDivider
 *
 ***/

// JRB addition to adjust division between panes
void CBrowserDoc::AdjustPaneDivider(Point hitPt, Rect *theDragBox)	/* already in CPane coords */
{
RgnHandle		theRgn;
LongRect		theLongFrameBox;
Rect			theFrameBox,theLimitRect,changeSizeRect;
long			offsetPt;
short 			vOffset;

	theRgn=NewRgn();
	
	itsWindow->Prepare();	
	
	itsWindow->GetFrame(&theLongFrameBox);
	theFrameBox=itsMainPane->macPort->portRect;
	ClipRect(&theFrameBox);

	theDragBox->left = theFrameBox.left;
	theDragBox->right = theFrameBox.right;
	theLimitRect = theFrameBox;
	InsetRect(&theLimitRect,0,48);
	
	RectRgn(theRgn,theDragBox);
	offsetPt=DragGrayRgn(theRgn,hitPt,&theLimitRect,&theFrameBox,vAxisOnly,0L);
	DisposeRgn(theRgn);

	vOffset = ((Point *)&offsetPt)->v;
	if (offsetPt!=0x80008000) {		// if user stayed within limitRect ...
		
		SetRect(&changeSizeRect,0,vOffset,0,0);
		((CBrowserScrollPane*)itsMessage->itsScrollPane)->ChangeSize(&changeSizeRect,true);
//			ChangeSize with a CEditText instead of a CBrowserEditText caused the itsMessage
//			pane to be offset and resizes the bottom of the pane as well as the top, so
//			CBrowserEditText prevents this and offsets the viewRect to compensate.
//			Probably my error in using the class library, but I couldn't find another way
//			around it, so I subclassed CScrollPane. 	-- JRB
	
		SetRect(&changeSizeRect,0,0,0,vOffset);
		((CBrowserScrollPane*)itsListIndex->itsScrollPane)->ChangeSize(&changeSizeRect,true);
	}
}
					

