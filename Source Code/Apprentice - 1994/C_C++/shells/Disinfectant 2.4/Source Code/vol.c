/*______________________________________________________________________

	vol.c - Volume Selection Module.
	
	Copyright © 1988, 1989, 1990 Northwestern University.  Permission is granted
	to use this code in your own projects, provided you give credit to both
	John Norstad and Northwestern University in your about box or document.
	
	This reusable module implements volume selection as in Apple's 
	standard file package.
	
	The caller supplies a Drive button, an Eject button, and a rectangle
	in which the name of the currently selected volume should be displayed.
	
	The module takes care of cycling through selected on-line volumes,
	processing hits in the drive and eject buttons, properly hiliting
	these buttons, processing disk insertion events, and drawing the 
	volume name in response to update events.
	
	In addition to the volume name displayed in a rectangle, the caller 
	can also request that small floppy and hard drive icons be displayed.  
	The module displays the appropriate small icon in a caller-specified 
	rectangle.  The caller supplies the small icons.
	
	If the current system supports popup menus, the volume name rectangle is
	framed with a drop shadow.  If the user clicks in this rectangle a popup
	menu is presented listing all mounted volumes.
	
	All of the routines do error checking, and return an error code
	as their function result.
	
	This module should only be used in a modal environment (e.g., modal
	dialogs).  The reason is that it assumes only it is mounting and
	unmounting volumes.  In a non-modal environment this assumption
	is invalid.
	
	All of the code placed in its own segment named "vol", except for the
	vol_Init routine, which is placed in segment voli.
_____________________________________________________________________*/


#pragma load "precompile"
#include "vol.h"
#include "utl.h"

#pragma segment vol

/*______________________________________________________________________

	Global Variables.
_____________________________________________________________________*/


static ControlHandle	DriveH;		/* handle to drive push button */
static ControlHandle	EjectH;		/* handle to eject push button */
static Rect				*NameRect;	/* pointer to volume name rectangle */
static Rect				AdjNameRect;	/* rectangle enclosing current vol name,
												adjusted for width of name */
static Rect				*IconRect;	/* pointer to small vol icon rectangle */
static Handle			FloppyH;		/*	handle to small floppy icon */
static Handle			HardH;		/* handle to small hard drive icon */
static Boolean			UnmountFlag;	/* true to unmount ejected volumes */
static short			CurVol;		/* index of currently selected volume,
												or 0 if none */
static short			ApplVol;		/* vol ref num of vol contng current appl */
static short			SysVol;		/* vol ref num of system vol */
static Boolean			HavePopUp; 	/* true if system supports popup menus */

/*______________________________________________________________________

	NextVol - Advance to the Next Volume.
	
	Exit:		function result = error code.
_____________________________________________________________________*/


static OSErr NextVol(void)

{
	HParamBlockRec	pBlock;				/* vol info param block */
	OSErr				rCode;				/* result code */
	short				volInx;				/* volume index */
	
	pBlock.volumeParam.ioNamePtr = nil;
	volInx = CurVol;
	while (true) {
		pBlock.volumeParam.ioVolIndex = ++volInx;
		pBlock.volumeParam.ioVRefNum = 0;
		if (rCode = PBHGetVInfo(&pBlock, false)) {
			volInx = 0;
		} else if (pBlock.volumeParam.ioVDrvInfo) {
			CurVol = volInx;
			return noErr;
		};
		if (volInx == CurVol) {
			CurVol = 0;
			return noErr;
		};
	};
}

/*______________________________________________________________________

	HiliteDrive() - Hilite the Drive Button.
	
	The drive button is hilited iff there are at least two on-line
	volumes.
_____________________________________________________________________*/


static void HiliteDrive(void)

{
	HParamBlockRec	pBlock;				/* vol info param block */
	short				volInx;				/* vol index */
	short				numOnLine;			/* number of online vols */
	OSErr				rCode;				/* result code */
	
	pBlock.volumeParam.ioNamePtr = nil;
	volInx = 0;
	numOnLine = 0;
	while (true) {
		pBlock.volumeParam.ioVolIndex = ++volInx;
		pBlock.volumeParam.ioVRefNum = 0;
		if (rCode = PBHGetVInfo(&pBlock, false)) break;
		if (pBlock.volumeParam.ioVDrvInfo) numOnLine++;
		if (numOnLine > 1 ) {
			HiliteControl(DriveH, 0);
			return;
		};
	};
	HiliteControl(DriveH, 255);
}

/*______________________________________________________________________

	HiliteEject() - Hilite the eject Button.
	
	Exit:		function result = error code.
	
	The eject button is hilited iff the current volume is ejectable.
_____________________________________________________________________*/


static OSErr HiliteEject(void)

{
	short				vRefNum;				/* vol ref num of cur vol */
	OSErr				rCode;				/* result code */
	
	/* If there is no current volume unhilite the eject button. */
	
	if (!CurVol) {
		HiliteControl(EjectH, 255);
		return noErr;
	};
	
	/* Hilite the button iff the cur vol is ejectable. */
	
	if (rCode = vol_GetSel(&vRefNum)) return rCode;
	HiliteControl(EjectH, utl_Ejectable(vRefNum) ? 0 : 255);
	return noErr;
}

/*______________________________________________________________________

	Inval - Invalidate the volume name and icon rectangles.
_____________________________________________________________________*/


static void Inval(void)

{
	Rect		boxRect;

	boxRect = *NameRect;
	InsetRect(&boxRect, -2, -2);
	InvalRect(&boxRect);
	if (IconRect) InvalRect(IconRect);
}

/*______________________________________________________________________

	vol_Init - Initialize.
	
	Entry:	driveH = handle to drive push button control.
				ejectH = handle to eject push button control.
				nameRect = pointer to volume name rectangle.
				iconRect = pointer to small volume icon rectangle, 
					or nil if none.
				floppyH = handle to small floppy icon, or nil if none.
				hardH = handle to small hard drive icon, or nil if none.
				applSel = true if the initial volume should be the volume
					containing the current application.  False if the initial
					volume should be some other volume (if there is one).
				unmmountFlag = true if ejected floppies should also be 
					unmounted.  The boot volume and the volume containing the 
					current application are never unmounted.
					
	Exit:		function result = error code.
					
	vol_Init must be called before calling any of the other routines
	in the module.  It may be called more than once to reinitialize
	the module.
					
	vol_Init saves the parameters in private globals, selects an 
	appropriate initial volume, and hilites the drive and eject buttons 
	appropriately.
	
	Don't forget to do a SetPort to the window before calling this routine.
_____________________________________________________________________*/


#pragma segment voli

OSErr vol_Init (ControlHandle driveH, ControlHandle ejectH, 
	Rect *nameRect, Rect *icnRect, Handle floppyH, 
	Handle hardH, Boolean applSel, Boolean unmountFlag)
	
{

	HParamBlockRec	pBlock;				/* Vol info param block */
	short				applInx;				/* index of application volume */
	OSErr				rCode;				/* result code */
	
	/* Save parameter values in private global variables. */
	
	DriveH = driveH;
	EjectH = ejectH;
	NameRect = nameRect;
	IconRect = icnRect;
	FloppyH = floppyH;
	HardH = hardH;
	UnmountFlag = unmountFlag;
	
	/* Get applInx = index of volume containing current application,
		and ApplVol = vol ref num of applicaton volume. */
	
	pBlock.volumeParam.ioNamePtr = nil;
	ApplVol = utl_GetApplVol();
	applInx = 0;
	do {
		pBlock.volumeParam.ioVolIndex = ++applInx;
		pBlock.volumeParam.ioVRefNum = 0;
		rCode = PBHGetVInfo(&pBlock, false);
	} while (!rCode && pBlock.volumeParam.ioVRefNum != ApplVol);
	if (rCode) return rCode;
	
	/* Select initial volume.  */
	
	if (applSel) {
		CurVol = applInx;
	} else {
		CurVol = 0;
		do {
			pBlock.volumeParam.ioVolIndex = ++CurVol;
			pBlock.volumeParam.ioVRefNum = 0;
			rCode = PBHGetVInfo(&pBlock, false);
		} while (!rCode && (!pBlock.volumeParam.ioVDrvInfo || 
			pBlock.volumeParam.ioVRefNum == ApplVol));
		if (rCode) CurVol = applInx;
	};
	
	/* Get SysVol = vol ref num of system volume. */
	
	SysVol = utl_GetSysVol();
	
	/* Find out whether the system supports popup menus. */
	
	HavePopUp = utl_SysHasPopUp();
		
	/* Hilite buttons and invalidate rectangles. */
	
	HiliteDrive();
	if (rCode = HiliteEject()) return rCode;
	Inval();
	return noErr;
}

#pragma segment vol

/*______________________________________________________________________

	vol_DoDrive - Process Drive Button Hit.
					
	Exit:		function result = error code.
	
	This routine must be called whenever there's a hit in the Drive
	button.  It advances to the next on-line volume, hilites the 
	eject button appropriately, and invalidates the volume name and 
	icon rectangles.
	
	To perfectly mimic the standard file package, you can also call this
	routine whenever the tab key is pressed.
	
	Don't forget to do a SetPort to the window before calling this routine.
_____________________________________________________________________*/


OSErr vol_DoDrive (void)

{
	OSErr		rCode;			/* result code */

	if (rCode = NextVol()) return rCode;
	if (rCode = HiliteEject()) return rCode;
	Inval();
	return noErr;
}

/*______________________________________________________________________

	vol_DoPopUp - Present Volume Popup Menu.
	
	Entry:	where = location of mouse down, local coords.
				menuID = menu id to use for popup menu.
					
	Exit:		function result = error code.
	
	This routine should be called whenever there's a mouse hit in the volume 
	name rectangle.  It builds and presents a pop up menu listing all the 
	mounted volumes.  The current volume is set to the volume selected from 
	the menu by the user.
	
	The routine returns immediately if the mouse down event is not inside
	the volume name rectangle.  It also returns immediately if the ROM or
	system does not support popup menus.
	
	Don't forget to do a SetPort to the window before calling this routine.
_____________________________________________________________________*/


OSErr vol_DoPopUp (Point where, short menuID)

{
	MenuHandle		theMenu;				/* handle to popup menu */
	long				theItem;				/* 16/menu number, 16/item number */
	short				volInx;				/* index in volume list */
	HParamBlockRec	pBlock;				/* vol info param block */
	Str255			volName;				/* vol name */
	OSErr				rCode;				/* result code */
	Point				whereMenu;			/* loc of popup menu, global coords */
	short				item;					/* menu item number */
	short				checkedItem;		/* menu item number of checked item */
	char				*p;					/* pointer into vol name string */
	Rect				invertRect;			/* rectangle to be inverted */
	Rect				popTestRect;		/* rectangle to hit test */
	
	/* Return if pop up menus are not available on this sytem. */
	
	if (!HavePopUp) return noErr;
	
	/* Return if mousedown not in volume name rectangle. */
	
	popTestRect = *NameRect;
	popTestRect.right = AdjNameRect.right - 1;
	if (!PtInRect(where, &popTestRect)) return noErr;
	
	/* Return if there is no current volume. */
	
	if (!CurVol) return noErr;
	
	/* Create the menu. */
	
	theMenu = NewMenu(menuID, "\p");
	
	/* Fill the menu with the names of all currently mounted volumes.  Put
		a check mark next to the currently selected volume. */
	
	volInx = item = 0;
	pBlock.volumeParam.ioNamePtr = volName;
	while (true) {
		pBlock.volumeParam.ioVolIndex = ++volInx;
		pBlock.volumeParam.ioVRefNum = 0;
		rCode = PBHGetVInfo(&pBlock, false);
		if (rCode) break;
		if (pBlock.volumeParam.ioVDrvInfo) {
			AppendMenu(theMenu, "\p ");
			if (*(volName+1) == '-') {
				/* vol name starts with '-': prepend a blank to the beginning
					of the name to avoid having the menu manager display it as
					a separator line. */
				for (p = volName + *volName; p > volName; p--) *(p+1) = *p;
				*(volName+1) = ' ';
				(*volName)++;
			};
			SetItem(theMenu, ++item, volName);
			if (volInx == CurVol) 
				CheckItem(theMenu, checkedItem = item, true);
		};
	};
		
	/* Insert menu into menu bar. */
	
	InsertMenu(theMenu, -1);
	
	/* Pop it up.  Extract the item number selected.  Make the selected item
		the current volume. */
	
	SetPt(&whereMenu, NameRect->left, NameRect->top);
	LocalToGlobal(&whereMenu);
	if (IconRect) {
		invertRect = *IconRect;
		invertRect.left--;
		invertRect.right = NameRect->left;
		InvertRect(&invertRect);
	};
	theItem = PopUpMenuSelect(theMenu, whereMenu.v, whereMenu.h, 
		checkedItem);
	if (IconRect) InvertRect(&invertRect);
	if ((theItem >> 16) & 0xffff) {
		item = theItem & 0xffff;
		volInx = 0;
		pBlock.volumeParam.ioNamePtr = nil;
		while (true) {
			pBlock.volumeParam.ioVolIndex = ++volInx;
			pBlock.volumeParam.ioVRefNum = 0;
			rCode = PBHGetVInfo(&pBlock, false);
			if (rCode) break;
			if (pBlock.volumeParam.ioVDrvInfo) {
				if (!(--item)) {
					CurVol = volInx;
					break;
				};
			};
		};
		if (rCode = HiliteEject()) return rCode;
		Inval();
	};
	
	/* Delete and dispose of the menu and return. */
	
	DeleteMenu(menuID);
	DisposeMenu(theMenu);
	return noErr;
}

/*______________________________________________________________________

	vol_DoEject - Process eject Button Hit.
					
	Exit:		function result = error code.
	
	This routine must be called whenever there's a hit in the eject
	button.  It ejects the currently selected volume, and unmounts it
	if the unmount option was selected and the volume is not the
	startup volume or the one containing the current application.  It 
	then advances to the next on-line volume, hilites the Drive and 
	eject buttons appropriately, and invalidates the volume name and 
	icon rectangles.
	
	Don't forget to do a SetPort to the window before calling this routine.
_____________________________________________________________________*/


OSErr vol_DoEject (void)

{
	HParamBlockRec	pBlock;				/* vol info param block */
	OSErr				rCode;				/* result code */
	short				volRefNum;			/* vol ref num of current vol */
	
	/* eject the current volume. */
	
	pBlock.volumeParam.ioNamePtr = nil;
	pBlock.volumeParam.ioVolIndex = CurVol;
	pBlock.volumeParam.ioVRefNum = 0;
	if (rCode = PBHGetVInfo(&pBlock, false)) return rCode;
	volRefNum = pBlock.volumeParam.ioVRefNum;
	if (rCode = Eject(nil, volRefNum)) return rCode;
	
	/* If UnmountFlag is true, and the volume is not the system or
		application volume, then unmount it. */
		
	if (UnmountFlag && volRefNum != SysVol && volRefNum != ApplVol) {
		if (rCode = UnmountVol(nil, volRefNum)) return rCode;
		CurVol = 0;
	};
	
	/* Advance to next volume, hilite the buttons, and invalidate the 
	rectangles. */
		
	if(rCode = NextVol()) return rCode;
	HiliteDrive();
	if (rCode = HiliteEject()) return rCode;
	Inval();
	return noErr;
}

/*______________________________________________________________________

	vol_DoInsert - Process Disk Insertion Evect.
	
	Entry:	message = message field from event record.
					
	Exit:		function result = error code.
	
	This routine must be called whenever there's a disk insertion event.
	It makes the inserted volume the current volume, hilites the
	Drive and eject buttons appropriately, and invalidates the volume
	name and icon rectangles.
	
	Don't forget to do a SetPort to the window before calling this routine.
_____________________________________________________________________*/


OSErr vol_DoInsert (long message)

{
	OSErr				rCode;				/* result code */
	short				vRefNum;				/* vol ref num of inserted disk */

	/* Process the disk insertion event, and get the vol ref num */
	
	if (rCode = utl_DoDiskInsert(message, &vRefNum)) return rCode;
	
	/* Make the new disk the current volume. */
	
	if (rCode = vol_SetSel(vRefNum, true)) return rCode;
}

/*______________________________________________________________________

	vol_DoUpdate - Process Update Event.
					
	Exit:		function result = error code.
	
	This routine must be called whenever an update event occurs.  It
	draws the volume name using the font attributes.  It also draws 
	the proper small icon.
	
	If the current system supports popup menus, the volume name is also 
	framed, with a drop shadow.
	
	Don't forget to do a SetPort to the window, call BeginUpdate, and
	set your desired font and size before calling this routine.
_____________________________________________________________________*/


OSErr vol_DoUpdate (void)

{
	HParamBlockRec	pBlock;				/* vol info param block */
	Str255			volName;				/* vol name */
	OSErr				rCode;				/* result code */
	Rect				textRect;			/* rectangle for TextBox */
	short				nameWidth;			/* width of vol name */
	short				vRefNum;				/* vol ref num of cur vol */
	short				slop;					/* extra pixels needed in name width */
	PolyHandle		triangle;			/* handle to triangle polygon */
	
	/* Return if there is no current volume. */
	
	if (!CurVol) return noErr;
	
	/* Get the volume name. */
	
	pBlock.volumeParam.ioNamePtr = volName;
	pBlock.volumeParam.ioVolIndex = CurVol;
	pBlock.volumeParam.ioVRefNum = 0;
	if (rCode = PBHGetVInfo(&pBlock, false)) return rCode;
	
	/* If the name is too long truncate it and append the ellipsis char "É" */
	
	slop = HavePopUp ? 20 : 9;
	while (true) {
		nameWidth = StringWidth(volName);
		if (NameRect->left + nameWidth + slop <= NameRect->right) break;
		(*volName)--;
		*(volName + *volName) = 'É';
	};

	/* Draw the volume name and icon. */
	
	textRect = *NameRect;
	textRect.left += 4;
	TextBox(volName+1, *volName, &textRect, teJustLeft);
	if (IconRect) {
		if (rCode = vol_GetSel(&vRefNum)) return rCode;
		utl_PlotSmallIcon (IconRect, utl_Ejectable(vRefNum) ? FloppyH : HardH);
	};
		
	/* Frame the volume name, with a drop shadow, and draw the triangle. */
		
	if (HavePopUp) {
		AdjNameRect = *NameRect;
		AdjNameRect.right = AdjNameRect.left + nameWidth + slop;
		InsetRect(&AdjNameRect, -1, -1);
		FrameRect(&AdjNameRect);
		MoveTo(AdjNameRect.right, AdjNameRect.top+2);
		LineTo(AdjNameRect.right, AdjNameRect.bottom);
		LineTo(AdjNameRect.left+2, AdjNameRect.bottom);
		triangle = OpenPoly();
		MoveTo(AdjNameRect.right - 14, 
			(AdjNameRect.top + AdjNameRect.bottom - 5)>>1);
		Line(10, 0);
		Line(-5, 5);
		Line(-5, -5);
		ClosePoly();
		PaintPoly(triangle);
		KillPoly(triangle);
	};
	return noErr;
}

/*______________________________________________________________________

	vol_Verify - Verify and Adjust Selected Volume.
	
	Exit:		function result = error code.
	
	This routine should be called after any call to the standard file
	package or any other activity which might have caused the volume
	list to change state. 
	
	vol_Verify checks to make sure that the current volume still exists and 
	is online.  If it isn't, it finds some other volume that is and makes it 
	the current one.  

	The volume name and icon rectangles are invalidated, and the Drive and 
	eject buttons are rehilited appropriately.
	_____________________________________________________________________*/


OSErr vol_Verify (void)

{
	HParamBlockRec	pBlock;			/* vol info param block */
	OSErr				rCode;			/* reslut code */
	
	if (!CurVol) {
		rCode = NextVol();
	} else {
		pBlock.volumeParam.ioNamePtr = nil;
		pBlock.volumeParam.ioVolIndex = CurVol;
		pBlock.volumeParam.ioVRefNum = 0;
		rCode = PBHGetVInfo(&pBlock, false);
		if ((rCode == nsvErr) || !pBlock.volumeParam.ioVDrvInfo) {
			CurVol = 0;
			rCode = NextVol();
		};
	};
	if (rCode) return rCode;
	HiliteDrive();
	if (rCode = HiliteEject()) return rCode;
	Inval();
}

/*______________________________________________________________________

	vol_GetSel - Get Volume Reference Number of Selected Volume.
	
	Exit:		*vRefNum = volume reference number.
				function result = error code (= nsvErr if no current vol).
_____________________________________________________________________*/


OSErr vol_GetSel (short *vRefNum)

{
	HParamBlockRec	pBlock;				/* vol info param block */
	OSErr				rCode;				/* result code */
	
	if (!CurVol) return nsvErr;
	pBlock.volumeParam.ioNamePtr = nil;
	pBlock.volumeParam.ioVolIndex = CurVol;
	pBlock.volumeParam.ioVRefNum = 0;
	if (rCode = PBHGetVInfo(&pBlock, false)) return rCode;
	*vRefNum = pBlock.volumeParam.ioVRefNum;
	return noErr;
}

/*______________________________________________________________________

	vol_SetSel - Set Volume Reference Number of Selected Volume.
	
	Entry:	vRefNum = volume reference number.
				doButtons = true to hilite Drive and eject buttons.
	
	Exit:		function result = error code.
_____________________________________________________________________*/


OSErr vol_SetSel (short vRefNum, Boolean doButtons)

{
	HParamBlockRec	pBlock;				/* vol info param block */
	OSErr				rCode;				/* result code */
	
	CurVol = 0;
	pBlock.volumeParam.ioNamePtr = nil;
	do {
		pBlock.volumeParam.ioVolIndex = ++CurVol;
		pBlock.volumeParam.ioVRefNum = 0;
		rCode = PBHGetVInfo(&pBlock, false);
	} while (!rCode && pBlock.volumeParam.ioVRefNum != vRefNum);
	if (rCode) return rCode;
	if (doButtons) {
		HiliteDrive();
		if (rCode = HiliteEject()) return rCode;
	};
	Inval();
	return noErr;
}

/*______________________________________________________________________

	vol_GetName - Get Name of Selected Volume.
	
	Exit:		vName = volume name.
				function result = error code (= nsvErr if no current vol).
_____________________________________________________________________*/


OSErr vol_GetName (Str255 vName)

{
	HParamBlockRec	pBlock;				/* vol info param block */
	OSErr				rCode;				/* result code */
	
	if (!CurVol) return nsvErr;
	
	/* Get the volume name. */
	
	pBlock.volumeParam.ioNamePtr = vName;
	pBlock.volumeParam.ioVolIndex = CurVol;
	pBlock.volumeParam.ioVRefNum = 0;
	if (rCode = PBHGetVInfo(&pBlock, false)) return rCode;
	return noErr;
}