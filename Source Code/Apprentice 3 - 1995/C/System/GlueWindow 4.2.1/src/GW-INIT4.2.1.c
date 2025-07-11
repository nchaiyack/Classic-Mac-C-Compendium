//	GW-INIT4.2.1.c

#include	"GW-INIT4.2.1.h"

//	Declaration of global variables

QDGlobals		myQDGlobals;
SndListHandle	pushSndH, popSndH;
CursHandle		handCursorH, gripCursorH;
initDataHandle	settingH;

//	Main -----

void main(void)
{
	long	oldA4;
	
	oldA4 = SetCurrentA4();
	RememberA4();
	
	if(isSystem7()) {
		InitGraf(&myQDGlobals.thePort);
		setupINIT();
	}
	SetA4(oldA4);
}

Boolean isSystem7(void)
{
	Boolean	f = false;
	OSErr	err;
	long	res;
	
	if(trapAvailable(_Gestalt)) {
		err = Gestalt(gestaltSystemVersion, &res);
		if(err == noErr && res >= 0x0700)	f = true;
	}
	return(f);
}

//	Utils -----

Boolean trapAvailable(short theTrap)
{
	TrapType	tType;
	
	tType = getTrapType(theTrap);
	if(tType == ToolTrap)
		theTrap = theTrap & 0x07FF;
	if(theTrap >= numToolboxTrap())
		theTrap = _Unimplemented;
	
	return(NGetTrapAddress(theTrap, tType) !=
			NGetTrapAddress(_Unimplemented, ToolTrap));
}

TrapType getTrapType(short theTrap)
{
	if((theTrap & trapMask) > 0)
		return(ToolTrap);
	else
		return(OSTrap);
}

short numToolboxTrap(void)
{
	if(NGetTrapAddress(_InitGraf, ToolTrap) ==
			NGetTrapAddress(0xAA6E, ToolTrap))
		return(0x0200);
	else
		return(0x0400);
}

//

pascal Handle get1Resource(OSType rsrcType, short id)
{
	THz		oldZone;
	Handle	h;
	
	oldZone = GetZone();
	SetZone(SystemZone());
	h = Get1Resource(rsrcType, id);
	SetZone(oldZone);
	return(h);
}

pascal CursHandle getCursor(short id)
{
	THz			oldZone;
	CursHandle	h;
	
	oldZone = GetZone();
	SetZone(SystemZone());
	h = GetCursor(id);
	SetZone(oldZone);
	return(h);
}
//	Setup -----

void setupINIT(void)
{
	short				iconID = OK_cicn_id;
	Handle				h;
	UniversalProcPtr	awayAddress, growAddress, dragAddress;
	SelectorFunctionUPP	selectorFuncAddress;
	
	h = get1Resource('INIT', my_INIT_id);
	if(h) {
		settingH = loadSetting();
		if(settingH) {
			HLock(h);
			HNoPurge(h);
			DetachResource(h);
			
			awayAddress = NGetTrapAddress(_TrackGoAway,	ToolTrap);
			dragAddress = NGetTrapAddress(_DragWindow,	ToolTrap);
			growAddress = NGetTrapAddress(_GrowWindow,	ToolTrap);
			
			trackGoAwayProc	= (pascal Boolean (*)(WindowPtr, Point))awayAddress;
			dragWindowProc	= (pascal void (*)(WindowPtr, Point, Rect*))dragAddress;
			growWindowProc	= (pascal long (*)(WindowPtr, Point, Rect*))growAddress;
			
			NSetTrapAddress((UniversalProcPtr)trackGoAway, _TrackGoAway, ToolTrap);
			NSetTrapAddress((UniversalProcPtr)dragWindow, _DragWindow, ToolTrap);
			NSetTrapAddress((UniversalProcPtr)growWindow, _GrowWindow, ToolTrap);
			
			handCursorH = getCursor(hand_CURS_id);
			if(handCursorH) {
				HNoPurge((Handle)handCursorH);
				DetachResource((Handle)handCursorH);
			}
			
			gripCursorH = getCursor(grip_CURS_id);
			if(gripCursorH) {
				HNoPurge((Handle)gripCursorH);
				DetachResource((Handle)gripCursorH);
			}
			
			if(trapAvailable(_SndPlay)) {
				pushSndH	= (SndListHandle)get1Resource('snd ', push_sndx_id);
				popSndH		= (SndListHandle)get1Resource('snd ', pop_sndx_id);
				if(pushSndH) {
					DetachResource((Handle)pushSndH);
					HNoPurge((Handle)pushSndH);
				}
				else {
					(**settingH).pushSound = false;
				}
				if(popSndH) {
					DetachResource((Handle)popSndH);
					HNoPurge((Handle)popSndH);
				}
				else {
					(**settingH).popSound = false;
				}
			}
			
			selectorFuncAddress = (SelectorFunctionUPP)gestaltSelector;
			setSelectorFunc(selectorFuncAddress);
			
			if(!(**settingH).showIcon)
							iconID = NO_icon;
		}
		else {
			iconID = NG_cicn_id;
			ReleaseResource(h);
		}
	}
	else {
		iconID = NG_cicn_id;
	}
	drawIcon(iconID);
}

void drawIcon(short iconID)
{
	Handle		iconH = nil;
	GrafPtr		savedPortP;
	GrafPort	tempPort;
	Rect		r;
	short		screenWidth;
	Boolean		hasColor;
	
	if(iconID != NO_icon) {
		hasColor = trapAvailable(_GetCIcon);
		if(hasColor)
			iconH = (Handle)GetCIcon(iconID);
		else
			iconH = GetIcon(iconID);
		if(iconH) {
			HNoPurge(iconH);
			GetPort(&savedPortP);
			OpenPort(&tempPort);
			SetPort(&tempPort);
			if(((iconLoc_h << 1) ^ 0x1021) != iconLoc_cs)	iconLoc_h = 8;
			screenWidth = ((tempPort.portRect.right -
							tempPort.portRect.left) / 40) * 40;
			r.left	= iconLoc_h % screenWidth;
			r.right	= r.left + 32;
			r.top	= tempPort.portRect.bottom -
						(40 * (1 + iconLoc_h / screenWidth));
			r.bottom = r.top + 32;
			
			if(hasColor) {
				PlotCIcon(&r, (CIconHandle)iconH);
				DisposeCIcon((CIconHandle)iconH);
			}
			else {
				PlotIcon(&r, iconH);
				DisposeHandle(iconH);
			}
			
			iconLoc_h += 40;
			iconLoc_cs = (iconLoc_h << 1) ^ 0x1021;
			
			SetPort(savedPortP);
			ClosePort(&tempPort);
		}
	}
}

//	Selector -----

void setSelectorFunc(SelectorFunctionUPP selectorFuncAddress)
{
	long	oldA4;
	OSErr	err;
	
	oldA4 = SetUpA4();
	err = NewGestalt(myCreator, selectorFuncAddress);
	RestoreA4(oldA4);
}

pascal OSErr gestaltSelector(OSType selector, long *response)
{
	long	oldA4;
	
	oldA4 = SetUpA4();
	*response = (long)settingH;
	RestoreA4(oldA4);
	return(noErr);
}

//	Load -----

initDataHandle loadSetting(void)
{
	initDataHandle	dH = nil;
	prefsFileSpec	pfSpec;
	OSErr			err;
	short			savedRefNum;
	
	savedRefNum = CurResFile();
	err = openPrefsFile(&pfSpec);
	if(err == noErr) {
		dH = load420Data();
		if(dH == nil) {
			dH = load412Data();
		}
		if(dH == nil) {
			dH = load401Data();
		}
		if(dH == nil) {
			err = ResError();
			if(err == resNotFound) {
				dH = setDefaultData();
				if(dH) {
					saveSetting(dH);
				}
			}
		}
		CloseResFile(pfSpec.RsrcRefNum);
		UseResFile(savedRefNum);
	}
	else if(err == resFNotFound || err == fnfErr) {
		err = createPrefsFile();
		if(err == noErr)
			err = openPrefsFile(&pfSpec);
		if(err == noErr) {
			dH = setDefaultData();
			if(dH) {
				saveSetting(dH);
			}
			CloseResFile(pfSpec.RsrcRefNum);
			UseResFile(savedRefNum);
		}
	}
	
	return(dH);
}

initDataHandle load420Data(void)
{
	initDataHandle	dH = nil;

	dH = (initDataHandle)get1Resource(myDataType, gw420_DATA_id);
	if(dH) {
		HNoPurge((Handle)dH);
		DetachResource((Handle)dH);
	}
	return(dH);
}

initDataHandle load412Data(void)
{
	initDataHandle	dH = nil;
	gw412DataHandle	oldH = nil;
	
	oldH = (gw412DataHandle)get1Resource(myDataType, gw412_DATA_id);
	if(oldH) {
		dH = (initDataHandle)NewHandleSys(sizeof(initData));
		if(dH) {
			(**dH).dragOn		= (**oldH).dragOn;
			(**dH).noMarquee	= (**oldH).noMarquee;
			(**dH).glueOn		= defaultGlueOn;
			(**dH).growOn		= (**oldH).growOn;
			(**dH).pushOn		= (**oldH).pushOn;
			(**dH).popOn		= (**oldH).popOn;
			(**dH).pushSound	= (**oldH).pushSound;
			(**dH).popSound		= (**oldH).popSound;
			(**dH).dragKey[0]	= (**oldH).dragKey[0];
			(**dH).dragKey[1]	= (**oldH).dragKey[1];
			(**dH).dragKey[2]	= (**oldH).dragKey[2];
			(**dH).dragKey[3]	= (**oldH).dragKey[3];
			(**dH).glueKey[0]	= defaultGlueKey0;
			(**dH).glueKey[1]	= defaultGlueKey1;
			(**dH).glueKey[2]	= defaultGlueKey2;
			(**dH).glueKey[3]	= defaultGlueKey3;
			(**dH).pushKey[0]	= (**oldH).pushKey[0];
			(**dH).pushKey[1]	= (**oldH).pushKey[1];
			(**dH).pushKey[2]	= (**oldH).pushKey[2];
			(**dH).pushKey[3]	= (**oldH).pushKey[3];
			(**dH).popKey[0]	= (**oldH).popKey[0];
			(**dH).popKey[1]	= (**oldH).popKey[1];
			(**dH).popKey[2]	= (**oldH).popKey[2];
			(**dH).popKey[3]	= (**oldH).popKey[3];
			(**dH).showIcon		= (**oldH).showIcon;
			HNoPurge((Handle)dH);
			saveSetting(dH);
		}
		ReleaseResource((Handle)oldH);
	}
	return(dH);
}

initDataHandle load401Data(void)
{
	initDataHandle	dH = nil;
	gw401DataHandle	oldH;
	
	oldH = (gw401DataHandle)Get1Resource(myDataType, gw401_DATA_id);
	if(oldH) {
		dH = (initDataHandle)NewHandleSys(sizeof(initData));
		if(dH) {
			(**dH).dragOn		= (**oldH).dragOn;
			(**dH).noMarquee	= defaultNoMarquee;
			(**dH).glueOn		= defaultGlueOn;
			(**dH).growOn		= (**oldH).growOn;
			(**dH).pushOn		= (**oldH).pushOn;
			(**dH).popOn		= (**oldH).popOn;
			(**dH).pushSound	= (**oldH).pushSound;
			(**dH).popSound		= (**oldH).popSound;
			(**dH).dragKey[0]	= defaultDragKey0;
			(**dH).dragKey[1]	= defaultDragKey1;
			(**dH).dragKey[2]	= defaultDragKey2;
			(**dH).dragKey[3]	= defaultDragKey3;
			(**dH).glueKey[0]	= defaultGlueKey0;
			(**dH).glueKey[1]	= defaultGlueKey1;
			(**dH).glueKey[2]	= defaultGlueKey2;
			(**dH).glueKey[3]	= defaultGlueKey3;
			(**dH).pushKey[0]	= (**oldH).pushKey[0];
			(**dH).pushKey[1]	= (**oldH).pushKey[1];
			(**dH).pushKey[2]	= (**oldH).pushKey[2];
			(**dH).pushKey[3]	= (**oldH).pushKey[3];
			(**dH).popKey[0]	= (**oldH).popKey[0];
			(**dH).popKey[1]	= (**oldH).popKey[1];
			(**dH).popKey[2]	= (**oldH).popKey[2];
			(**dH).popKey[3]	= (**oldH).popKey[3];
			(**dH).showIcon		= (**oldH).showIcon;
			HNoPurge((Handle)dH);
			saveSetting(dH);
		}
		ReleaseResource((Handle)oldH);
	}
	return(dH);
}

initDataHandle setDefaultData(void)
{
	initDataHandle	dH;
	
	dH = (initDataHandle)NewHandleSys(sizeof(initData));
	if(dH) {		
		(**dH).dragOn		= defaultDragOn;
		(**dH).noMarquee	= defaultNoMarquee;
		(**dH).glueOn		= defaultGlueOn;
		(**dH).growOn		= defaultGrowOn;	//
		(**dH).pushOn		= defaultPushOn;
		(**dH).popOn		= defaultPopOn;
		(**dH).pushSound	= defaultPushSound;
		(**dH).popSound		= defaultPopSound;
		(**dH).dragKey[0]	= defaultDragKey0;
		(**dH).dragKey[1]	= defaultDragKey1;
		(**dH).dragKey[2]	= defaultDragKey2;
		(**dH).dragKey[3]	= defaultDragKey3;
		(**dH).glueKey[0]	= defaultGlueKey0;
		(**dH).glueKey[1]	= defaultGlueKey1;
		(**dH).glueKey[2]	= defaultGlueKey2;
		(**dH).glueKey[3]	= defaultGlueKey3;
		(**dH).pushKey[0]	= defaultPushKey0;
		(**dH).pushKey[1]	= defaultPushKey1;
		(**dH).pushKey[2]	= defaultPushKey2;
		(**dH).pushKey[3]	= defaultPushKey3;
		(**dH).popKey[0]	= defaultPopKey0;
		(**dH).popKey[1]	= defaultPopKey1;
		(**dH).popKey[2]	= defaultPopKey2;
		(**dH).popKey[3]	= defaultPopKey3;
		(**dH).showIcon		= defaultShowIcon;
		HNoPurge((Handle)dH);
	}
	return(dH);
}

OSErr openPrefsFile(prefsFileSpec *pfSpecP)
{
	OSErr	err;
	short	refNum;
	Str255	prefsFileName;
	
	GetIndString(prefsFileName, prefs_STRx_id, nameIndex);
	err = FindFolder(kOnSystemDisk, kPreferencesFolderType, kDontCreateFolder,
						&pfSpecP->vRefNum, &pfSpecP->DirID);
	if(err == noErr) {
		refNum = HOpenResFile(pfSpecP->vRefNum, pfSpecP->DirID,
													prefsFileName, fsRdWrPerm);	
		if(refNum == -1)
			return(ResError());
		else {
			pfSpecP->RsrcRefNum = refNum;
			return(noErr);
		}
	}
	return(err);
}

//	Save -----

void saveSetting(initDataHandle dH)
{
	initDataHandle	tempH;
	Handle			h;
	
	tempH = (initDataHandle)NewHandle(sizeof(initData));
	if(tempH) {
		**tempH = **dH;
		h = Get1Resource(myDataType, gw420_DATA_id);
		if(h)
			RmveResource(h);
		AddResource((Handle)tempH, myDataType, gw420_DATA_id, "\p");
		ReleaseResource((Handle)tempH);
	}
}

OSErr createPrefsFile(void)
{
	short			savedRefNum;
	OSErr			err;
	FInfo			finderInfo;
	prefsFileSpec	pfSpec;
	Str255			prefsFileName;
	
	GetIndString(prefsFileName, prefs_STRx_id, nameIndex);
	savedRefNum = CurResFile();
	err = FindFolder(kOnSystemDisk, kPreferencesFolderType, kDontCreateFolder,
						&pfSpec.vRefNum, &pfSpec.DirID);
	if(err == noErr) {
		HCreateResFile(pfSpec.vRefNum, pfSpec.DirID,
							prefsFileName);
		finderInfo.fdType		= 'pref';
		finderInfo.fdCreator	= myCreator;
		finderInfo.fdFlags		= prefsFileFlag;
		HSetFInfo(pfSpec.vRefNum, pfSpec.DirID, prefsFileName, &finderInfo);
	}
	UseResFile(savedRefNum);
	return(err);
}

//	Trap -----

pascal Boolean trackGoAway(WindowPtr wp, Point mp)
{
	Boolean		f;
	long		oldA4;
	
	oldA4 = SetUpA4();
	
	if((**settingH).popOn && checkKeys(&(**settingH).popKey[0]))
		f = customGoAway(wp, mp);
	else
		f = (*trackGoAwayProc)(wp, mp);
	
	RestoreA4(oldA4);
	return(f);
}

pascal void dragWindow(WindowPtr wp, Point mp, Rect *r)
{
	long	oldA4;
	KeyMap	keyMap;
	
	oldA4 = SetUpA4();
	
	if(!settingH) {
		(*dragWindowProc)(wp, mp, r);
	}
	else if((**settingH).noMarquee	&& checkKeys(&(**settingH).dragKey[0])) {
		directDrag(wp, mp, r);
	}
	else if((**settingH).glueOn		&& checkKeys(&(**settingH).glueKey[0])) {
		strongGlue(wp, mp, r);
	}
	else if((**settingH).pushOn		&& checkKeys(&(**settingH).pushKey[0])) {
		pushWindow(wp, mp, r);
	}
	else if((**settingH).popOn		&& checkKeys(&(**settingH).popKey[0])) {
		popWindow(wp, mp, r);
	}
	else if(!(**settingH).dragOn) {
		(*dragWindowProc)(wp, mp, r);
	}
	else
		customDrag(wp, mp, r);
	
	GetKeys(keyMap);
	if(!BitTst(keyMap, commandKeyBit))
		SelectWindow(wp);
	
	RestoreA4(oldA4);
}

pascal long growWindow(WindowPtr wp, Point mp, Rect *r)
{
	long	oldA4, value;
	
	oldA4 = SetUpA4();
	
	if(!settingH) {
		value = (*growWindowProc)(wp, mp, r);
	}
	else if(!(**settingH).growOn) {
		value = (*growWindowProc)(wp, mp, r);
	}
	else
		value = customGrow(wp, mp, r);
	
	RestoreA4(oldA4);
	return(value);
}

Boolean checkKeys(Boolean *key)
{
	KeyMap	keyMap;

	GetKeys(keyMap);
	if(	(key[0] == BitTst(keyMap, controlKeyBit))	&&
		(key[1] == BitTst(keyMap, shiftKeyBit))	&&
		(key[2] == BitTst(keyMap, optionKeyBit))	&&
		(key[3] == BitTst(keyMap, commandKeyBit))	)
		return(true);
	else
		return(false);
}

pascal void pushWindow(WindowPtr wp, Point mp, Rect *r)
{
	GrafPtr		savedPortP;
	OSErr		err;
	Point		pt;
	
	GetPort(&savedPortP);
	SetPort(wp);
	
	pt.h = wp->portRect.left;
	pt.v = wp->portRect.top;
	LocalToGlobal(&pt);
	if(PtInRect(pt, r)) {	
		stdState.left	= pt.h;
		stdState.top	= pt.v;
		
		SetPort(savedPortP);
		
		if((**settingH).pushSound) {
			if(trapAvailable(_SndPlay)) {
				if(pushSndH)
					err = SndPlay(nil, pushSndH, false);
				else
					SysBeep(1);
			}
			else
				SysBeep(1);
		}
	}
	else {
		customDrag(wp, mp, r);
	}	
}

pascal void popWindow(WindowPtr wp, Point mp, Rect *r)
{
	OSErr			err;
	
	popFunc(wp, r);
	if((**settingH).popSound) {
		if(trapAvailable(_SndPlay)) {
			if(popSndH)
				err = SndPlay(nil, popSndH, false);
			else
				SysBeep(1);
		}
		else
			SysBeep(1);
	}
}

pascal Boolean customGoAway(WindowPtr wp, Point mp)
{
	Boolean			f;
	OSErr			err;
	Rect			r;
	
	f = (*trackGoAwayProc)(wp, mp);
	if(f) {
		SetRect(&r, -32768, -32768, 32767, 32767);
		popFunc(wp, &r);
	}
	if((**settingH).popSound) {
		if(trapAvailable(_SndPlay)) {
			if(popSndH)
				err = SndPlay(nil, popSndH, false);
			else
				SysBeep(1);
		}
		else
			SysBeep(1);
	}
	return(f);
}

void popFunc(WindowPtr wp, Rect *r)
{
	GrafPtr			savedPortP;
	RgnHandle		grH;
	Rect			ristRect, titleRect;
	Point			pt, globalPt;
	
	GetPort(&savedPortP);
	SetPort(wp);
	
	globalPt.h = wp->portRect.left;
	globalPt.v = wp->portRect.top;
	LocalToGlobal(&globalPt);
	
	grH = LMGetGrayRgn();
	ristRect = (**grH).rgnBBox;
	InsetRect(&ristRect, 4, 4);
	SectRect(&ristRect, r, &ristRect);
	
	if(!EmptyRect(&wp->portRect)) {
		if(contRect.top - strucRect.top > contRect.left - strucRect.left) {
			SetRect(&titleRect, strucRect.left, strucRect.top,
										strucRect.right, contRect.top);
		}
		else {
			SetRect(&titleRect, strucRect.left, strucRect.top,
										contRect.left, strucRect.bottom);
		}
		if(titleRect.left > titleRect.right || titleRect.top > titleRect.bottom) {
			titleRect = strucRect;
		}
	}
	else {
		titleRect = strucRect;
	}
	
	pt.h = stdState.left;
	pt.v = stdState.top;

	OffsetRect(&titleRect, stdState.left - globalPt.h,
								stdState.top - globalPt.v);
	
	if(SectRect(&titleRect, &ristRect, &titleRect)) {
		MoveWindow(wp, pt.h, pt.v, false);
	}
	SetPort(savedPortP);
}

//	Drag -----

pascal void customDrag(WindowPtr wp, Point mp, Rect *r)
{
	GrafPtr		savedPortP;
	GrafPort	myPort;
	Point		oldPt, newPt, diff;
	short		offset_h, offset_v;
	WindowPeek	wpk;
	Boolean		h_isChanged, v_isChanged;
	Rect		wpkRect, ristRect;	//	Not restrict!
	short		wLeft, wTop;
	short		wWidth, wHeight;
	short		gapLeft, gapTop, gapRight, gapBottom;
	RgnHandle	grH;
	
	GetPort(&savedPortP);
	SetPort(wp);
	diff.h = wp->portRect.left;
	diff.v = wp->portRect.top;
	LocalToGlobal(&diff);
	diff.h -= contRect.left;
	diff.v -= contRect.top;
	OpenPort(&myPort);
	SetPort(&myPort);
	
	grH = LMGetGrayRgn();
	CopyRgn(grH, myPort.visRgn);
	myPort.portRect = (**grH).rgnBBox;
	offset_h	= mp.h - contRect.left;
	offset_v	= mp.v - contRect.top;
	gapLeft		= contRect.left		- strucRect.left;
	gapTop		= contRect.top		- strucRect.top;
	gapRight	= strucRect.right	- contRect.right;
	gapBottom	= strucRect.bottom	- contRect.bottom;
	wWidth	= contRect.right	- contRect.left;
	wHeight	= contRect.bottom	- contRect.top;
	
	PenSize(1, 1);
	PenPat(&myQDGlobals.gray);
	PenMode(patXor);
	oldPt = mp;
	wLeft	= oldPt.h - offset_h;
	wTop	= oldPt.v - offset_v;
	
	ristRect = myPort.portRect;
	InsetRect(&ristRect, 4, 4);
	SectRect(&ristRect, r, &ristRect);
	
	frameRgn(((WindowPeek)wp)->strucRgn, wLeft - gapLeft, wTop - gapTop);
	while(StillDown()) {
		GetMouse(&newPt);
		LocalToGlobal(&newPt);
		if(PtInRect(newPt, &ristRect)) {
			if(!EqualPt(newPt, oldPt)) {
				frameRgn(((WindowPeek)wp)->strucRgn, wLeft - gapLeft, wTop - gapTop);
				wLeft	= newPt.h - offset_h;
				wTop	= newPt.v - offset_v;
				h_isChanged = v_isChanged = false;
				wpk = (WindowPeek)FrontWindow();
				while(wpk)	{
					if(wpk != (WindowPeek)wp) {
						wpkRect = (**wpk->strucRgn).rgnBBox;
						if (!h_isChanged &&(abs(wpkRect.left - (wLeft - gapLeft)) <= 5)) {
							wLeft = wpkRect.left + gapLeft;
							h_isChanged = true;
						}
						if (!h_isChanged &&(abs(wpkRect.right - (wLeft - gapLeft)) <= 5)) {
							wLeft = wpkRect.right + gapLeft;
							h_isChanged = true;
						}
						if (!h_isChanged &&(abs(wpkRect.left - (wLeft + wWidth + gapRight)) <= 5)) {
							wLeft = wpkRect.left - wWidth - gapRight;
							h_isChanged = true;
						}
						if (!h_isChanged &&(abs(wpkRect.right - (wLeft + wWidth + gapRight)) <= 5)) {
							wLeft = wpkRect.right - wWidth - gapRight;
							h_isChanged = true;
						}
						if (!v_isChanged &&(abs(wpkRect.top - (wTop - gapTop)) <= 5)) {
							wTop = wpkRect.top + gapTop;
							v_isChanged = true;
						}
						if (!v_isChanged &&(abs(wpkRect.bottom - (wTop - gapTop)) <= 5)) {
							wTop = wpkRect.bottom + gapTop;
							v_isChanged = true;
						}
						if (!v_isChanged &&(abs(wpkRect.top - (wTop + wHeight + gapBottom)) <= 5)) {
							wTop = wpkRect.top - wHeight - gapBottom;
							v_isChanged = true;
						}
						if (!v_isChanged &&(abs(wpkRect.bottom - (wTop + wHeight + gapBottom)) <= 5)) {
							wTop = wpkRect.bottom - wHeight - gapBottom;
							v_isChanged = true;
						}
						wpk = wpk->nextWindow;
						if(h_isChanged && v_isChanged)	wpk = (WindowPeek)0;
					} else {
						wpk = wpk->nextWindow;
					}
				}
				frameRgn(((WindowPeek)wp)->strucRgn, wLeft - gapLeft, wTop - gapTop);
				oldPt = newPt;
			}
		}
	}
	frameRgn(((WindowPeek)wp)->strucRgn, wLeft - gapLeft, wTop - gapTop);
	MoveWindow(wp, wLeft + diff.h, wTop + diff.v, false);
	SetPort(savedPortP);
	ClosePort(&myPort);
}

pascal void strongGlue(WindowPtr wp, Point mp, Rect *r)
{
	GrafPtr		savedPortP;
	GrafPort	myPort;
	Point		oldPt, newPt, startPt, wkPt;
	short		offset_h, offset_v;
	WindowPeek	wpk;
	Boolean		h_isChanged, v_isChanged;
	Boolean		isSelected;
	Rect		wpkRect, ristRect, marqueeRect;
	short		wLeft, wTop;
	short		wWidth, wHeight;
	short		gapLeft, gapTop, gapRight, gapBottom;
	short		wNum, ct;
	RgnHandle	grH, invisRgnH, strucVisRgnH, clipRgnH;
	CursHandle	curs;
	WindowPeek	wList[MaxWindowList];	 

	GetPort(&savedPortP);
	OpenPort(&myPort);
	SetPort(&myPort);

	grH = LMGetGrayRgn();
	CopyRgn(grH, myPort.visRgn);
	myPort.portRect = (**grH).rgnBBox;
	curs = GetCursor(crossCursor);
	SetCursor(*curs);
	
	gapLeft		= contRect.left		- strucRect.left;
	gapTop		= contRect.top		- strucRect.top;
	gapRight	= strucRect.right	- contRect.right;
	gapBottom	= strucRect.bottom	- contRect.bottom;
	wWidth	= contRect.right	- contRect.left;
	wHeight	= contRect.bottom	- contRect.top;
	
	PenSize(1, 1);
	PenPat(&myQDGlobals.gray);
	PenMode(patXor);
	oldPt = mp;
	
	ristRect = myPort.portRect;
	InsetRect(&ristRect, 4, 4);
	SectRect(&ristRect, r, &ristRect);
	
	invisRgnH = NewRgn();
	strucVisRgnH = NewRgn();
	clipRgnH = NewRgn();
	
	SetRect(&marqueeRect, oldPt.h, oldPt.v, oldPt.h, oldPt.v);
	
	frameRect(&marqueeRect);
	wNum = 0;
	while(StillDown()) {
		GetMouse(&newPt);
		LocalToGlobal(&newPt);
		if(!EqualPt(newPt, oldPt)) {
			frameRect(&marqueeRect);
			marqueeRect.right	= newPt.h;
			marqueeRect.bottom	= newPt.v;
			frameRect(&marqueeRect);
			wpk = (WindowPeek)FrontWindow();
			SetEmptyRgn(invisRgnH);
			while(wpk)	{
				wpkRect = (**(wpk->strucRgn)).rgnBBox;
				DiffRgn(wpk->strucRgn, invisRgnH, strucVisRgnH);
				isSelected = false;
				for(ct = 0; ct <= wNum - 1; ct++) {
					if(wpk == wList[ct]) {
						isSelected = true;
					}
				}
				if(!isSelected) {
					if(!EqualRect(&myPort.portRect, &wpk->port.portRect) &&
						!EmptyRgn(strucVisRgnH) && 
							rectInRgn(&marqueeRect, strucVisRgnH)) {
						PenPat(&myQDGlobals.black);
						paintRgn(strucVisRgnH);
						UnionRgn(clipRgnH, strucVisRgnH, clipRgnH);
						PenPat(&myQDGlobals.gray);
						wList[wNum++] = wpk;
						if(wNum >= MaxWindowList)	wNum = MaxWindowList - 1;
					}
				}
				else {
					if(EmptyRgn(strucVisRgnH) ||
							!rectInRgn(&marqueeRect, strucVisRgnH)) {
						PenPat(&myQDGlobals.black);
						paintRgn(strucVisRgnH);
						DiffRgn(clipRgnH, strucVisRgnH, clipRgnH);
						PenPat(&myQDGlobals.gray);
						ct = 0;
						while(wList[ct] != wpk)	ct++;
						for(--wNum; ct <= wNum - 1; ct++)
							wList[ct] = wList[ct + 1];
					}
				}
				UnionRgn(invisRgnH, wpk->strucRgn, invisRgnH);
				SetEmptyRgn(strucVisRgnH);
				wpk = wpk->nextWindow;
			}
		}
		oldPt = newPt;
	}
	frameRect(&marqueeRect);
	PenPat(&myQDGlobals.black);
	paintRgn(clipRgnH);
	PenPat(&myQDGlobals.gray);
	
	DisposeRgn(strucVisRgnH);
	DisposeRgn(invisRgnH);
	DisposeRgn(clipRgnH);
	
// --------------------------------
	
	if(handCursorH)
		SetCursor(*handCursorH);
	for(ct = 0; ct <= wNum -1; ct++) {
		frameRgn(wList[ct]->strucRgn, (**(wList[ct]->strucRgn)).rgnBBox.left,
										(**(wList[ct]->strucRgn)).rgnBBox.top);
	}
	while(!Button()) {
	}
	FlushEvents(mDownMask + mUpMask, 0);
	if(gripCursorH)
		SetCursor(*gripCursorH);
	GetMouse(&oldPt);
	LocalToGlobal(&oldPt);
	startPt = oldPt;
	offset_h = startPt.h - contRect.left;
	offset_v = startPt.v - contRect.top;
	wLeft	= oldPt.h - offset_h;
	wTop	= oldPt.v - offset_v;
	
	while(StillDown()) {
		GetMouse(&newPt);
		LocalToGlobal(&newPt);
		if(PtInRect(newPt, &ristRect)) {
			if(!EqualPt(newPt, oldPt)) {
				for(ct = 0; ct <= wNum -1; ct++) {
					frameRgn(wList[ct]->strucRgn,
						(**(wList[ct]->strucRgn)).rgnBBox.left + wLeft - contRect.left,
							(**(wList[ct]->strucRgn)).rgnBBox.top + wTop - contRect.top);
				}
				wLeft	= newPt.h - offset_h;
				wTop	= newPt.v - offset_v;
				h_isChanged = v_isChanged = false;
				wpk = (WindowPeek)FrontWindow();
				while(wpk)	{
					if(wpk != (WindowPeek)wp) {
						wpkRect = (**wpk->strucRgn).rgnBBox;
						if (!h_isChanged &&(abs(wpkRect.left - (wLeft - gapLeft)) <= 5)) {
							wLeft = wpkRect.left + gapLeft;
							h_isChanged = true;
						}
						if (!h_isChanged &&(abs(wpkRect.right - (wLeft - gapLeft)) <= 5)) {
							wLeft = wpkRect.right + gapLeft;
							h_isChanged = true;
						}
						if (!h_isChanged &&(abs(wpkRect.left - (wLeft + wWidth + gapRight)) <= 5)) {
							wLeft = wpkRect.left - wWidth - gapRight;
							h_isChanged = true;
						}
						if (!h_isChanged &&(abs(wpkRect.right - (wLeft + wWidth + gapRight)) <= 5)) {
							wLeft = wpkRect.right - wWidth - gapRight;
							h_isChanged = true;
						}
						if (!v_isChanged &&(abs(wpkRect.top - (wTop - gapTop)) <= 5)) {
							wTop = wpkRect.top + gapTop;
							v_isChanged = true;
						}
						if (!v_isChanged &&(abs(wpkRect.bottom - (wTop - gapTop)) <= 5)) {
							wTop = wpkRect.bottom + gapTop;
							v_isChanged = true;
						}
						if (!v_isChanged &&(abs(wpkRect.top - (wTop + wHeight + gapBottom)) <= 5)) {
							wTop = wpkRect.top - wHeight - gapBottom;
							v_isChanged = true;
						}
						if (!v_isChanged &&(abs(wpkRect.bottom - (wTop + wHeight + gapBottom)) <= 5)) {
							wTop = wpkRect.bottom - wHeight - gapBottom;
							v_isChanged = true;
						}
						wpk = wpk->nextWindow;
						if(h_isChanged && v_isChanged)	wpk = (WindowPeek)0;
					} else {
						wpk = wpk->nextWindow;
					}
				}
				for(ct = 0; ct <= wNum -1; ct++) {
					frameRgn(wList[ct]->strucRgn,
						(**(wList[ct]->strucRgn)).rgnBBox.left + wLeft - contRect.left,
							(**(wList[ct]->strucRgn)).rgnBBox.top + wTop - contRect.top);
				}
				oldPt = newPt;
			}
		}
	}
	for(ct = 0; ct <= wNum -1; ct++) {
		frameRgn(wList[ct]->strucRgn,
			(**(wList[ct]->strucRgn)).rgnBBox.left + wLeft - contRect.left,
				(**(wList[ct]->strucRgn)).rgnBBox.top + wTop - contRect.top);
	}
	offset_h = wLeft - contRect.left;
	offset_v = wTop - contRect.top;
	for(ct = 0; ct <= wNum -1; ct++) {
		if(titleInRect(wList[ct], offset_h, offset_v, &ristRect)) {
			wkPt.h = ((WindowPtr)wList[ct])->portRect.left;
			wkPt.v = ((WindowPtr)wList[ct])->portRect.top;
			getGlobal(wList[ct], &wkPt);
			MoveWindow((WindowPtr)wList[ct], wkPt.h + offset_h,
												wkPt.v + offset_v, false);
		}
	}

	if(handCursorH)
		SetCursor(&myQDGlobals.arrow);
	
	SetPort(savedPortP);
	ClosePort(&myPort);
}

pascal void directDrag(WindowPtr wp, Point mp, Rect *r)
{
	GrafPtr		savedPortP;
	Point		oldPt, newPt, diff;
	short		offset_h, offset_v;
	WindowPeek	wpk;
	Boolean		h_isChanged, v_isChanged;
	Rect		wpkRect, ristRect;
	short		wLeft, wTop;
	short		wWidth, wHeight;
	short		gapLeft, gapTop, gapRight, gapBottom;
	RgnHandle	grH;
	
	GetPort(&savedPortP);
	SetPort(wp);
	diff.h = wp->portRect.left;
	diff.v = wp->portRect.top;
	LocalToGlobal(&diff);
	diff.h -= contRect.left;
	diff.v -= contRect.top;
	
	offset_h	= mp.h - contRect.left;
	offset_v	= mp.v - contRect.top;
	gapLeft		= contRect.left		- strucRect.left;
	gapTop		= contRect.top		- strucRect.top;
	gapRight	= strucRect.right	- contRect.right;
	gapBottom	= strucRect.bottom	- contRect.bottom;
	wWidth	= contRect.right	- contRect.left;
	wHeight	= contRect.bottom	- contRect.top;
	
	oldPt = mp;
	
	grH = LMGetGrayRgn();
	ristRect = (**grH).rgnBBox;
	InsetRect(&ristRect, 4, 4);
	SectRect(&ristRect, r, &ristRect);
	
	while(StillDown()) {
		GetMouse(&newPt);
		LocalToGlobal(&newPt);
		if(PtInRect(newPt, &ristRect)) {
			if(!EqualPt(newPt, oldPt)) {
				wLeft = newPt.h - offset_h;
				wTop  = newPt.v - offset_v;
				h_isChanged = v_isChanged = false;

				wpk = (WindowPeek)FrontWindow();
				while(wpk)	{
					if(wpk != (WindowPeek)wp) {
						wpkRect = (**wpk->strucRgn).rgnBBox;
						if (!h_isChanged &&(abs(wpkRect.left - (wLeft - gapLeft)) <= 5)) {
							wLeft = wpkRect.left + gapLeft;
							h_isChanged = true;
						}
						if (!h_isChanged &&(abs(wpkRect.right - (wLeft - gapLeft)) <= 5)) {
							wLeft = wpkRect.right + gapLeft;
							h_isChanged = true;
						}
						if (!h_isChanged &&(abs(wpkRect.left - (wLeft + wWidth + gapRight)) <= 5)) {
							wLeft = wpkRect.left - wWidth - gapRight;
							h_isChanged = true;
						}
						if (!h_isChanged &&(abs(wpkRect.right - (wLeft + wWidth + gapRight)) <= 5)) {
							wLeft = wpkRect.right - wWidth - gapRight;
							h_isChanged = true;
						}
						if (!v_isChanged &&(abs(wpkRect.top - (wTop - gapTop)) <= 5)) {
							wTop = wpkRect.top + gapTop;
							v_isChanged = true;
						}
						if (!v_isChanged &&(abs(wpkRect.bottom - (wTop - gapTop)) <= 5)) {
							wTop = wpkRect.bottom + gapTop;
							v_isChanged = true;
						}
						if (!v_isChanged &&(abs(wpkRect.top - (wTop + wHeight + gapBottom)) <= 5)) {
							wTop = wpkRect.top - wHeight - gapBottom;
							v_isChanged = true;
						}
						if (!v_isChanged &&(abs(wpkRect.bottom - (wTop + wHeight + gapBottom)) <= 5)) {
							wTop = wpkRect.bottom - wHeight - gapBottom;
							v_isChanged = true;
						}
						wpk = wpk->nextWindow;
						if(h_isChanged && v_isChanged)	wpk = (WindowPeek)0;
					} else {
						wpk = wpk->nextWindow;
					}
				}
			
				MoveWindow(wp, wLeft + diff.h, wTop + diff.v, false);
				oldPt = newPt;
			}
		}
	}
	SetPort(savedPortP);
}

pascal long customGrow(WindowPtr wp, Point mp, Rect *r)
{
	GrafPtr		savedPortP;
	GrafPort	myPort;
	Point		oldPt, newPt, pt;
	short		offset_h, offset_v;
	WindowPeek	wpk;
	Boolean		h_isChanged, v_isChanged;
	Rect		wpkRect, ristRect;
	short		wRight, wBottom;
	short		wWidth, wHeight;
	short		gapLeft, gapTop, gapRight, gapBottom;
	long		value = 0, size;
	
	GetPort(&savedPortP);
	OpenPort(&myPort);
	SetPort(&myPort);
	
	offset_h	= contRect.right	- mp.h;
	offset_v	= contRect.bottom	- mp.v;
	gapLeft		= contRect.left		- strucRect.left;
	gapTop		= contRect.top		- strucRect.top;
	gapRight	= strucRect.right	- contRect.right;
	gapBottom	= strucRect.bottom	- contRect.bottom;
	wWidth	= contRect.right	- contRect.left;
	wHeight	= contRect.bottom	- contRect.top;
	
	ristRect = *r;
	if(ristRect.right	> myPort.portRect.right)
				ristRect.right	= myPort.portRect.right;
	if(ristRect.bottom	> myPort.portRect.bottom)
				ristRect.bottom	= myPort.portRect.bottom;
	OffsetRect(&ristRect, contRect.left, contRect.top);
	
	PenSize(1, 1);
	PenPat(&myQDGlobals.gray);
	PenMode(patXor);
	oldPt = mp;
	wRight	= oldPt.h + offset_h;
	wBottom	= oldPt.v + offset_v;
	
	drawMarquee(((WindowPeek)wp)->strucRgn, wRight, wBottom, gapTop);
	while(StillDown()) {
		GetMouse(&newPt);
		LocalToGlobal(&newPt);
		pt = newPt;
		pt.h += offset_h;
		pt.v += offset_v;
		if(!PtInRect(pt, &ristRect)) {
			size = PinRect(&ristRect, pt);
			if(pt.v != HiWord(size))
				newPt.v = HiWord(size) - offset_v;
			if(pt.h != LoWord(size))
				newPt.h = LoWord(size) - offset_h;
		}
		if(!EqualPt(newPt, oldPt)) {
			drawMarquee(((WindowPeek)wp)->strucRgn, wRight, wBottom, gapTop);
			wRight	= newPt.h + offset_h;
			wBottom	= newPt.v + offset_v;
			h_isChanged = v_isChanged = false;
			wpk = (WindowPeek)FrontWindow();
			while(wpk)	{
				if(wpk != (WindowPeek)wp) {
					wpkRect = (**wpk->strucRgn).rgnBBox;
					if (!h_isChanged &&(abs(wpkRect.left - (wRight + gapRight)) <= 5)) {
						wRight = wpkRect.left - gapRight;
						h_isChanged = true;
					}
					if (!h_isChanged &&(abs(wpkRect.right - (wRight + gapRight)) <= 5)) {
						wRight = wpkRect.right - gapRight;
						h_isChanged = true;
					}
					
					if (!v_isChanged &&(abs(wpkRect.top - (wBottom + gapBottom)) <= 5)) {
						wBottom = wpkRect.top - gapBottom;
						v_isChanged = true;
					}
					if (!v_isChanged &&(abs(wpkRect.bottom - (wBottom + gapBottom)) <= 5)) {
						wBottom = wpkRect.bottom - gapBottom;
						v_isChanged = true;
					}
					
					wpk = wpk->nextWindow;
					if(h_isChanged && v_isChanged)	wpk = (WindowPeek)0;
				} else {
					wpk = wpk->nextWindow;
				}
			}
		
			drawMarquee(((WindowPeek)wp)->strucRgn, wRight, wBottom, gapTop);
			oldPt = newPt;
		}
	}
	drawMarquee(((WindowPeek)wp)->strucRgn, wRight, wBottom, gapTop);
	SetPort(savedPortP);
	ClosePort(&myPort);
	if((mp.h != oldPt.h) || (mp.v != oldPt.v)) {
		value = wBottom - contRect.top;
		value <<= 16;
		value += wRight - contRect.left;
	}
	return(value);
}

void drawMarquee(RgnHandle rh, short globalRight, short globalBottom, short gapTop)
{
	RgnHandle	localRgnH;
	Point		globalPt, localPt;
	Rect		marqueeRect, localRect;
	
	localRgnH = NewRgn();
	CopyRgn(rh, localRgnH);
	globalPt.h = (**localRgnH).rgnBBox.right;
	globalPt.v = (**localRgnH).rgnBBox.bottom;
	localPt = globalPt;
	
	GlobalToLocal(&localPt);
	OffsetRgn(localRgnH, globalPt.h - localPt.h, globalPt.v - localPt.v);
	
	SetRect(&localRect, (**localRgnH).rgnBBox.left, (**localRgnH).rgnBBox.top,
				(**localRgnH).rgnBBox.right, (**localRgnH).rgnBBox.bottom);
	
	SetRect(&marqueeRect, localRect.left, localRect.top,
				localRect.right + (globalRight - globalPt.h) + 1,
				localRect.bottom + (globalBottom - globalPt.v) + 1);
	FrameRect(&marqueeRect);
	MoveTo(marqueeRect.left,	marqueeRect.top + gapTop - 1);
	LineTo(marqueeRect.right,	marqueeRect.top + gapTop - 1);
	MoveTo(marqueeRect.left,	marqueeRect.bottom - 16);
	LineTo(marqueeRect.right,	marqueeRect.bottom - 16);
	MoveTo(marqueeRect.right - 16, marqueeRect.top + gapTop);
	LineTo(marqueeRect.right - 16, marqueeRect.bottom);
	
	DisposeRgn(localRgnH);
}

void frameRgn(RgnHandle rh, short globalLeft, short globalTop)
{
	RgnHandle	localRgnH;
	Point		localPt, globalPt;
	
	localRgnH = NewRgn();
	CopyRgn(rh, localRgnH);
	globalPt.h = (**localRgnH).rgnBBox.left;
	globalPt.v = (**localRgnH).rgnBBox.top;
	localPt = globalPt;
	
	GlobalToLocal(&localPt);
	OffsetRgn(localRgnH, (globalLeft - globalPt.h) + (localPt.h - globalPt.h),
						(globalTop - globalPt.v) + (localPt.v- globalPt.v));
	
	FrameRgn(localRgnH);
	DisposeRgn(localRgnH);
}

void frameRect(Rect *rect)
{
	short	wk;
	Rect	r = *rect;
	Point	localPt, globalPt;
	
	globalPt.h = r.left;
	globalPt.v = r.top;
	localPt = globalPt;
	GlobalToLocal(&localPt);
	
	if(r.left > r.right) {
		wk = r.right;
		r.right = r.left;
		r.left = wk;
	}
	else {
		r.right++;
	}
	
	if(r.top > r.bottom) {
		wk = r.bottom;
		r.bottom = r.top;
		r.top = wk;
	}
	else {
		r.bottom++;
	}
	
	OffsetRect(&r, localPt.h - globalPt.h, localPt.v - globalPt.v);
	FrameRect(&r);
}

Boolean rectInRgn(Rect *rect, RgnHandle vRgn)
{
	short	wk;
	Rect	mRect = *rect;

	if(mRect.left > mRect.right) {
		wk = mRect.right;
		mRect.right = mRect.left;
		mRect.left = wk;
	}
	else	mRect.right++;
	
	if(mRect.top > mRect.bottom) {
		wk = mRect.bottom;
		mRect.bottom = mRect.top;
		mRect.top = wk;
	}
	else	mRect.bottom++;
	
	return(RectInRgn(&mRect, vRgn));
}

void paintRgn(RgnHandle strucVisRgnH)
{
	Point		localPt, globalPt;
	RgnHandle	localVisRgnH;
	
	globalPt.h = (**strucVisRgnH).rgnBBox.left;
	globalPt.v = (**strucVisRgnH).rgnBBox.top;
	localPt = globalPt;
	GlobalToLocal(&localPt);
	
	localVisRgnH = NewRgn();
	CopyRgn(strucVisRgnH, localVisRgnH);
	OffsetRgn(localVisRgnH, localPt.h - globalPt.h, localPt.v - globalPt.v);
	
	PaintRgn(localVisRgnH);
	
	DisposeRgn(localVisRgnH);
}

Boolean titleInRect(WindowPeek wPeek, short h, short v, Rect *r)
{
	WindowPtr	wp;
	Rect		titleRect, dummy;

	wp = (WindowPtr)wPeek;
	if(!EmptyRect(&wp->portRect)) {
		if(contRect.top - strucRect.top > contRect.left - strucRect.left) {
			SetRect(&titleRect, strucRect.left, strucRect.top,
										strucRect.right, contRect.top);
		}
		else {
			SetRect(&titleRect, strucRect.left, strucRect.top,
										contRect.left, strucRect.bottom);
		}
		if(titleRect.left > titleRect.right || titleRect.top > titleRect.bottom)
			titleRect = strucRect;
	}
	else {
		titleRect = strucRect;
	}
	OffsetRect(&titleRect, h, v);
	return(SectRect(&titleRect, r, &dummy));
}

void getGlobal(WindowPeek wPeek, Point *pt)
{
	GrafPtr		savedPortP;
	
	GetPort(&savedPortP);
	SetPort((WindowPtr)wPeek);
	
	LocalToGlobal(pt);
	
	SetPort(savedPortP);
}
