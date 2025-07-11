/*
	This shell was created to demonstrate the correct way to use AppendDITL(), CountDITL(),
	and ShortenDITL() Dialog Manager calls in System7.  This shell also show how to corectly
	use the WindowFont for drawing text in a dialog This demo creates a sample dialog which 
	contains a checkbox control using the WindowFont.  This checkbox appends and shortens the
	dialogs DITL.  
	
	This sample is provided with the following files:
	testShell.�		// CodeWarroir 1.1.1.2 project file
	testShell.c		// source for this demo
	testShell.rsrc	// resources for this demo
	AppendDITLDem	// sample compiled app
	
	!!!NOTE: The correct way to do this is to use overlayDITL if you plan on
	shrinking the dialog back and forth between the appended and normal length.  I far as
	I can tell this is not fully documanted anywhere!!!
	
	This code was provided to me by Phil Kearney, III (otherwise known as just plain "Dude")
	
	Thanks Dude :)
	
	- Marty Wachter
	mrw@welchgate.welch.jhu.edu or afaMarty@aol.com
	

*/


void 			main(void);
static void		ToolboxInit(void);
void 			DoTestDialog(void);

// � main
//
void main(void)
{
	ToolboxInit();
	DoTestDialog();
	ExitToShell();
}


// � DoTestDialog
//
void DoTestDialog(void)
{
	DialogPtr	theDialog = nil;
	short		item, wide, high, origCnt;
	Handle		ditlHndl = nil;
	FontInfo	fInfo;
	
	// get the dialog from the resource fork
	theDialog = GetNewDialog(1025, nil, (WindowPtr)-1L);
	if(!theDialog)
		return;
	
	// save the original item count for later shortening of the DITL
	origCnt = CountDITL(theDialog);
	
	// get a handle to the DITL to append
	ditlHndl = Get1Resource('DITL', 1026);
	if(!ditlHndl)
		return;
	
	// lock it down until we are through with the dialog
	HLock(ditlHndl);
	
	SetPort(theDialog);
	
	// the proper way to set the dialog's text to be drawn
	// in some other font chicago.
	TextFont(geneva);
	TextSize(9);
	GetFontInfo(&fInfo);
	
	(**(((DialogPeek)theDialog)->textH)).txFont = geneva;
	(**(((DialogPeek)theDialog)->textH)).txSize = 9;
	
	(**(((DialogPeek)theDialog)->textH)).lineHeight = fInfo.ascent + fInfo.descent + fInfo.leading;
	(**(((DialogPeek)theDialog)->textH)).fontAscent = fInfo.ascent;
	
	// save the original width and height for later shortening
	wide = theDialog->portRect.right - theDialog->portRect.left;
	high = theDialog->portRect.bottom - theDialog->portRect.top;
	
	// show our dialog
	ShowWindow(theDialog);
	
	do {
	
		ModalDialog(nil, &item);
		
		if(item == 3) {		// our checkbox
		
			short	iType, numAppended;
			Rect	iRect;
			Handle	iHandle;
			Boolean	checked;
			
			// see if the checkbox is checked or not
			GetDItem(theDialog, item, &iType, &iHandle, &iRect);
			checked = GetCtlValue((ControlHandle)iHandle);
			
			if(!checked) {
				// append the DITL using overlayDITL mode
				AppendDITL(theDialog, ditlHndl, overlayDITL);
				
			} else {
				// shorten the DITL to it's original size
				
				// get the differnece of items in the dialog - the original count
				numAppended = CountDITL(theDialog) - origCnt;
				
				// shorten the DITL
				ShortenDITL(theDialog, numAppended);
				
				// resize the dialog to it's original dimensions
				SizeWindow(theDialog, wide, high, true);
			}
			
			// reset the control's value
			SetCtlValue((ControlHandle)iHandle, !checked);
		}
		
	} while(item > ok);
	
	// unlock and release
	HUnlock(ditlHndl);
	ReleaseResource(ditlHndl);
	
	// get rid of our dialog
	DisposeDialog(theDialog);
}


// � ToolboxInit
// Standard set of calls to enable all the toolbox calls
//
static void ToolboxInit(void)
{
    InitGraf((Ptr)&qd.thePort);
    InitFonts();
    InitWindows();
    InitMenus();
    TEInit();
    InitDialogs(0L);
    InitCursor();
    MaxApplZone();
    MoreMasters();
    MoreMasters();
    MoreMasters();
 }