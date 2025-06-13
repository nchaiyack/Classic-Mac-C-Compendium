/**********************************************************
 *	Basic Black © 1993,1994 by Mason L. Bliss
 *	All Rights Reserved
 *	version 1.4.1
 *
 *	This cdev code uses the sample cdev class included with
 *	the Think C 5.0.4 compiler. To build the project, you
 *	will have to supply the Symantec code or an equivalent.
 *
 *	The important bits involve the use of the Gestalt manager
 *	for cdev/INIT communication, to which the cdev mechanism
 *	itself is irrelevant.
 **********************************************************/

#include <cdev.h>
#include <GestaltEqu.h>
#include "BBcdev.h"
#include "BB.h"



/**********************************************************
 *	BBcdev::Init
 *
 *	This method initializes stuff.
 **********************************************************/
void BBcdev::Init(void)
{
	PrefStructureHandle	prefHandle;
	ControlHandle		theControl;
	short				controlValue;
	long				theLong, width, height;
	Rect				box;
	Str255				theString;

	/* Initialize stuff */
	inherited::Init();
	
	/* Get our preferences data */
	prefHandle = (PrefStructureHandle) GetIndResource('PREF', 1);
	if (ResError()) {
		Error(cdevResErr);
		return;
	}
	
	/* Load & Lock Picture Resources */
	pictHandles[0] = GetResource('PICT', topLeftPict);
	pictHandles[1] = GetResource('PICT', topRightPict);
	pictHandles[2] = GetResource('PICT', botRightPict);
	pictHandles[3] = GetResource('PICT', botLeftPict);
	pictHandles[4] = GetResource('PICT', noArrowPict);
	pictHandles[5] = GetResource('PICT', upArrowPict);
	pictHandles[6] = GetResource('PICT', dnArrowPict);
	pictHandles[7] = GetResource('PICT', grArrowPict);
	HLock(pictHandles[0]);
	HLock(pictHandles[1]);
	HLock(pictHandles[2]);
	HLock(pictHandles[3]);
	HLock(pictHandles[4]);
	HLock(pictHandles[5]);
	HLock(pictHandles[6]);
	HLock(pictHandles[7]);

	/* get the sleep corner info */
	sleepNowRect = (**prefHandle).sleepNowCorner;
	GetDItem(dp, lastItem - initDev + sleepNow,
				&controlValue, &theControl, &box);
	SetDItem(dp, lastItem - initDev + sleepNow,
				controlValue, pictHandles[sleepNowRect], &box);
	InsetRect(&box, 3, 3);
	EraseRect(&box);
	InsetRect(&box, -3, -3);
	DrawPicture(pictHandles[sleepNowRect], &box);
	
	width  = ((box.right - box.left) / 2);
	height = ((box.bottom - box.top) / 2);
	
	/* Set up some small rects */
	smallSNRect[0] = box;
	smallSNRect[0].right -= width;
	smallSNRect[0].bottom -= height;
	smallSNRect[1] = box;
	smallSNRect[1].left += width;
	smallSNRect[1].bottom -= height;
	smallSNRect[2] = box;
	smallSNRect[2].left += width;
	smallSNRect[2].top += height;
	smallSNRect[3] = box;
	smallSNRect[3].right -= width;
	smallSNRect[3].top += height;
	
	/* get the wake corner info */
	neverSleepRect = (**prefHandle).sleepNeverCorner;
	GetDItem(dp, lastItem - initDev + neverSleep,
				&controlValue, &theControl, &box);
	SetDItem(dp, lastItem - initDev + neverSleep,
				controlValue, pictHandles[neverSleepRect], &box);
	InsetRect(&box, 3, 3);
	EraseRect(&box);
	InsetRect(&box, -3, -3);
	DrawPicture(pictHandles[neverSleepRect], &box);

	width  = ((box.right - box.left) / 2);
	height = ((box.bottom - box.top) / 2);

	/* Set up more small rects */
	smallNSRect[0] = box;
	smallNSRect[0].right -= width;
	smallNSRect[0].bottom -= height;
	smallNSRect[1] = box;
	smallNSRect[1].left += width;
	smallNSRect[1].bottom -= height;
	smallNSRect[2] = box;
	smallNSRect[2].left += width;
	smallNSRect[2].top += height;
	smallNSRect[3] = box;
	smallNSRect[3].right -= width;
	smallNSRect[3].top += height;
			
	/* get the clock checkbox */
	GetDItem(dp, lastItem - initDev + clockOn, &controlValue, &theControl, &box);
	if ((**prefHandle).bounceClock)
		SetCtlValue(theControl, 1);
	
	/* get the "screen saver on" checkbox */
	GetDItem(dp, lastItem - initDev + screenOn, &controlValue, &theControl, &box);
	if ((**prefHandle).saverOn)
		SetCtlValue(theControl, 1);
	
	/* get the "Show Startup Icon" checkbox */
	GetDItem(dp, lastItem - initDev + startupIcon, &controlValue, &theControl, &box);
	if ((**prefHandle).showIcon)
		SetCtlValue(theControl, 1);
	
	/* get the "Fade To White" checkbox */
	GetDItem(dp, lastItem - initDev + fadeWhite, &controlValue, &theControl, &box);
	if ((**prefHandle).fadeToWhite)
		SetCtlValue(theControl, 1);

	/* get the "Zero Menubar" checkbox */
	GetDItem(dp, lastItem - initDev + zeroMenu, &controlValue, &theControl, &box);
	if ((**prefHandle).zeroBar)
		SetCtlValue(theControl, 1);

	/* get the "EraseRect" checkbox */
	GetDItem(dp, lastItem - initDev + eraseRect, &controlValue, &theControl, &box);
	if ((**prefHandle).eraseRectPatch)
		SetCtlValue(theControl, 1);
	
	/* get the "EraseOval" checkbox */
	GetDItem(dp, lastItem - initDev + eraseOval, &controlValue, &theControl, &box);
	if ((**prefHandle).eraseOvalPatch)
		SetCtlValue(theControl, 1);
	
	/* get the "EraseRgn" checkbox */
	GetDItem(dp, lastItem - initDev + eraseRgn, &controlValue, &theControl, &box);
	if ((**prefHandle).eraseRgnPatch)
		SetCtlValue(theControl, 1);
	
	/* get the "DrawMenuBar" checkbox */
	GetDItem(dp, lastItem - initDev + drawMenu, &controlValue, &theControl, &box);
	if ((**prefHandle).drawMenuBarPatch)
		SetCtlValue(theControl, 1);
	
	/* get the "InitCursor" checkbox */
	GetDItem(dp, lastItem - initDev + initCurs, &controlValue, &theControl, &box);
	if ((**prefHandle).initCursorPatch)
		SetCtlValue(theControl, 1);
	
	/* get the idle time edit text item */
	GetDItem(dp, lastItem - initDev + idleMins, &controlValue, &theControl, &box);
	theLong = (long) (**prefHandle).idleTime;
	NumToString(theLong, theString);
	SetIText(theControl, theString);
	SelIText(dp, lastItem - initDev + idleMins, 0, 32767);

	GetDItem(dp, lastItem - initDev + idleRect, &controlValue, &theControl, &box);
	height = ((box.bottom - box.top) / 2);
	smIdleRect = box;
	smIdleRect.bottom -= height;
	
	/* get the refresh time edit text item */
	GetDItem(dp, lastItem - initDev + refreshSecs, &controlValue, &theControl, &box);
	theLong = (long) (**prefHandle).refreshSeconds;
	NumToString(theLong, theString);
	SetIText(theControl, theString);

	GetDItem(dp, lastItem - initDev + refrRect, &controlValue, &theControl, &box);
	height = ((box.bottom - box.top) / 2);
	smRefrRect = box;
	smRefrRect.bottom -= height;

	/* see if the idle time and refresh secs arrows need to be grey */
	if (Gestalt('sysv', &theLong) != noErr || theLong < 0x700) {
		GetDItem(dp, lastItem - initDev + idleRect,
						&controlValue, &theControl, &box);
		SetDItem(dp, lastItem - initDev + idleRect,
				controlValue, pictHandles[grArrow], &box);
		GetDItem(dp, lastItem - initDev + refrRect,
						&controlValue, &theControl, &box);
		SetDItem(dp, lastItem - initDev + refrRect,
				controlValue, pictHandles[grArrow], &box);
	}

	ReleaseResource(prefHandle);
}



/**********************************************************
 *	BBcdev::Idle
 *
 *	Fix the EditText items if needed.
 **********************************************************/
void BBcdev::Idle(void)
{
	ControlHandle	theControl;
	short			controlValue;
	long			theLong;
	Rect			box;
	Str255			theString;
	
	if (mustRemember) {
		mustRemember = false;
		/* fix the idle time information if needed */
		GetDItem(dp, lastItem - initDev + idleMins, &controlValue, &theControl, &box);
		GetIText(theControl, theString);
		StringToNum(theString, &theLong);
		if (theLong < 1) {
			SetIText(theControl, "\p1");
			SelIText(dp, lastItem - initDev + idleMins, 0, 32767);
		} else if (theLong > 60) {
			SetIText(theControl, "\p60");
			SelIText(dp, lastItem - initDev + idleMins, 0, 32767);
		}
	
		/* fix the refresh time information if needed */
		GetDItem(dp, lastItem - initDev + refreshSecs, &controlValue, &theControl, &box);
		GetIText(theControl, theString);
		StringToNum(theString, &theLong);
		if (theLong < 1) {
			SetIText(theControl, "\p1");
			SelIText(dp, lastItem - initDev + refreshSecs, 0, 32767);
		} else if (theLong > 20) {
			SetIText(theControl, "\p20");
			SelIText(dp, lastItem - initDev + refreshSecs, 0, 32767);
		}
		RememberValues();
	}
}



/**********************************************************
 *	BBcdev::Close
 *
 *	This method releases the 'PICT' resources.
 **********************************************************/
void BBcdev::Close(void)
{
	short	i;
	
	for (i = 0; i < 7; ++i) {
		HUnlock(pictHandles[i]);
		ReleaseResource(pictHandles[i]);
	}
	
	inherited::Close();
}



/**********************************************************
 *	BBcdev::Key
 *
 *	This filters out invalid key presses and stuff.
 **********************************************************/
void BBcdev::Key(short theKey)
{
	if (((theKey < '0') || (theKey > '9')) && (theKey != 8)
						&& (theKey != 9) && ((theKey < 28) || (theKey > 31))) {
		event->what = nullEvent;
		return;
	}
	
	/* if we got this far, then we have a reasonably valid keystroke */
	mustRemember = true;
}



/**********************************************************
 *	BBcdev::ItemHit
 *
 *	This handles dialog item hits.
 **********************************************************/
void BBcdev::ItemHit(short theItem)
{
	ControlHandle	theControl;
	short			controlValue;
	Rect			box, save;
	long			theLong, theTime = 0;
	Str255			theString;

	GetDItem(dp, lastItem - initDev + theItem, &controlValue, &theControl, &box);
	switch (theItem) {

		case sleepNow:
			GlobalToLocal(&event->where);
			if (PtInRect(event->where, &smallSNRect[topLeftRect])
											&& neverSleepRect != topLeftRect)
				sleepNowRect = topLeftRect;
			else if (PtInRect(event->where, &smallSNRect[topRightRect])
											&& neverSleepRect != topRightRect)
				sleepNowRect = topRightRect;
			else if (PtInRect(event->where, &smallSNRect[botRightRect])
											&& neverSleepRect != botRightRect)
				sleepNowRect = botRightRect;
			else if (PtInRect(event->where, &smallSNRect[botLeftRect])
											&& neverSleepRect != botLeftRect)
				sleepNowRect = botLeftRect;
			else {
				SysBeep(5);
				break;
			}
			
			SetDItem(dp, lastItem - initDev + sleepNow,
					controlValue, pictHandles[sleepNowRect], &box);
			InsetRect(&box, 3, 3);
			EraseRect(&box);
			InsetRect(&box, -3, -3);
			DrawPicture(pictHandles[sleepNowRect], &box);
			RememberValues();
			break;
			
		case neverSleep:
			GlobalToLocal(&event->where);
			if (PtInRect(event->where, &smallNSRect[topLeftRect])
											&& sleepNowRect != topLeftRect)
				neverSleepRect = topLeftRect;
			else if (PtInRect(event->where, &smallNSRect[topRightRect])
											&& sleepNowRect != topRightRect)
				neverSleepRect = topRightRect;
			else if (PtInRect(event->where, &smallNSRect[botRightRect])
											&& sleepNowRect != botRightRect)
				neverSleepRect = botRightRect;
			else if (PtInRect(event->where, &smallNSRect[botLeftRect])
											&& sleepNowRect != botLeftRect)
				neverSleepRect = botLeftRect;
			else {
				SysBeep(5);
				break;
			}
			
			SetDItem(dp, lastItem - initDev + neverSleep,
					controlValue, pictHandles[neverSleepRect], &box);
			InsetRect(&box, 3, 3);
			EraseRect(&box);
			InsetRect(&box, -3, -3);
			DrawPicture(pictHandles[neverSleepRect], &box);
			RememberValues();
			break;

		case idleRect:
			if (Gestalt('sysv', &theLong) != noErr && theLong < 0x700)
				break;
			save = box;
			GlobalToLocal(&event->where);
			GetDItem(dp, lastItem - initDev + idleMins,
							&controlValue, &theControl, &box);
			GetIText(theControl, theString);
			StringToNum(theString, &theLong);
			theTime = Ticks - 16;
			if (PtInRect(event->where, &smIdleRect)) {			// higher!!!
				DrawPicture(pictHandles[upArrow], &save);
				SelIText(dp, lastItem - initDev + idleMins, 0, 32767);
				do {
					SystemTask();
					if (Ticks - theTime > 15) {
						theTime = Ticks;
						if (theLong < 10) {
							NumToString(theLong += 1, theString);
							SetIText(theControl, theString);
							if (theLong == 10)
								SelIText(dp, lastItem - initDev + idleMins, 0, 32767);
						} else if (theLong < 60) {
							NumToString(theLong += 5, theString);
							SetIText(theControl, theString);
						}
					}
				} while (Button());
			} else {											// lower.
				DrawPicture(pictHandles[dnArrow], &save);
				SelIText(dp, lastItem - initDev + idleMins, 0, 32767);
				do {
					SystemTask();
					if (Ticks - theTime > 15) {
						theTime = Ticks;
						if (theLong > 10) {
							NumToString(theLong -= 5, theString);
							SetIText(theControl, theString);
						} else if (theLong > 1) {
								NumToString(theLong -= 1, theString);
								SetIText(theControl, theString);
						}
					}
				} while (Button());
			}
			box = save;
			box.left += 3;
			box.right -= 3;
			box.top += 3;
			box.bottom -= 3;
			EraseRect(&box);
			DrawPicture(pictHandles[noArrow], &save);
			RememberValues();
			break;

		case refrRect:
			if (Gestalt('sysv', &theLong) != noErr && theLong < 0x700)
				break;
			save = box;
			GlobalToLocal(&event->where);
			GetDItem(dp, lastItem - initDev + refreshSecs,
							&controlValue, &theControl, &box);
			GetIText(theControl, theString);
			StringToNum(theString, &theLong);
			theTime = Ticks - 16;
			if (PtInRect(event->where, &smRefrRect)) {			// higher!!!
				DrawPicture(pictHandles[upArrow], &save);
				SelIText(dp, lastItem - initDev + refreshSecs, 0, 32767);
				do {
					SystemTask();
					if (Ticks - theTime > 15) {
						theTime = Ticks;
						if (theLong < 20) {
							NumToString(theLong += 1, theString);
							SetIText(theControl, theString);
							if (theLong == 10)
								SelIText(dp, lastItem - initDev + refreshSecs, 0, 32767);
						}					}
				} while (Button());
			} else {											// lower.
				DrawPicture(pictHandles[dnArrow], &save);
				SelIText(dp, lastItem - initDev + refreshSecs, 0, 32767);
				do {
					SystemTask();
					if (Ticks - theTime > 15) {
						theTime = Ticks;
						if (theLong > 1) {
							NumToString(theLong -= 1, theString);
							SetIText(theControl, theString);
						}
					}
				} while (Button());
			}
			box = save;
			box.left += 3;
			box.right -= 3;
			box.top += 3;
			box.bottom -= 3;
			EraseRect(&box);
			DrawPicture(pictHandles[noArrow], &save);
			RememberValues();
			break;

		case screenOn:
		case clockOn:
		case startupIcon:
		case fadeWhite:
		case zeroMenu:
		case eraseRect:
		case eraseOval:
		case eraseRgn:
		case drawMenu:
		case initCurs:
			controlValue = GetCtlValue(theControl);
			controlValue = !controlValue;
			SetCtlValue(theControl, controlValue);
			RememberValues();
			break;

		default:
			break;
	}
}



/**********************************************************
 *	BBcdev::RememberValues
 *
 *	This saves the information in the CNFG resource, and,
 *	if the Basic Black INIT is loaded, sets the values of
 *	its patch globals in memory.
 **********************************************************/
void BBcdev::RememberValues(void)
{
	PrefStructureHandle	prefHandle;
	ControlHandle		theControl;
	short				controlValue;
	long				theLong;
	Rect				box;
	Str255				theString;
	PatchGlobalsPtr		pgPtr;

	/* load in the first 'PREF' resource */
	prefHandle = (PrefStructureHandle) GetIndResource('PREF', 1);

	/* save the idle time information */
	GetDItem(dp, lastItem - initDev + idleMins, &controlValue, &theControl, &box);
	GetIText(theControl, theString);
	StringToNum(theString, &theLong);
	(**prefHandle).idleTime = (short) theLong;

	/* save the refresh time information */
	GetDItem(dp, lastItem - initDev + refreshSecs, &controlValue, &theControl, &box);
	GetIText(theControl, theString);
	StringToNum(theString, &theLong);
	(**prefHandle).refreshSeconds = (short) theLong;


	/* save the sleep corner information */
	(**prefHandle).sleepNowCorner = sleepNowRect;
	
	/* save the wake corner information */
	(**prefHandle).sleepNeverCorner = neverSleepRect;

	/* save the clock information */
	GetDItem(dp, lastItem - initDev + clockOn, &controlValue, &theControl, &box);
	(**prefHandle).bounceClock = GetCtlValue(theControl);

	/* save the "screen saver on" information */
	GetDItem(dp, lastItem - initDev + screenOn, &controlValue, &theControl, &box);
	(**prefHandle).saverOn = GetCtlValue(theControl);

	/* save the "Startup Icon" information */
	GetDItem(dp, lastItem - initDev + startupIcon, &controlValue, &theControl, &box);
	(**prefHandle).showIcon = GetCtlValue(theControl);

	/* save the "Fade To White" information */
	GetDItem(dp, lastItem - initDev + fadeWhite, &controlValue, &theControl, &box);
	(**prefHandle).fadeToWhite = GetCtlValue(theControl);

	/* save the "Zero Menu Bar" information */
	GetDItem(dp, lastItem - initDev + zeroMenu, &controlValue, &theControl, &box);
	(**prefHandle).zeroBar = GetCtlValue(theControl);

	/* save the "EraseRect" information */
	GetDItem(dp, lastItem - initDev + eraseRect, &controlValue, &theControl, &box);
	(**prefHandle).eraseRectPatch = GetCtlValue(theControl);

	/* save the "EraseOval" information */
	GetDItem(dp, lastItem - initDev + eraseOval, &controlValue, &theControl, &box);
	(**prefHandle).eraseOvalPatch = GetCtlValue(theControl);

	/* save the "EraseRgn" information */
	GetDItem(dp, lastItem - initDev + eraseRgn, &controlValue, &theControl, &box);
	(**prefHandle).eraseRgnPatch = GetCtlValue(theControl);

	/* save the "DrawMenuBar" information */
	GetDItem(dp, lastItem - initDev + drawMenu, &controlValue, &theControl, &box);
	(**prefHandle).drawMenuBarPatch = GetCtlValue(theControl);

	/* save the "InitCursor" information */
	GetDItem(dp, lastItem - initDev + initCurs, &controlValue, &theControl, &box);
	(**prefHandle).initCursorPatch = GetCtlValue(theControl);

	/* If the extension's installed, then update it's patch globals */
	if (Gestalt('BBlk', (long *) &pgPtr) == noErr) {
		if (pgPtr->pgVersion == (**prefHandle).version) {
			pgPtr->pgMustSleep = false;
			pgPtr->pgSleepRect = (**prefHandle).sleepNowCorner;
			pgPtr->pgWakeRect = (**prefHandle).sleepNeverCorner;
			pgPtr->pgIdleTicks = (long) (**prefHandle).idleTime * 3600;
			pgPtr->pgBouncingClock = (**prefHandle).bounceClock;
			pgPtr->pgMustSave = (**prefHandle).saverOn;
			pgPtr->pgRefreshTime = (**prefHandle).refreshSeconds * 60;
			if ((**prefHandle).fadeToWhite) {				// Fade to white
				StuffHex(&(pgPtr->pgForePat), "\pFFFFFFFFFFFFFFFF");
				StuffHex(&(pgPtr->pgBackPat), "\p0000000000000000");
			} else {										// Fade to black
				StuffHex(&(pgPtr->pgForePat), "\p0000000000000000");
				StuffHex(&(pgPtr->pgBackPat), "\pFFFFFFFFFFFFFFFF");
			}
		}
	}

	ChangedResource(prefHandle);
	WriteResource(prefHandle);
	ReleaseResource(prefHandle);
}
