/* SFMultiGet.c */

#include <StandardFile.h>
#include <Resources.h>
#include <Memory.h>
#include <Lists.h>
#include <Packages.h>
#include <Files.h>
#include <Controls.h>
#include "SFMultiGet.h" /* Mine */
#include "CSack.h" /* Mine */

/*
	Hi there!
	
	After craving a multi-get dialog box for a while, I finally got the time
	to write one. And since I could never find one on-line, I'm uploading it
	far and wide (or at least to cis/aol/sumex). It works, and _should_ work
	under MPW & Think. The methods are a little ugly (stringtonum), but hey,
	you can write your own if you want.
	
	Potential improvements:
		� Filtering out the files already in the list. This doesn't really
		  bother me, but if you want, the fileFilter function is ready.
		� Better System 6 behavior. Actually, I'm not sure what the thing
		  looks like under 6. Probably the auto-centering is off, the
		  button isn't hilited, & the enter keys don't work.
	
	If you use this, feel free to send me a copy of the program ;)
	
	Manuel Veloso
	9 High Rock Way #3
	Allston, MA 02134-2414
	
	cis: 70365, 1426
	aol: Fish26
	net: veloso@husc10.harvard.edu
	
	History:
	1.0	same as current, except without the scrollbar.
	1.1	added a scrollbar.
*/

#define SFSaveDisk 	0x214
#define CurDirStore 0x398
	
enum
{
	okButton = 1,
	blank2,
	cancelButton,
	item1,
	ejectButton,
	driveButton,
	sfListBox,
	scrollBar,
	line1,
	blank10,
	filesBox,
	removeButton,
	addButton,
	line,
	myScrollBar,
	kMyDialog = 128
};

ListHandle	gTheList;			// list of files selected
SFReply		gTheReply;			// the sf reply
ControlHandle	gTheScrollBar;	// the scrollbar

/*
	NOTE: the refCon of the control holds the # of files in the list AND the # of files
	that are visible. The refCon starts out as a negative #; when a file is added, the
	refCon is incremented. When refCon == 1, that means that scrolling must take place.
	When a file is removed, the refCon is decremented by 1. Presumably, when all files
	are removed the refCon == the original value.
*/

pascal void setItem(DialogPtr theDialog, short item, Handle theProc);
pascal void hiliteItem(DialogPtr theDialog, short item, short hiliteValue);
pascal void getBox(DialogPtr theDialog, short item, Rect *box);
void addData(void);
void removeCells(void);
Boolean selected(void);
pascal void myItem(DialogPtr theDialog, short itemNumber);
pascal void initList(DialogPtr theDialog);
pascal Boolean myFileFilter(CInfoPBPtr pb);
pascal short myDlgHook(short item, DialogPtr theDialog);
pascal Boolean myFilterProc(DialogPtr theDialog, EventRecord *event, short *itemHit);
pascal void cleanup(void);
void processData(void);
void handleScrollBar(DialogPtr, EventRecord*);
pascal void scrollProc(ControlHandle theControl, short theCode);

pascal void setItem(DialogPtr theDialog, short item, Handle theProc)
{
	short	itemType;
	Rect	box;
	Handle	theControl;
	
	GetDItem(theDialog, item, &itemType, &theControl, &box);
	SetDItem(theDialog, item, itemType, theProc, &box);
}
	
pascal void hiliteItem(DialogPtr theDialog, short item, short hiliteValue)
{
	short	itemType;
	Rect	box;
	Handle	theControl;
	
	GetDItem(theDialog, item, &itemType, &theControl, &box);
	HiliteControl((ControlHandle) theControl, hiliteValue);
}

pascal void getBox(DialogPtr theDialog, short item, Rect *box)
{
	short	itemType;
	Handle	theControl;
	
	GetDItem(theDialog, item, &itemType, &theControl, box);
}

/*
	Initializes the list within the sfdialog. Sets the useritem to the
	updateProc.
*/

pascal void initList(DialogPtr theDialog)
{
	Rect		rView, dataBounds;
	Point		cSize = {0, 0};
	short		temp;
	FontInfo	fInfo;
	GrafPtr		oldPort;
	Boolean		good = false;

	GetPort(&oldPort);
	SetPort(theDialog);
	getBox(theDialog, filesBox, &rView);
	SetRect(&dataBounds, 0, 0, 3, 0);
	GetFontInfo(&fInfo);
	cSize.h = rView.right;
	cSize.v = fInfo.ascent + fInfo.descent + fInfo.leading;
	gTheList = LNew(&rView, &dataBounds, cSize, 0, theDialog, true, false, false, false);
	if (gTheList)
	{
		LDoDraw(true, gTheList);
		(*gTheList)->selFlags = lNoNilHilite + lUseSense + lOnlyOne;
		(*gTheList)->listFlags = lDoVAutoscroll;
		LActivate(true, gTheList);
		hiliteItem(theDialog, removeButton, 255);
		setItem(theDialog, filesBox, (Handle) myItem);
	}
	hiliteItem(theDialog, myScrollBar, 255);
	GetDItem(theDialog, myScrollBar, &temp, (Handle*)&gTheScrollBar, &dataBounds);
	SetCRefCon(gTheScrollBar, -((rView.bottom - rView.top)/cSize.v));	// # of lines in box, negativized
	SetCtlMin(gTheScrollBar, 0);
	SetCtlMax(gTheScrollBar, GetCRefCon(gTheScrollBar));
	SetPort(oldPort);
	return;
}

pascal void cleanup(void)
{
	if (gTheList) LDispose(gTheList);
}

/*
	Updates the list display, and does sundry drawing activities.
*/

pascal void myItem(DialogPtr theDialog, short itemNumber)
{
	Rect	box;
	GrafPtr	oldPort;
	
	GetPort(&oldPort);
	SetPort(theDialog);
	
	getBox(theDialog, filesBox, &box);
	InsetRect(&box, -1, -1);
	FrameRect(&box);
	getBox(theDialog, line, &box);
	FrameRect(&box);
	LUpdate(theDialog->visRgn, gTheList);
	SetPort(oldPort);
}

// false displays the file, true doesn�t.

pascal Boolean myFileFilter(CInfoPBPtr pb)
{
	return false;
}

/*
	must return item #; if no number, nothing happens. I've remapped the
	add button to the okButton, and vice-versa to avoid lots of exess code.
*/

pascal short myDlgHook(short item, DialogPtr theDialog)
{
	switch (item)
	{
		case sfHookFirstCall:
			initList(theDialog);
			break;
		case okButton:			// maps to add!
			addData();
			if (GetCRefCon(gTheScrollBar)>0) hiliteItem(theDialog, myScrollBar, 0);
			else hiliteItem(theDialog, myScrollBar, 255);
			item = addButton;
			break;
		case addButton:			// maps to ok!
			item = okButton;
			break;
		case removeButton:
			removeCells();
			hiliteItem(theDialog, removeButton, 255);
			if (GetCRefCon(gTheScrollBar)>0) hiliteItem(theDialog, myScrollBar, 0);
			else hiliteItem(theDialog, myScrollBar, 255);
			break;
	}
	return item;
}

// false->handle the event || true->ignore the event

pascal Boolean myFilterProc(DialogPtr theDialog, EventRecord *event, short *itemHit)
{
	if (event->what == mouseDown)
	{
		if (FrontWindow() == theDialog)
		{
			Rect	theView;
			getBox(theDialog, filesBox, &theView);
			GlobalToLocal(&(event->where));
			if (PtInRect(event->where, &theView))
			{
				LClick(event->where, event->modifiers, gTheList);
			}
			else
			{
				getBox(theDialog, myScrollBar, &theView);
				if (PtInRect(event->where, &theView))
				{
					handleScrollBar(theDialog, event);
				}
			}
			LocalToGlobal(&(event->where));
			hiliteItem(theDialog, removeButton, selected() ? 0 : 255);
		}
	}
	return false;
}

// are any cells selected?

Boolean selected(void)
{
	Cell	theCell = {0, 0};
	
	return LGetSelect(true, &theCell, gTheList);
}

// remove active cells. Even though there should be only one, I go thru them all anyway.
void removeCells(void)
{
	Cell	theCell = {0, 0};
	
	while(LGetSelect(true, &theCell, gTheList))
	{
		LDelRow(1, theCell.v, gTheList);
		SetCRefCon(gTheScrollBar, GetCRefCon(gTheScrollBar) -1);	// subtract
		SetCtlMax(gTheScrollBar, GetCRefCon(gTheScrollBar));			// set scroll limit
	}
}

//	ugly, but effective: the list is 3 columns wide, so I just stuff
//	the other 2 colums with the parID and the vRefNum, respectively.

void addData(void)
{
	long	parID;
	short	vRefNum;
	Str32	text;
	short	len;
	Cell	theCell = {0, 0};
	
	theCell.v = (**gTheList).dataBounds.bottom+1;
	theCell.v = LAddRow(1, theCell.v, gTheList);
	
	parID = *((long *)CurDirStore);
	vRefNum = -1 * (*(short *)SFSaveDisk);

	len = gTheReply.fName[0];
	LSetCell(&(gTheReply.fName[1]), len, theCell, gTheList);	// name

	NumToString(parID, text);
	len = text[0];
	theCell.h++;
	LSetCell(&(text[1]), len, theCell, gTheList);				// parent id
	
	parID = vRefNum;
	NumToString(parID, text);
	len = text[0];
	theCell.h++;
	LSetCell(&(text[1]), len, theCell, gTheList);				// vrefnum
	SetCRefCon(gTheScrollBar, GetCRefCon(gTheScrollBar) + 1);	// add 1 file to count
	SetCtlMax(gTheScrollBar, GetCRefCon(gTheScrollBar));			// set scroll limit
}

//	just a test, to make sure all the files are there.

void processData(void)
{
	FSSpec	theFile;
	long	num;
	Str32	text;
	Cell	theCell = {0, 2};
	short	len;
	
	for (theCell.v = 0; theCell.v < (**gTheList).dataBounds.bottom; theCell.v++)
	{
		len = 63;
		LGetCell((theFile.name)+1, &len, theCell, gTheList);
		theFile.name[0] = len;
		StringToNum(theFile.name, &(theFile.parID));
		theFile.vRefNum = theFile.parID;
		
		theCell.h--;
		len = 63;
		LGetCell((theFile.name)+1, &len, theCell, gTheList);
		theFile.name[0] = len;
		StringToNum(theFile.name, &(theFile.parID));
		
		theCell.h--;
		len = 63;
		LGetCell(theFile.name+1, &len, theCell, gTheList);
		theFile.name[0] = len;
		theCell.h = 2;
	}
}

/* this routine has been left in here for purposes of testing and completeness */
//void main(void)
//{
//	Point	where = {-1, -1};
//		
//	// note: the file filter can be omitted, if you want.
//	SFPGetFile(where, "\pSelect files:", myFileFilter, -1, nil, myDlgHook, &gTheReply, kMyDialog, myFilterProc);
//
//	if (gTheReply.good)
//	{
//		processData();
//	}
//	cleanup();
//}

void handleScrollBar(DialogPtr theDialog, EventRecord *event)
{
	short			thePart, oldVal;
	Point			pt = (*event).where;
	ControlHandle	theControl;
	
	if ((thePart = FindControl(pt, theDialog, &theControl)) == inThumb)
	{
		oldVal = GetCtlValue(theControl);
		thePart = TrackControl(theControl, pt, nil);
		LScroll(0, GetCtlValue(theControl) - oldVal, gTheList);
	}
	else
	{
		thePart = TrackControl(theControl, pt, &scrollProc);
	}
}

pascal void scrollProc(ControlHandle theControl, short theCode)
{
	short	theValue = GetCtlValue(theControl);
	switch (theCode)
	{
		case inPageDown:
		case inDownButton:
			theValue++;
			LScroll(0, 1, gTheList);
			break;
		case inPageUp:
		case inUpButton:
			LScroll(0, -1, gTheList);
			theValue--;
			break;
	}
	SetCtlValue(theControl, theValue);
}


/*********************************************************************************/


/* returns a sack containing FSSpecs if good, otherwise returns NIL if cancelled */
CSack*		SFMultiGet(void)
	{
		Point			where = {-1, -1};
		CSack*		FileList;

		/* note: the file filter can be omitted, if you want. */
		SFPGetFile(where, "\pSelect files:", (void*)myFileFilter, -1, nil, myDlgHook, &gTheReply, kMyDialog, myFilterProc);

		if (gTheReply.good)
			{
				FSSpec	theFile;
				long		num;
				Str32		text;
				Cell		theCell = {0, 2};
				short		len;

				FileList = new CSack;
				FileList->ISack(sizeof(FSSpec),sizeof(FSSpec) * 16);

				for (theCell.v = 0; theCell.v < (**gTheList).dataBounds.bottom; theCell.v++)
				{
					len = 63;
					LGetCell((theFile.name)+1, &len, theCell, gTheList);
					theFile.name[0] = len;
					StringToNum(theFile.name, &(theFile.parID));
					theFile.vRefNum = theFile.parID;
					
					theCell.h--;
					len = 63;
					LGetCell((theFile.name)+1, &len, theCell, gTheList);
					theFile.name[0] = len;
					StringToNum(theFile.name, &(theFile.parID));
					
					theCell.h--;
					len = 63;
					LGetCell(theFile.name+1, &len, theCell, gTheList);
					theFile.name[0] = len;
					theCell.h = 2;

					FileList->PushElement(&theFile);
				}
			}
		 else
			{
				FileList = NIL;
			}
		cleanup();
		return FileList;
	}
