#include "Global.h"
#include "Utilities.h"
#include "Pack3.h"				// for gRefNum
#include "PLStringFuncs.h"		// PStringCopy

Boolean				MoveBeforeSystemFile(short refNum);
Boolean				MoveToEndOfResourceChain(short refNum);

//--------------------------------------------------------------------------------
/*
	Glue to toolbox.
*/

#if 0
pascal void	AppendDITL(DialogPtr dlg, Handle items, DITLMethod method)
{
	asm
	{
		MOVE.L		(A7)+,D0
		MOVE.W		#0x0402,-(A7)
		MOVEA.L		A7,A0
		MOVE.L		D0,-(A7)
		DC.W		0xA08B				; _CommToolboxDispatch
		MOVEA.L		(A7)+,A0
		LEA			0x000C(A7),A7
		JMP			(A0)
	}
}

pascal short	CountDITL(DialogPtr dlg)
{
	asm
	{
		MOVE.L		(A7)+,D0
		MOVE.W		#0x0403,-(A7)
		MOVEA.L		A7,A0
		MOVE.L		D0,-(A7)
		DC.W		0xA08B				; _CommToolboxDispatch
		MOVEA.L		(A7)+,A0
		MOVE.W		D0,(A7)
		JMP			(A0)
	}
}
#endif

//--------------------------------------------------------------------------------
/*
	Return TRUE if the Command key is down.
*/

Boolean	CommandKeyIsDown()
{
	const short kCommandKey = 0x37;

	return KeyIsDown(kCommandKey);
}


//--------------------------------------------------------------------------------
/*
	Return TRUE if the Control key is down.
*/

Boolean	ControlKeyIsDown()
{
	const short kControlKey = 0x3B;

	return KeyIsDown(kControlKey);
}


//--------------------------------------------------------------------------------
/*
	Handy utility for creating a dialog the 7.0 way. It creates the dialog,
	sets the port to that dialog, and then calls the 7.0 routines that set the
	default item, the cancel item, and turn on cursor tracking over edit text
	items.
*/

DialogPtr	CreateNewDialog(short dlgID, short okID, short cancelID)
{
	DialogPtr	dlg;

	dlg = GetNewDialog(dlgID, NIL, (WindowPtr) -1);

	if (okID)
		SetDialogDefaultItem(dlg, okID);
	if (cancelID)
		SetDialogCancelItem(dlg, cancelID);
//	if (track)
		SetDialogTracksCursor(dlg, TRUE);

	SetPort(dlg);

	return dlg;
}


//--------------------------------------------------------------------------------
/*
	Flash the given item in the given dialog. Useful for simulating a click on
	a button in response to a key press.

	We don’t currently use this routine, so it’s commented out to save space
	(THINK C doesn’t seem to dead-code strip it.
*/

#ifdef __NEVER__
void	FlashDialogItem(DialogPtr dlg, short itemToFlash)
{
	Handle	control;
	long	ignored;

	control = GetItemHandle(dlg, itemToFlash);
	HiliteControl(control, 1);
	Delay(8, &ignored);
	HiliteControl(control, 0);
}
#endif


//--------------------------------------------------------------------------------
/*
	Assuming the given dialog item is a control, return the control’s value.
*/

short	GetDialogItemState(DialogPtr dlg, short controlNumber)
{
	return GetCtlValue((ControlHandle) GetItemHandle(dlg, controlNumber));
}


//--------------------------------------------------------------------------------
/*
	Return the “kind” of the given dialog item.

	We don’t currently use this routine, so it’s commented out to save space
	(THINK C doesn’t seem to dead-code strip it.
*/

#ifdef __NEVER__
short	GetItemKind(DialogPtr dlg, short item)
{
	short	iKind;
	Handle	iHandle;
	Rect	iRect;

	GetDItem(dlg, item, &iKind, &iHandle, &iRect);
	return iKind;
}
#endif


//--------------------------------------------------------------------------------
/*
	Return the handle associated with the given item.
*/

Handle	GetItemHandle(DialogPtr dlg, short item)
{
	short	iKind;
	Handle	iHandle;
	Rect	iRect;

	GetDItem(dlg, item, &iKind, &iHandle, &iRect);
	return iHandle;
}


//--------------------------------------------------------------------------------
/*
	Return the bounding rectangle of the given dialog item.
*/

Rect	GetItemRect(DialogPtr dlg, short item)
{
	short	iKind;
	Handle	iHandle;
	Rect	iRect;

	GetDItem(dlg, item, &iKind, &iHandle, &iRect);
	return iRect;
}


//--------------------------------------------------------------------------------
/*
	Return the volume in the VCB queue that follows the given volume. If the
	given volume is at the end of the queue, wrap back to the beginning, even
	if this means returning the original volume. This can happen if the given
	volume is the only one mounted.
*/

short	GetNextVolume(short thisVolume)
{
	OSErr			err;
	HParamBlockRec	pbVol;
	short			index;

	index = 0;
	pbVol.volumeParam.ioNamePtr = NIL;

	do {
		pbVol.volumeParam.ioVolIndex = ++index;
		err = PBHGetVInfoSync(&pbVol);
		if (err == nsvErr) {
			ReportError(kNoStartVolume, err);
			return -1;
		}
	} while (pbVol.volumeParam.ioVRefNum != thisVolume);

	pbVol.volumeParam.ioVolIndex = ++index;
	err = PBHGetVInfoSync(&pbVol);
	if (err == nsvErr) {
		pbVol.volumeParam.ioVolIndex = 1;
		err = PBHGetVInfoSync(&pbVol);
	}

	return pbVol.volumeParam.ioVRefNum;
}


//--------------------------------------------------------------------------------
/*
	Read in the 'vers' resource with the given ID, and return either the short
	or long version strings, depending on the value of the “getShort”
	parameter. Return an empty string if we could’t load the resource.
*/

void	GetVersionString(short versID, Boolean getShort, StringPtr theString)
{
	VersRecHndl		versHandle;
	StringPtr		stringPtr;

	theString[0] = 0;
	versHandle = (VersRecHndl) MyGetResource('vers', versID);
	if (versHandle != NIL) {
		stringPtr = (StringPtr) &(**versHandle).shortVersion;
		if (!getShort) {
			stringPtr += *stringPtr;
		}

		PStringCopy(theString, stringPtr);
	}
}


//--------------------------------------------------------------------------------
/*
	Simple utility to copy a StringHandle into a Str255 (or whatever).
*/

void	HandleToStr255(StringHandle sh, StringPtr s)
{
	PStringCopy(s, *sh);
}


//--------------------------------------------------------------------------------
/*
	Return TRUE if the key indicatedby keyCode is currently down.
*/

Boolean	KeyIsDown(short keyCode)
{
	union {
		KeyMap			asMap;
		unsigned char	asBytes[16];
	} myMap;

	GetKeys(myMap.asMap);
	return ((myMap.asBytes[keyCode >> 3] >> (keyCode & 0x07)) & 1) != 0;
}


//--------------------------------------------------------------------------------
/*
	Similar to NewString, except that this one creates a string in the System
	Heap. Since NewString creates the string in whatever zone is current, we
	switch over to the System zone, create the string, and then switch back to
	whatever was current when we were called.
*/

StringHandle	NewStringSys(ConstStr255Param s)
{
	THz				currentZone;
	StringHandle	result;

	currentZone = GetZone();
	SetZone(SystemZone());
	result = NewString(s);
	SetZone(currentZone);
	return result;
}


//--------------------------------------------------------------------------------
/*
	Return TRUE if the option key is down.
*/

Boolean	OptionKeyIsDown()
{
	const short kOptionKey = 0x3A;

	return KeyIsDown(kOptionKey);
}


//--------------------------------------------------------------------------------
/*
	General purpose routine to display an error message. This function is
	entered with a value indicating a message string and an error number. Both
	the message and the error number are displayed in a dialog if we can load
	the dialog. If not, we break into Macsbug and display the information
	there.
*/

void	ReportError(short errStringIndex, short errorNumber)
{
	Boolean	closeIt;
	GrafPtr	oldPort;
	Str255	errText;
	Str255	errNumberText;

	gRefNum = MyOpenResFile(gMe, fsRdPerm, &closeIt);

	if (gRefNum != -1) {
		if (errorNumber != 0)
			NumToString(errorNumber, errNumberText);
		else
			errNumberText[0] = 0;

		GetIndString(errText, kErrorStrings, errStringIndex);

		ParamText(errText, errNumberText, NIL, NIL);

		GetPort(&oldPort);
		(void) StopAlert(kErrorAlert, NIL);
		
		if (OptionKeyIsDown())
			Debugger();

		SetPort(oldPort);

		if (closeIt) {
			CloseResFile(gRefNum);
			gRefNum = -1;
		}
	} else {
		DebugStr("\pI cannot scream, for I have no mouth.");
	}
}


//--------------------------------------------------------------------------------
/*
	Assuming that the given item is a control, set its value.
*/

void	SetDialogItemState(DialogPtr dlg, short controlNumber, short value)
{
	SetCtlValue((ControlHandle) GetItemHandle(dlg, controlNumber), value);
}


//--------------------------------------------------------------------------------
/*
	Set the “kind” of the given dialog item.

	We don’t currently use this routine, so it’s commented out to save space
	(THINK C doesn’t seem to dead-code strip it.
*/

#ifdef __NEVER__
void	SetItemKind(DialogPtr dlg, short item, short kind)
{
	short	iKind;
	Handle	iHandle;
	Rect	iRect;

	GetDItem(dlg, item, &iKind, &iHandle, &iRect);
	SetDItem(dlg, item, kind, iHandle, &iRect);
}
#endif


//--------------------------------------------------------------------------------
/*
	Set the handle associated with the given item.
*/

void	SetItemHandle(DialogPtr dlg, short item, Handle handle)
{
	short	iKind;
	Handle	iHandle;
	Rect	iRect;

	GetDItem(dlg, item, &iKind, &iHandle, &iRect);
	SetDItem(dlg, item, iKind, handle, &iRect);
}


//--------------------------------------------------------------------------------
/*
	Set the bounding rectangle of the given dialog item.
*/

void	SetItemRect(DialogPtr dlg, short item, Rect* rect)
{
	short	iKind;
	Handle	iHandle;
	Rect	iRect;

	GetDItem(dlg, item, &iKind, &iHandle, &iRect);
	SetDItem(dlg, item, iKind, iHandle, rect);
}


//--------------------------------------------------------------------------------
/*
	Assuming that “buttonNumber” and “*previousRadio” are radio buttons (or,
	at least, are controls), turn off previousRadio, turn on buttonNumber, and
	return buttonNumber in previousRadio. We check that the two buttons are
	different before doing anything.
*/

void	SetRadioButton(DialogPtr dlg, short buttonNumber, short* previousRadio)
{
	if (buttonNumber != *previousRadio) {
		if (*previousRadio != 0)
			SetDialogItemState(dlg, *previousRadio, 0);
		*previousRadio = buttonNumber;
		SetDialogItemState(dlg, *previousRadio, 1);
	}
}


//--------------------------------------------------------------------------------
/*
	Assuming that the given item is a checkBox (or, at least, a control), toggle
	its value between zero and non-zero.
*/

void	ToggleCheckBox(DialogPtr dlg, short buttonNumber)
{
	short newState;

	if (GetDialogItemState(dlg, buttonNumber) == 0)
		newState = 1;
	else
		newState = 0;
	SetDialogItemState(dlg, buttonNumber, newState);
}


//--------------------------------------------------------------------------------
/*
	Returns TRUE if the given volume supports PBCatSearch. This is done
	by calling GetVolParms for that volume and checking the bHasCatSearch bit
	in vMAttrib.

	Note that just because a volume supports PBCatSearch doesn’t mean that you
	can call PBCatSearch. If the volume you are checking is a remote volume,
	you must make sure that the host machine you are running on also supports
	PBCatSearch. This means checking for System 7.0. Since we check for 7.0
	when we install the INIT, we don’t check again here.
*/

Boolean VolHasCatSearch(short vRefNum)
{
	HParamBlockRec			pb;
	GetVolParmsInfoBuffer	buffer;

	pb.ioParam.ioNamePtr = NIL;
	pb.ioParam.ioVRefNum = vRefNum;
	pb.ioParam.ioBuffer = (Ptr) &buffer;
	pb.ioParam.ioReqCount = sizeof(GetVolParmsInfoBuffer);
	if (PBHGetVolParmsSync(&pb) != noErr) {
		return FALSE;
	}
	return (buffer.vMAttrib & (1 << bHasCatSearch));
}


//--------------------------------------------------------------------------------

typedef struct ResourceMap{
	long		dataOffset;
	long		mapOffset;
	long		dataLength;
	long		mapLength;
	struct ResourceMap	**nextMap;
	short		refNum;
	short		attributes;
	short		typesOffset;
	short		namesOffset;
} ResourceMap, *ResMapPtr, **ResMapHandle;


//--------------------------------------------------------------------------------
/*
	My special version of FSpOpenResFile. If our resource fork is already
	open, we do nothing except return our refNum and set closeIt to FALSE. If
	our resource fork is not open yet, we open it, return the refNum, and set
	closeIt to TRUE.

	In order to not interfere with the running application, we move our
	resource map just before the system file’s and behind the application’s.
*/

short MyOpenResFile(FSSpecPtr file, short perm, Boolean *closeIt)
{
	short		oldFile;
	short		refNum;
	Boolean		result;

	*closeIt = FALSE;
	refNum = GetMyRefNum(file);
	if (refNum == -1) {
		oldFile = CurResFile();
		refNum = FSpOpenResFile(file, perm);
		UseResFile(oldFile);
		if (refNum != -1) {
		//	result = MoveBeforeSystemFile(refNum);
			result = MoveToEndOfResourceChain(refNum);
			if (result == FALSE) {
				CloseResFile(refNum);
				refNum == -1;
			} else {
				*closeIt = TRUE;
			}
		}
	}
	return refNum;
}


//--------------------------------------------------------------------------------
short	UseMyResFile()
{
	short oldResFile = CurResFile();
	UseResFile(0);
	return oldResFile;
}


//--------------------------------------------------------------------------------
Handle	MyGetResource(ResType type, short resID)
{
	Handle	hndl = NIL;
	short	oldResFile = CurResFile();
	if (gRefNum > 0) {
		UseResFile(gRefNum);
		hndl = GetResource(type, resID);
		UseResFile(oldResFile);
	};

	return hndl;
}

//--------------------------------------------------------------------------------
/*
	See if the resource fork of the given file is open. This is done by
	walking the resource map list, getting the refNum for each resource map,
	and getting the entry in the FCB list for that resource fork. If that
	entry matches the one passed into this procedure, we return the refNum. If
	not, we go to the next resource map. If we come to the end of the resource
	map without finding a match. we return -1.

	Another approach would be to walk the entire FCB list, looking for an
	entry with a matching name, etc., and was for the resource fork. However,
	that won’t tell us the file is open within the currently running
	application, which is what we really want to know.
*/

short GetMyRefNum(FSSpecPtr file)
{
	OSErr			err;
	Str31			name;
	FCBPBRec		pb;
	ResMapHandle	current = (ResMapHandle) LMGetTopMapHndl();

	pb.ioNamePtr = name;
	pb.ioFCBIndx = 0;

	while (current != NIL) {

		pb.ioVRefNum = 0;
		pb.ioRefNum = (**current).refNum;

		err = PBGetFCBInfoSync(&pb);

		if ((err == noErr)
			&& (pb.ioFCBVRefNum == file->vRefNum)
			&& (pb.ioFCBParID == file->parID)
			&& (PLstrcmp(name, file->name) == 0)) {

				return pb.ioRefNum;
		}
		current = (**current).nextMap;
	}
	return -1;
}


//--------------------------------------------------------------------------------
/*
	Move the resource map with the given refNum just before the system
	resource map.
*/
#if 0
Boolean MoveBeforeSystemFile(short refNum)
{
	Boolean			result;
	ResMapHandle	current = NIL;
	ResMapHandle	previous = NIL;
	ResMapHandle	ourParent = NIL;
	ResMapHandle	us = NIL;
	ResMapHandle	systemParent = NIL;
	ResMapHandle	system = NIL;

	//
	// Try to find us in the chain
	//
	current = (ResMapHandle) TopMapHndl;
	while ((current != NIL) && ((**current).refNum != refNum)) {
		previous = current;
		current = (**current).nextMap;
	}

	//
	// If we found ourselves, remember our handle and our parent’s handle
	//
	if (current != NIL) {
		ourParent = previous;
		us = current;
	}

	//
	// Look for the system resource file
	//
	while ((current != NIL) && ((**current).refNum != 2)) {
		previous = current;
		current = (**current).nextMap;
	}

	//
	// If we found it, remember its handle and its parent’s handle
	//
	if (current != NIL) {
		systemParent = previous;
		system = current;
	}

	//
	// If either search failed, return failure
	//
	if ((us == NIL) || (system == NIL)) {
		result = FALSE;
	} else {

		//
		// Now that we have both our handle and the system’s, we want
		// to insert ourself right before the system (which will probably
		// put us just right behind the application. First check to see
		// that we aren’t already just in front of the system.
		//
		if (systemParent != us) {

			//
			// If we aren’t just before the system already, prepare to
			// “make it so.” First, remove our resource map handle from
			// the resource chain by telling our parent to point to the
			// same guy we point to. Also update TopMapHndl if it happens
			// to point to us by pointing it to our child.
			//
			if (ourParent)
				(**ourParent).nextMap = (**us).nextMap;
			if ((ResMapHandle) TopMapHndl == us)
				TopMapHndl = (void*) (**us).nextMap;

			//
			// Finally, stick us in where we want to be, which is
			// between the system resource file and its parent. It’s
			// important that we do this carefully; we don’t ever want
			// the resource chain to be inconsistant or circular. Doing
			// so may mess up debuggers like TMON, which check the
			// resource chain when we’re single stepping. Therefore,
			// we first point our child link to point to the system
			// file. Only then so we make the last step of making the
			// system file’s parent point to us.
			//
			(**us).nextMap = system;
			(**systemParent).nextMap = us;
		}
		result = TRUE;
	}

	return result;
}
#endif


//--------------------------------------------------------------------------------
/*
	Move the resource map with the given refNum just before the system
	resource map.
*/

Boolean MoveToEndOfResourceChain(short refNum)
{
	Boolean			result = FALSE;
	ResMapHandle	current = NIL;
	ResMapHandle	previous = NIL;
	ResMapHandle	ourParent = NIL;
	ResMapHandle	us = NIL;
	ResMapHandle	targetParent = NIL;

	//
	// Try to find us in the chain
	//
	current = (ResMapHandle) LMGetTopMapHndl();
	while ((current != NIL) && ((**current).refNum != refNum))
	{
		previous = current;
		current = (**current).nextMap;
	}

	//
	// If we found ourselves, remember our handle and our parent’s handle
	//
	if (current != NIL)
	{
		ourParent = previous;
		us = current;

		//
		// Look for the end of the chain
		//
		while (current != NIL)
		{
			targetParent = current;
			current = (**current).nextMap;
		}

		//
		// First, remove our resource map handle from
		// the resource chain by telling our parent to point to the
		// same guy we point to. Also update TopMapHndl if it happens
		// to point to us by pointing it to our child.
		//

		if (ourParent)
			(**ourParent).nextMap = (**us).nextMap;
		if ((ResMapHandle) LMGetTopMapHndl() == us)
			LMSetTopMapHndl((Handle) (**us).nextMap);

		//
		// Finally, stick us in where we want to be, which is
		// between the system resource file and its parent. It’s
		// important that we do this carefully; we don’t ever want
		// the resource chain to be inconsistant or circular. Doing
		// so may mess up debuggers like TMON, which check the
		// resource chain when we’re single stepping. Therefore,
		// we first point our child link to point to the system
		// file. Only then so we make the last step of making the
		// system file’s parent point to us.
		//

		(**us).nextMap = NIL;
		(**targetParent).nextMap = us;

		result = TRUE;
	}

	return result;
}

