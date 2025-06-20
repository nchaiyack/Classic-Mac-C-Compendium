//	GW-cdev4.2.1.c

#include	"GW-cdev4.2.1.h"

pascal long main(short msg, short item, short numItems, short panelID,
					EventRecord *eventP, long val, DialogPtr dp)
{
	long	retVal;
		
	retVal = val;
	switch(msg) {
		case nulDev:
		case keyEvtDev:
		case undoDev:
		case cutDev:
		case copyDev:
		case clearDev:
		case activDev:
		case deactivDev:
			break;
		case updateDev:
			setupDialogItems((initDataHandle)val, dp, numItems);
			break;
		case macDev:
			retVal = checkMachine();
			break;
		case initDev:
			retVal = initPanel(dp, numItems);
			break;
		case closeDev:
			closePanel((initDataHandle)val);
			break;
		case hitDev:
			hitItems((initDataHandle)val, dp, item + numItems, eventP, numItems);
			break;
		default:
			break;
	}
	return(retVal);
}

//

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

//	Load -----

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

OSErr openPrefsFile(prefsFileSpec *pfSpecP)
{
	OSErr	err;
	short	refNum;
	Str255	prefsFileName;

	GetIndString(prefsFileName, prefs_STRx_id, nameIndex);
	err = FindFolder(kOnSystemDisk, kPreferencesFolderType, kDontCreateFolder,
						&(pfSpecP->vRefNum), &(pfSpecP->DirID));
	if(err == noErr) {
		refNum = HOpenResFile(pfSpecP->vRefNum, pfSpecP->DirID,
													prefsFileName, fsRdWrPerm);	
		if(refNum == -1) {
			return(ResError());
		}
		else {
			pfSpecP->RsrcRefNum = refNum;
			return(noErr);
		}
	}
	return(err);
}

void saveSetting(initDataHandle dH)
{
	initDataHandle	tempH;
	Handle			h;
	
	tempH = (initDataHandle)NewHandle(sizeof(initData));
	if(tempH) {
		**tempH = **dH;
		h = Get1Resource(myDataType, gw420_DATA_id);
		if(h)
			RemoveResource(h);
		AddResource((Handle)tempH, myDataType, gw420_DATA_id, "\p");
		ReleaseResource((Handle)tempH);
	}
}

//	Event routines	-----

long checkMachine(void)
{
	long	retVal = 0;
	
	if(trapAvailable(_Gestalt)) {
		if(isSystem7()) {
			retVal = 1;
		}
	}
	if(retVal == 0) {
		StopAlert(system7_ALRT_id, nil);
	}
	return(retVal);
}

long initPanel(DialogPtr dp, short numItems)
{
	initDataHandle	ih = nil, workH = nil;
	OSErr			err;
	short			savedRefNum;
	prefsFileSpec	pf;
	Str255			str;
	
	SetPort(dp);
	ih = getSetting();
	if(ih) {
		savedRefNum = CurResFile();
		err = openPrefsFile(&pf);
		if(err == noErr) {
			workH = load420Data();
			CloseResFile(pf.RsrcRefNum);
		}
		else {
			NumToString(err, str);
			ParamText(str, "\p", "\p", "\p");
			CautionAlert(error_ALRT_id, nil);
		}
		UseResFile(savedRefNum);
		
		if(workH) {
			setupDialogItems(ih, dp, numItems);
			DisposeHandle((Handle)workH);
		}
		else {
			StopAlert(prefs_ALRT_id, nil);
			return(cdevGenErr);
		}
	}
	else {
		StopAlert(restart_ALRT_id, nil);
		return(cdevGenErr);
	}
	return((long)ih);
}

initDataHandle getSetting(void)
{
	initDataHandle	ih = nil;
	long			ihAddress;
	OSErr			err;
	
	err = Gestalt(myCreator, &ihAddress);
	if(err == noErr) {
		ih = (initDataHandle)ihAddress;
	}
	return(ih);
}

void hitItems(initDataHandle ih, DialogPtr dp, short item,
				EventRecord *eventP, short numItems)
{
	short	itemType;
	Handle	h;
	Rect	r;

	switch(item) {
		case checkDragOn:
			(**ih).dragOn = !(**ih).dragOn;
			setCheckButton(dp, checkDragOn		+ numItems, (**ih).dragOn);
			if((**ih).noMarquee && (**ih).dragOn)
				setActive(ih, dp, numItems, checkDragCtrl);
			else
				setInactive(ih, dp, numItems, checkDragCtrl);
			break;
		case checkNoMarquee:
			(**ih).noMarquee = !(**ih).noMarquee;
			setCheckButton(dp, checkNoMarquee	+ numItems, (**ih).noMarquee);
			if((**ih).noMarquee && (**ih).dragOn)
				setActive(ih, dp, numItems, checkDragCtrl);
			else
				setInactive(ih, dp, numItems, checkDragCtrl);
			break;
		case checkGlueOn:
			(**ih).glueOn = !(**ih).glueOn;
			setCheckButton(dp, checkGlueOn		+ numItems, (**ih).glueOn);
			if((**ih).glueOn)
				setActive(ih, dp, numItems, checkGlueCtrl);
			else
				setInactive(ih, dp, numItems, checkGlueCtrl);
			break;
		case checkGrowOn:
			(**ih).growOn = !(**ih).growOn;
			setCheckButton(dp, checkGrowOn		+ numItems, (**ih).growOn);
			break;
		case checkPushOn:
			(**ih).pushOn = !(**ih).pushOn;
			setCheckButton(dp, checkPushOn		+ numItems, (**ih).pushOn);
			if((**ih).pushOn) {
				GetDialogItem(dp, numItems + checkPushSound, &itemType, &h, &r);
				HiliteControl((ControlHandle)h, itemActive);
				setActive(ih, dp, numItems, checkPushCtrl);
			}
			else {
				GetDialogItem(dp, numItems + checkPushSound, &itemType, &h, &r);
				HiliteControl((ControlHandle)h, itemInactive);
				setInactive(ih, dp, numItems, checkPushCtrl);
			}
			break;
		case checkPopOn:
			(**ih).popOn = !(**ih).popOn;
			setCheckButton(dp, checkPopOn		+ numItems, (**ih).popOn);
			if((**ih).popOn) {
				GetDialogItem(dp, numItems + checkPopSound, &itemType, &h, &r);
				HiliteControl((ControlHandle)h, itemActive);
				setActive(ih, dp, numItems, checkPopCtrl);
			}
			else {
				GetDialogItem(dp, numItems + checkPopSound, &itemType, &h, &r);
				HiliteControl((ControlHandle)h, itemInactive);
				setInactive(ih, dp, numItems, checkPopCtrl);
			}
			break;
		case checkPushSound:
			(**ih).pushSound = !(**ih).pushSound;
			setCheckButton(dp, checkPushSound	+ numItems, (**ih).pushSound);
			break;
		case checkPopSound:
			(**ih).popSound = !(**ih).popSound;
			setCheckButton(dp, checkPopSound	+ numItems, (**ih).popSound);
			break;
		case checkShowIcon:
			(**ih).showIcon = !(**ih).showIcon;
			setCheckButton(dp, checkShowIcon	+ numItems, (**ih).showIcon);
			break;
		case checkDragCtrl:
		case checkDragShift:
		case checkDragOption:
		case checkDragCommand:
			if((**ih).noMarquee) {
				(**ih).dragKey[item - checkDragCtrl]
					= !(**ih).dragKey[item - checkDragCtrl];
			}
			setCheckButton(dp, checkDragCtrl	+ numItems, (**ih).dragKey[0]);
			setCheckButton(dp, checkDragShift	+ numItems, (**ih).dragKey[1]);
			setCheckButton(dp, checkDragOption	+ numItems, (**ih).dragKey[2]);
			setCheckButton(dp, checkDragCommand	+ numItems, (**ih).dragKey[3]);
			break;
		case checkGlueCtrl:
		case checkGlueShift:
		case checkGlueOption:
		case checkGlueCommand:
			if((**ih).glueOn) {
				(**ih).glueKey[item - checkGlueCtrl]
					= !(**ih).glueKey[item - checkGlueCtrl];
			}
			setCheckButton(dp, checkGlueCtrl	+ numItems, (**ih).glueKey[0]);
			setCheckButton(dp, checkGlueShift	+ numItems, (**ih).glueKey[1]);
			setCheckButton(dp, checkGlueOption	+ numItems, (**ih).glueKey[2]);
			setCheckButton(dp, checkGlueCommand	+ numItems, (**ih).glueKey[3]);
			break;
		case checkPushCtrl:
		case checkPushShift:
		case checkPushOption:
		case checkPushCommand:
			if((**ih).pushOn) {
				(**ih).pushKey[item - checkPushCtrl]
					= !(**ih).pushKey[item - checkPushCtrl];
				checkKeyCombination(ih, item - checkPushCtrl);
			}
			setCheckButton(dp, checkPushCtrl	+ numItems, (**ih).pushKey[0]);
			setCheckButton(dp, checkPushShift	+ numItems, (**ih).pushKey[1]);
			setCheckButton(dp, checkPushOption	+ numItems, (**ih).pushKey[2]);
			setCheckButton(dp, checkPushCommand	+ numItems, (**ih).pushKey[3]);
			break;
		case checkPopCtrl:
		case checkPopShift:
		case checkPopOption:
		case checkPopCommand:
			if((**ih).popOn) {
				(**ih).popKey[item - checkPopCtrl]
					= !(**ih).popKey[item - checkPopCtrl];
				checkKeyCombination(ih, item - checkPopCtrl);
			}
			setCheckButton(dp, checkPopCtrl		+ numItems, (**ih).popKey[0]);
			setCheckButton(dp, checkPopShift	+ numItems, (**ih).popKey[1]);
			setCheckButton(dp, checkPopOption	+ numItems, (**ih).popKey[2]);
			setCheckButton(dp, checkPopCommand	+ numItems, (**ih).popKey[3]);
			break;
		case buttonDragCtrl:
		case buttonDragShift:
		case buttonDragOption:
		case buttonDragCommand:
			if((**ih).noMarquee) {
				if(trackButton(dp, numItems, item, item - buttonDragCtrl)) {
					(**ih).dragKey[item - buttonDragCtrl]
						= !(**ih).dragKey[item - buttonDragCtrl];
				}
			}
			setCheckButton(dp, checkDragCtrl	+ numItems, (**ih).dragKey[0]);
			setCheckButton(dp, checkDragShift	+ numItems, (**ih).dragKey[1]);
			setCheckButton(dp, checkDragOption	+ numItems, (**ih).dragKey[2]);
			setCheckButton(dp, checkDragCommand	+ numItems, (**ih).dragKey[3]);
			break;
		case buttonGlueCtrl:
		case buttonGlueShift:
		case buttonGlueOption:
		case buttonGlueCommand:
			if((**ih).glueOn) {
				if(trackButton(dp, numItems, item, item - buttonGlueCtrl)) {
					(**ih).glueKey[item - buttonGlueCtrl]
						= !(**ih).glueKey[item - buttonGlueCtrl];
				}
			}
			setCheckButton(dp, checkGlueCtrl	+ numItems, (**ih).glueKey[0]);
			setCheckButton(dp, checkGlueShift	+ numItems, (**ih).glueKey[1]);
			setCheckButton(dp, checkGlueOption	+ numItems, (**ih).glueKey[2]);
			setCheckButton(dp, checkGlueCommand	+ numItems, (**ih).glueKey[3]);
			break;
		case buttonPushCtrl:
		case buttonPushShift:
		case buttonPushOption:
		case buttonPushCommand:
			if((**ih).pushOn) {
				if(trackButton(dp, numItems, item, item - buttonPushCtrl)) {
					(**ih).pushKey[item - buttonPushCtrl]
						= !(**ih).pushKey[item - buttonPushCtrl];
					checkKeyCombination(ih, item - buttonPushCtrl);
				}
			}
			setCheckButton(dp, checkPushCtrl	+ numItems, (**ih).pushKey[0]);
			setCheckButton(dp, checkPushShift	+ numItems, (**ih).pushKey[1]);
			setCheckButton(dp, checkPushOption	+ numItems, (**ih).pushKey[2]);
			setCheckButton(dp, checkPushCommand	+ numItems, (**ih).pushKey[3]);
			break;
		case buttonPopCtrl:
		case buttonPopShift:
		case buttonPopOption:
		case buttonPopCommand:
			if((**ih).popOn) {
				if(trackButton(dp, numItems, item, item - buttonPopCtrl)) {
					(**ih).popKey[item - buttonPopCtrl]
						= !(**ih).popKey[item - buttonPopCtrl];
					checkKeyCombination(ih, item - buttonPopCtrl);
				}
			}
			setCheckButton(dp, checkPopCtrl		+ numItems, (**ih).popKey[0]);
			setCheckButton(dp, checkPopShift	+ numItems, (**ih).popKey[1]);
			setCheckButton(dp, checkPopOption	+ numItems, (**ih).popKey[2]);
			setCheckButton(dp, checkPopCommand	+ numItems, (**ih).popKey[3]);
			break;
		case buttonAbout:
			if(trackAbout(dp, numItems))
				about(dp);
			break;
		default:
			break;
	}
}

void setupDialogItems(initDataHandle ih, DialogPtr dp, short numItems)
{
	setCheckButton(dp, checkDragOn		+ numItems, (**ih).dragOn);
	setCheckButton(dp, checkNoMarquee	+ numItems, (**ih).noMarquee);
	setCheckButton(dp, checkGlueOn		+ numItems, (**ih).glueOn);
	setCheckButton(dp, checkGrowOn		+ numItems, (**ih).growOn);
	setCheckButton(dp, checkPushOn		+ numItems, (**ih).pushOn);
	setCheckButton(dp, checkPopOn		+ numItems, (**ih).popOn);
	setCheckButton(dp, checkPushSound	+ numItems, (**ih).pushSound);
	setCheckButton(dp, checkPopSound	+ numItems, (**ih).popSound);
	setCheckButton(dp, checkShowIcon	+ numItems, (**ih).showIcon);
	setCheckButton(dp, checkDragCtrl	+ numItems, (**ih).dragKey[0]);
	setCheckButton(dp, checkDragShift	+ numItems, (**ih).dragKey[1]);
	setCheckButton(dp, checkDragOption	+ numItems, (**ih).dragKey[2]);
	setCheckButton(dp, checkDragCommand	+ numItems, (**ih).dragKey[3]);
	setCheckButton(dp, checkGlueCtrl	+ numItems, (**ih).glueKey[0]);
	setCheckButton(dp, checkGlueShift	+ numItems, (**ih).glueKey[1]);
	setCheckButton(dp, checkGlueOption	+ numItems, (**ih).glueKey[2]);
	setCheckButton(dp, checkGlueCommand	+ numItems, (**ih).glueKey[3]);
	setCheckButton(dp, checkPushCtrl	+ numItems, (**ih).pushKey[0]);
	setCheckButton(dp, checkPushShift	+ numItems, (**ih).pushKey[1]);
	setCheckButton(dp, checkPushOption	+ numItems, (**ih).pushKey[2]);
	setCheckButton(dp, checkPushCommand	+ numItems, (**ih).pushKey[3]);
	setCheckButton(dp, checkPopCtrl		+ numItems, (**ih).popKey[0]);
	setCheckButton(dp, checkPopShift	+ numItems, (**ih).popKey[1]);
	setCheckButton(dp, checkPopOption	+ numItems, (**ih).popKey[2]);
	setCheckButton(dp, checkPopCommand	+ numItems, (**ih).popKey[3]);
	
	setHilite(ih, dp, numItems);
}

void setHilite(initDataHandle ih, DialogPtr dp, short numItems)
{
	short 		itemType;
	Handle		h, iconH;
	Rect		r;
	Boolean		colorIcon;
	
	colorIcon = trapAvailable(_GetCIcon);
	GetDialogItem(dp, numItems + buttonAbout, &itemType, &h, &r);
	if(colorIcon) {
		iconH = (Handle)GetCIcon(aboutup_cicn_id);
		PlotCIcon(&r, (CIconHandle)iconH);
		DisposeCIcon((CIconHandle)iconH);
	}
	else {
		iconH = GetIcon(aboutup_cicn_id);
		PlotIcon(&r, iconH);
		DisposeHandle(iconH);
	}
	
	if((**ih).noMarquee && (**ih).dragOn)
		setActive(ih, dp, numItems, checkDragCtrl);
	else
		setInactive(ih, dp, numItems, checkDragCtrl);
	
	if((**ih).glueOn)
		setActive(ih, dp, numItems, checkGlueCtrl);
	else
		setInactive(ih, dp, numItems, checkGlueCtrl);
	
	
	if((**ih).pushOn) {
		GetDialogItem(dp, numItems + checkPushSound, &itemType, &h, &r);
		HiliteControl((ControlHandle)h, itemActive);
		setActive(ih, dp, numItems, checkPushCtrl);
	}
	else {
		GetDialogItem(dp, numItems + checkPushSound, &itemType, &h, &r);
		HiliteControl((ControlHandle)h, itemInactive);
		setInactive(ih, dp, numItems, checkPushCtrl);
	}
	
	if((**ih).popOn) {
		GetDialogItem(dp, numItems + checkPopSound, &itemType, &h, &r);
		HiliteControl((ControlHandle)h, itemActive);
		setActive(ih, dp, numItems, checkPopCtrl);
	}
	else {
		GetDialogItem(dp, numItems + checkPopSound, &itemType, &h, &r);
		HiliteControl((ControlHandle)h, itemInactive);
		setInactive(ih, dp, numItems, checkPopCtrl);
	}
}

void setActive(initDataHandle ih, DialogPtr dp, short numItems, short item)
{
	short 		itemType, i;
	Handle		h, activeIconH, textIconH;
	Rect		r;
	Boolean		colorIcon;
	
	colorIcon = trapAvailable(_GetCIcon);
	if(colorIcon)
		activeIconH = (Handle)GetCIcon(active_cicn_id);
	else
		activeIconH = GetIcon(active_cicn_id);
		
	for(i = 0; i <= 3; i++) {
		GetDialogItem(dp, numItems + item + i, &itemType, &h, &r);
		HiliteControl((ControlHandle)h, itemActive);
		GetDialogItem(dp, numItems + item + 4 + i, &itemType, &h, &r);
		if(colorIcon) {
			PlotCIcon(&r, (CIconHandle)activeIconH);
			textIconH = (Handle)GetCIcon(ctrl_cicn_id + i);
		}
		else {
			PlotIcon(&r, activeIconH);
			textIconH = GetIcon(ctrl_cicn_id + i);
		}
		r.left	+= 6;
		r.top	+= 5;
		r.right	-= 10;
		r.bottom-= 3;
		if(colorIcon) {
			PlotCIcon(&r, (CIconHandle)textIconH);
			DisposeCIcon((CIconHandle)textIconH);
		}
		else {
			PlotIcon(&r, textIconH);
			DisposeHandle(textIconH);
		}
	}
	if(colorIcon)
		DisposeCIcon((CIconHandle)activeIconH);
	else
		DisposeHandle(activeIconH);
}

void setInactive(initDataHandle ih, DialogPtr dp, short numItems, short item)
{
	short 		itemType, i;
	Handle		h, iconH;
	Rect		r;
	Boolean		colorIcon;
	
	colorIcon = trapAvailable(_GetCIcon);
	if(colorIcon)
		iconH = (Handle)GetCIcon(inactive_cicn_id);
	else
		iconH = GetIcon(inactive_cicn_id);
	
	for(i = 0; i <= 3; i++) {
		GetDialogItem(dp, numItems + item + i, &itemType, &h, &r);
		HiliteControl((ControlHandle)h, itemInactive);
		GetDialogItem(dp, numItems + item + 4 + i, &itemType, &h, &r);
		if(colorIcon)
			PlotCIcon(&r,(CIconHandle)iconH);
		else
			PlotIcon(&r, iconH);
	}
	if(colorIcon)
		DisposeCIcon((CIconHandle)iconH);
	else
		DisposeHandle(iconH);
}

void setCheckButton(DialogPtr dp, short item, Boolean isOn)
{
	short	itemType;
	Handle	h;
	Rect	r;
	
	GetDialogItem(dp, item, &itemType, &h, &r);
	if(isOn)	SetControlValue((ControlHandle)h, 1);
	else		SetControlValue((ControlHandle)h, 0);
}

void checkKeyCombination(initDataHandle ih, short key)
{
	if(	((**ih).pushKey[0] == false) &&
		((**ih).pushKey[1] == false) &&
		((**ih).pushKey[2] == false) &&
		((**ih).pushKey[3] == false) )		(**ih).pushKey[key] = true;
	if(	((**ih).popKey[0] == false) &&
		((**ih).popKey[1] == false) &&
		((**ih).popKey[2] == false) &&
		((**ih).popKey[3] == false) )		(**ih).popKey[key] = true;
}

Boolean trackButton(DialogPtr dp, short numItems, short item, short key)
{
	short	itemType;
	Handle	h, downIconH, upIconH, textIconH;
	Rect	buttonRect, textRect;
	Point	pt;
	Boolean	keyDown, colorIcon;
	
	colorIcon = trapAvailable(_GetCIcon);
	GetDialogItem(dp, numItems + item, &itemType, &h, &buttonRect);
	textRect = buttonRect;
	textRect.left	+= 6;
	textRect.top	+= 4;
	textRect.right	-= 10;
	textRect.bottom-= 4;
	if(colorIcon) {
		upIconH = (Handle)GetCIcon(active_cicn_id);
		downIconH = (Handle)GetCIcon(down_cicn_id);
		textIconH = (Handle)GetCIcon(key + ctrl_cicn_id);
		PlotCIcon(&buttonRect, (CIconHandle)downIconH);
		PlotCIcon(&textRect, (CIconHandle)textIconH);
	}
	else {
		upIconH = GetIcon(active_cicn_id);
		downIconH = GetIcon(down_cicn_id);
		textIconH = GetIcon(key + ctrl_cicn_id);
		PlotIcon(&buttonRect, downIconH);
		PlotIcon(&textRect, textIconH);
	}
	keyDown = true;
	while(StillDown()) {
		GetMouse(&pt);
		if(!PtInRect(pt, &buttonRect)) {
			if(keyDown) {
				OffsetRect(&textRect, 0, 1);
				if(colorIcon) {
					PlotCIcon(&buttonRect, (CIconHandle)upIconH);
					PlotCIcon(&textRect, (CIconHandle)textIconH);
				}
				else {
					PlotIcon(&buttonRect, upIconH);
					PlotIcon(&textRect, textIconH);
				}
				keyDown = false;				
			}
		}
		else if(!keyDown) {
			OffsetRect(&textRect, 0, -1);
			if(colorIcon) {
				PlotCIcon(&buttonRect, (CIconHandle)downIconH);
				PlotCIcon(&textRect, (CIconHandle)textIconH);
			}
			else {
				PlotIcon(&buttonRect, downIconH);
				PlotIcon(&textRect, textIconH);
			}
			keyDown = true;
		}
	}
	if(keyDown) {
		OffsetRect(&textRect, 0, 1);
		if(colorIcon) {
			PlotCIcon(&buttonRect, (CIconHandle)upIconH);
			PlotCIcon(&textRect, (CIconHandle)textIconH);
		}
		else {
			PlotIcon(&buttonRect, upIconH);
			PlotIcon(&textRect, textIconH);
		}
	}
	if(colorIcon) {
		DisposeCIcon((CIconHandle)downIconH);
		DisposeCIcon((CIconHandle)upIconH);
		DisposeCIcon((CIconHandle)textIconH);
	}
	else {
		DisposeHandle(downIconH);
		DisposeHandle(upIconH);
		DisposeHandle(textIconH);
	}
	return(keyDown);
}

void closePanel(initDataHandle ih)
{
	short			savedRefNum, i;
	prefsFileSpec	pf;
	Str255			str[4];
	
	for(i = 0; i <= 3; i++) {
		GetIndString(str[i], keys_STRx_id, i + 1);
	}
	if(	(**ih).pushOn && (**ih).noMarquee &&
					checkSameKeys((**ih).pushKey, (**ih).dragKey) ) {
		ParamText(str[0], str[2], "\p", "\p");
		NoteAlert(samekeys_ALRT_id, nil);
		(**ih).pushOn = false;
	}
	if(	(**ih).dragOn && (**ih).popOn &&
					checkSameKeys((**ih).dragKey, (**ih).popKey) ) {
		ParamText(str[0], str[3], "\p", "\p");
		NoteAlert(samekeys_ALRT_id, nil);
		(**ih).popOn = false;
	}
	if(	(**ih).pushOn && (**ih).popOn &&
					checkSameKeys((**ih).pushKey, (**ih).popKey) ) {
		ParamText(str[2], str[3], "\p", "\p");
		NoteAlert(samekeys_ALRT_id, nil);
		(**ih).popOn = false;
	}
	if(	(**ih).pushOn && (**ih).glueOn &&
					checkSameKeys((**ih).pushKey, (**ih).glueKey) ) {
		ParamText(str[1], str[2], "\p", "\p");
		NoteAlert(samekeys_ALRT_id, nil);
		(**ih).pushOn = false;
	}
	if(	(**ih).glueOn && (**ih).popOn &&
					checkSameKeys((**ih).glueKey, (**ih).popKey) ) {
		ParamText(str[1], str[3], "\p", "\p");
		NoteAlert(samekeys_ALRT_id, nil);
		(**ih).popOn = false;
	}
	if(	(**ih).glueOn && (**ih).dragOn &&
					checkSameKeys((**ih).glueKey, (**ih).dragKey) ) {
		ParamText(str[0], str[1], "\p", "\p");
		NoteAlert(samekeys_ALRT_id, nil);
		(**ih).glueOn = false;
	}
	savedRefNum = CurResFile();	
	openPrefsFile(&pf);
	saveSetting(ih);
	CloseResFile(pf.RsrcRefNum);
	UseResFile(savedRefNum);
}

Boolean checkSameKeys(Boolean *key1, Boolean *key2)
{
	if(	(*(key1 + 0) == *(key2 + 0)) &&
		(*(key1 + 1) == *(key2 + 1)) &&
		(*(key1 + 2) == *(key2 + 2)) &&
		(*(key1 + 3) == *(key2 + 3)) ) 		return(true);
		
	return(false);
}
		
Boolean trackAbout(DialogPtr dp, short numItems)
{
	short	itemType;
	Handle	h, downIconH, upIconH;
	Rect	buttonRect;
	Point	pt;
	Boolean	keyDown, colorIcon;
	
	colorIcon = trapAvailable(_GetCIcon);
	GetDialogItem(dp, numItems + buttonAbout, &itemType, &h, &buttonRect);
	if(colorIcon) {
		upIconH = (Handle)GetCIcon(aboutup_cicn_id);
		downIconH = (Handle)GetCIcon(aboutdown_cicn_id);
		PlotCIcon(&buttonRect, (CIconHandle)downIconH);
	}
	else {
		upIconH = GetIcon(aboutup_cicn_id);
		downIconH = GetIcon(aboutdown_cicn_id);
		PlotIcon(&buttonRect, downIconH);
	}
	keyDown = true;
	while(StillDown()) {
		GetMouse(&pt);
		if(!PtInRect(pt, &buttonRect)) {
			if(keyDown) {
				if(colorIcon)
					PlotCIcon(&buttonRect, (CIconHandle)upIconH);
				else
					PlotIcon(&buttonRect, upIconH);
				keyDown = false;				
			}
		}
		else if(!keyDown) {
			if(colorIcon)
				PlotCIcon(&buttonRect, (CIconHandle)downIconH);
			else
				PlotIcon(&buttonRect, downIconH);
			keyDown = true;
		}
	}
	if(keyDown) {
		if(colorIcon)
			PlotCIcon(&buttonRect, (CIconHandle)upIconH);
		else
			PlotIcon(&buttonRect, upIconH);
	}
	if(colorIcon) {
		DisposeCIcon((CIconHandle)downIconH);
		DisposeCIcon((CIconHandle)upIconH);
	}
	else {
		DisposeHandle(downIconH);
		DisposeHandle(upIconH);
	}
	
	return(keyDown);
}

void about(DialogPtr dp)
{
	GrafPtr			savedPortP;
	PenState		savedPenState;
	WindowPtr		wp;
	EventRecord		anEvent;
	PicHandle		titleH;
	Handle			iconH;
	Rect			r;
	Str255			str;
	short			i;
	long			numOfStr;
	Boolean			exitDraw = false, colorIcon;
	Pattern			gray = {0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA};

	GetPort(&savedPortP);
	GetPenState(&savedPenState);

	SetRect(&r, 5, 5, 5 + 143, 5 + 30);	//	143, 30 are width and height of title.
	wp = GetNewWindow(about_WIND_id, 0, (WindowPtr)-1);
	ShowWindow(wp);
	SetPort(wp);
	titleH = GetPicture(title_PICT_id);
	DrawPicture(titleH, &r);
	PenNormal();
	TextSize(9);
	GetIndString(str, numof_STRx_id, 1);
	StringToNum(str, &numOfStr);
	for(i = 0; i <= numOfStr; i++) {
		GetIndString(str, about_STRx_id, i + 1);
		MoveTo(20, 45 + i * 12);
		DrawString(str); 
	}
	SetRect(&r, 280, 0, 280 + 48, 0 + 48);	//	48, 48 are width and height of window icon.
	colorIcon = trapAvailable(_GetCIcon);
	if(colorIcon) {
		iconH = (Handle)GetCIcon(window_cicn_id);
		PlotCIcon(&r, (CIconHandle)iconH);
	}
	else {
		iconH = GetIcon(window_cicn_id);
		PlotIcon(&r, iconH);
	}
	SetRect(&r, 1, 0, 49, 48);
	PenPat(&gray);
	PenMode(patXor);
	FrameRect(&r);
	while(!exitDraw) {
		FrameRect(&r);
		OffsetRect(&r, 2, 0);
		FrameRect(&r);
		if(r.left == (280 - 47)) {
			exitDraw = true;
			if(colorIcon)
				PlotCIcon(&r, (CIconHandle)iconH);
			else
				PlotIcon(&r, iconH);
		}
	}
	while(!(WaitNextEvent(mDownMask, &anEvent, 100, 0))) {
	}
	
	if(colorIcon)
		DisposeCIcon((CIconHandle)iconH);
	else
		DisposeHandle(iconH);
	DisposeWindow(wp);
	SetPort(savedPortP);
	SetPenState(&savedPenState);
	FlushEvents(everyEvent, 0);
}
