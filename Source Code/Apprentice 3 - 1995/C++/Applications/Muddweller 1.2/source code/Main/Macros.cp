/* Macros - Implementation of the login/logout macros and key bindings        */

#include "Macros.h"

		// � Toolbox
#ifndef __CTYPE__
#include "CType.h"
#endif

#ifndef __ERRORS__
#include "Errors.h"
#endif

#ifndef __STDLIB__
#include "StdLib.h"
#endif

#ifndef __TOOLUTILS__
#include "ToolUtils.h"
#endif

		// � Implementation use
#ifndef __GLOBALS__
#include "Globals.h"
#endif

#ifndef __MUDDOC__
#include "MUDDoc.h"
#endif


//------------------------------------------------------------------------------

const short iLogin = 1;
const short iLogout = 2;
const short iSeparator = 3;
const short kMaxItems = 250;
const short kUpKeyMask = 0x80;
const short kMAsmKeyCache = 38; // is this ok (system 7.0 headers)?

short gNumKeyMacros;
Boolean gCmdInhibit;


//------------------------------------------------------------------------------

#pragma segment SDocDialogs

pascal void AppendStr (Str255 str, Str255 add)
{
	int len, ip, op;
	
	ip = str [0];
	op = 0;
	len = add [0];
	if (ip + len > 255) len = 255 - ip;
	while (len > 0) {
		ip += 1;
		op += 1;
		str [ip] = add [op];
		len -= 1;
	}
	str [0] = ip;
}


//------------------------------------------------------------------------------

#pragma segment SDocDialogs

pascal void AppendNum (Str255 str, unsigned char num)
{
	Str255 tmp;
	int i;
	
	i = 1;
	if (num >= 100) tmp [i++] = '0' + num / 100;
	if (num >= 10) tmp [i++] = '0' + (num % 100) / 10;
	tmp [i++] = '0' + num % 10;
	tmp [0] = i - 1;
	AppendStr (str, tmp);
}


//------------------------------------------------------------------------------

#pragma segment SDocDialogs

pascal void GetKeyName (MHandle mh, Str255 keyName)
{
	Str255 tmp;
	
	keyName [0] = 1;
	keyName [1] = '[';
	AppendNum (keyName, (**mh).mCode);
	keyName [++keyName [0]] = ']';
	keyName [++keyName [0]] = ' ';
	if ((**mh).mShift) {
		GetIndString (tmp, kMUDStrings, kmsShift);
		AppendStr (keyName, tmp);
	}
	if ((**mh).mOption) {
		GetIndString (tmp, kMUDStrings, kmsOption);
		AppendStr (keyName, tmp);
	}
	if ((**mh).mControl) {
		GetIndString (tmp, kMUDStrings, kmsControl);
		AppendStr (keyName, tmp);
	}
	switch ((**mh).mCode) {
	case kClearVirtualCode:
		GetIndString (tmp, kMUDStrings, kmsClear);
		break;
	case kEscapeVirtualCode:
		GetIndString (tmp, kMUDStrings, kmsEscape);
		break;
	case kF1VirtualCode:
		GetIndString (tmp, kMUDStrings, kmsF1);
		break;
	case kF2VirtualCode:
		GetIndString (tmp, kMUDStrings, kmsF2);
		break;
	case kF3VirtualCode:
		GetIndString (tmp, kMUDStrings, kmsF3);
		break;
	case kF4VirtualCode:
		GetIndString (tmp, kMUDStrings, kmsF4);
		break;
	case kF5VirtualCode:
		GetIndString (tmp, kMUDStrings, kmsF5);
		break;
	case kF6VirtualCode:
		GetIndString (tmp, kMUDStrings, kmsF6);
		break;
	case kF7VirtualCode:
		GetIndString (tmp, kMUDStrings, kmsF7);
		break;
	case kF8VirtualCode:
		GetIndString (tmp, kMUDStrings, kmsF8);
		break;
	case kF9VirtualCode:
		GetIndString (tmp, kMUDStrings, kmsF9);
		break;
	case kF10VirtualCode:
		GetIndString (tmp, kMUDStrings, kmsF10);
		break;
	case kF11VirtualCode:
		GetIndString (tmp, kMUDStrings, kmsF11);
		break;
	case kF12VirtualCode:
		GetIndString (tmp, kMUDStrings, kmsF12);
		break;
	case kF13VirtualCode:
		GetIndString (tmp, kMUDStrings, kmsF13);
		break;
	case kF14VirtualCode:
		GetIndString (tmp, kMUDStrings, kmsF14);
		break;
	case kF15VirtualCode:
		GetIndString (tmp, kMUDStrings, kmsF15);
		break;
	case kFwdDelVirtualCode:
		GetIndString (tmp, kMUDStrings, kmsFwdDel);
		break;
	default:
		if ((**mh).mChar <= 32)
			GetIndString (tmp, kKeyStrings, (**mh).mChar + 1);
		else {
			tmp [0] = 1;
			tmp [1] = (**mh).mChar;
		}
		break;
	}
	AppendStr (keyName, tmp);
}


//------------------------------------------------------------------------------

#pragma segment SDocDialogs

class TMacroTEView: public TTEView {
public:

	virtual pascal void ComputeSize (VPoint *newSize);
};

pascal void TMacroTEView::ComputeSize (VPoint *newSize)
{
	inherited::ComputeSize (newSize);
	if (newSize->v < fSuperView->fSize.v) newSize->v = fSuperView->fSize.v;
}


//------------------------------------------------------------------------------

#pragma segment SDocDialogs

Boolean gShift;
Boolean gOption;
Boolean gControl;
unsigned char gChar;
unsigned char gCode;

typedef struct MAExpandMemRec *MAExpandMemRecPtr, **MAExpandMemRecHandle;

struct MAExpandMemRec {
	short emVersion;
	long emSize;
	long emIntlGlobals;
	long emKeyDeadState;
	Ptr emKeyCache;
	long emIntlDef;
	Boolean emFirstKeyboard;
	Boolean emAlign;
	long emItlCache [4];
	Boolean emItlNeedUnlock;
	Boolean emItlDirectGetIntl;
	unsigned char emFiller [22];
};

class TNewDialogView: public TDialogView {
public:

	virtual pascal TCommand *DoKeyCommand (short ch, short aKeyCode,
		EventInfo *info);
};

pascal TCommand *TNewDialogView::DoKeyCommand (short ch, short aKeyCode,
		EventInfo *info)
{
	short keyCode;
	long state, keyInfo;
	Ptr keyTransTable;
	
	gChar = (unsigned char) ch;
	gCode = (unsigned char) aKeyCode;
	gShift = info->theShiftKey;
	gOption = info->theOptionKey;
	gControl = info->theControlKey;
	if (gConfiguration.hasScriptManager) {
		keyCode = gCode + kUpKeyMask; // all modifiers stripped, upKey set
		state = 0;
		if (gConfiguration.systemVersion >= 0x700)
			keyTransTable = (Ptr) GetEnvirons (kMAsmKeyCache);
		else
			keyTransTable = (Ptr)(**(MAExpandMemRecHandle)ExpandMem).emKeyCache;
		keyInfo = KeyTrans (keyTransTable, keyCode, &state);
		gChar = (unsigned char) keyInfo;
		if (gChar == 0) gChar = (unsigned char) (keyInfo >> 16);
	}
	fDismissed = TRUE;
	fDismisser = 'OK  ';
	return NULL;
}


//------------------------------------------------------------------------------

#pragma segment SDocDialogs

class TNewMacButton: public TButton {
public:
	TMUDDoc *fMUDDoc;

	virtual pascal void DoChoice (TView *origView, short itsChoice);
};

pascal void TNewMacButton::DoChoice (TView *, short)
{
	TWindow *aWindow;
	TDialogView *aDialogView;
	TPopup *aPopup;
	Boolean cancelled;
	MHandle mh, oh, nh;
	short item;
	int mval, val;
	Str255 keyName;
	
	aWindow = NewTemplateWindow (kTypeKeyID, NULL);
	FailNIL (aWindow);
	aDialogView = (TDialogView *) aWindow->FindSubView ('DLOG');
	cancelled = (aDialogView->PoseModally () == 'cncl');
	aWindow->Close ();
	if (cancelled) Failure (noErr, msgCancelled);
	mh = NewMacroRec ();
	(**mh).mChar = gChar;
	(**mh).mCode = gCode;
	(**mh).mShift = gShift;
	(**mh).mOption = gOption;
	(**mh).mControl = gControl;
	item = iSeparator;
	nh = fMUDDoc->fMacroList;
	oh = nh;
	mval = (gCode << 3) + (gShift << 2) + (gOption << 1) + gControl;
	while (nh) {
		val = ((**nh).mCode << 3) + ((**nh).mShift << 2) +
			((**nh).mOption << 1) + (**nh).mControl;
		if (val >= mval) break;
		item += 1;
		oh = nh;
		nh = (**nh).mNext;
	}
	aPopup = (TPopup *) GetWindow ()->FindSubView ('popu');
	if (oh && (val == mval))
		DisposMacroRec (mh);
	else {
		if (nh == fMUDDoc->fMacroList) {
			(**mh).mNext = nh;
			fMUDDoc->fMacroList = mh;
		} else {
			(**mh).mNext = (**oh).mNext;
			(**oh).mNext = mh;
		}
		GetKeyName (mh, keyName);
		SetItemMark (aPopup->fMenuHandle, aPopup->fCurrentItem, ' ');
		InsMenuItem (aPopup->fMenuHandle, keyName, item);
		SetItem (aPopup->fMenuHandle, item + 1, keyName);
		aPopup->fCurrentItem = 0;
		aPopup->AdjustBotRight ();
		gNumKeyMacros += 1;
		if (gNumKeyMacros >= kMaxItems) DimState (TRUE, kRedraw);
	}
	aPopup->SetCurrentItem (item + 1, kRedraw);
	aPopup->DoChoice (aPopup, aPopup->fDefChoice);
}


//------------------------------------------------------------------------------

#pragma segment SDocDialogs

class TRemoveButton: public TButton {
public:
	TMUDDoc *fMUDDoc;

	virtual pascal void DoChoice (TView *origView, short itsChoice);
};

pascal void TRemoveButton::DoChoice (TView *, short)
{
	TPopup *aPopup;
	TNewMacButton *aNewMacButton;
	short item, i;
	MHandle mh, oh;
	
	aPopup = (TPopup *) GetWindow ()->FindSubView ('popu');
	item = aPopup->fCurrentItem;
	mh = fMUDDoc->fMacroList;
	if (mh && (item > iSeparator)) {
		DelMenuItem (aPopup->fMenuHandle, item);
		aPopup->ForceRedraw ();
		aPopup->AdjustBotRight ();
		i = iSeparator + 1;
		if (item == i)
			fMUDDoc->fMacroList = (**mh).mNext;
		else {
			while (mh && (i < item)) {
				oh = mh;
				mh = (**mh).mNext;
				i += 1;
			}
			(**oh).mNext = (**mh).mNext;
		}
		DisposMacroRec (mh);
		if (item >= iSeparator + gNumKeyMacros) item -= 1;
		if (item == iSeparator) item = iLogout;
		gNumKeyMacros -= 1;
		aNewMacButton = (TNewMacButton *) GetWindow ()->FindSubView ('new ');
		if (gNumKeyMacros < kMaxItems) aNewMacButton->DimState (FALSE, kRedraw);
		aPopup->fCurrentItem = 0;
		aPopup->SetCurrentItem (item, kDontRedraw);
		aPopup->DoChoice (aPopup, aPopup->fDefChoice);
	}
}


//------------------------------------------------------------------------------

#pragma segment SDocDialogs

class TMacroPopup: public TPopup {
public:
	TMUDDoc *fMUDDoc;

	virtual pascal void DoChoice (TView *origView, short itsChoice);
};

pascal void TMacroPopup::DoChoice (TView *origView, short itsChoice)
{
	MHandle mh;
	Handle th;
	TMacroTEView *aMacroTEView;
	TRemoveButton *aRemoveButton;
	int i;
	
	if (fCurrentItem == iLogin)
		mh = fMUDDoc->fLogin;
	else if (fCurrentItem == iLogout)
		mh = fMUDDoc->fLogout;
	else {
		i = iSeparator + 1;
		mh = fMUDDoc->fMacroList;
		while (mh && (fCurrentItem > i)) {
			mh = (**mh).mNext;
			i += 1;
		}
	}
	if (mh) {
		th = (**mh).mText;
		aMacroTEView = (TMacroTEView *) GetWindow ()->FindSubView ('text');
		if (aMacroTEView->fText != th) {
			(**aMacroTEView->fHTE).hText = th;
			(**aMacroTEView->fHTE).teLength = (short) GetHandleSize (th);
			aMacroTEView->fText = th;
			aMacroTEView->RecalcText ();
			aMacroTEView->ForceRedraw ();
			TESetSelect(0, 0, aMacroTEView->fHTE);
		}
	}
	aRemoveButton = (TRemoveButton *) GetWindow ()->FindSubView ('remv');
	aRemoveButton->DimState (fCurrentItem <= iSeparator, kRedraw);
	inherited::DoChoice (origView, itsChoice);
}


//------------------------------------------------------------------------------

#pragma segment SMacros

pascal void DisposMacroRec (MHandle theRec)
{
	if (theRec) {
		DisposIfHandle ((**theRec).mText);
		DisposIfHandle (theRec);
	}
}


//------------------------------------------------------------------------------

#pragma segment MAReadFile

pascal long MacroRecSize (MHandle theRec)
{
	return sizeof (MacroDiskRec) + GetHandleSize ((**theRec).mText);
}


//------------------------------------------------------------------------------

#pragma segment SMacros

pascal MHandle NewMacroRec (void)
{
	MHandle mh;
	
	mh = (MHandle) NewPermHandle (sizeof (MacroRec));
	FailNIL (mh);
	(**mh).mNext = NULL;
	(**mh).mText = NewPermHandle (0);
	if (!(**mh).mText) {
		DisposIfHandle (mh);
		Failure (memFullErr, 0);
	}
	(**mh).mShift = FALSE;
	(**mh).mOption = FALSE;
	(**mh).mControl = FALSE;
	(**mh).mChar = 0;
	(**mh).mCode = 0;
	return mh;
}


//------------------------------------------------------------------------------

#pragma segment SMacros

pascal MHandle ReadMacroRec (short theRefNum)
{
	MHandle mh;
	Handle th;
	MacroDiskRec diskRec;
	OSErr err;
	long count;
	
	mh = (MHandle) NewPermHandle (sizeof (MacroRec));
	FailNIL (mh);
	count = sizeof (diskRec);
	err = FSRead (theRefNum, &count, (Ptr) &diskRec);
	if (err != noErr) {
		DisposIfHandle (mh);
		FailOSErr (err);
	}
	(**mh).mNext = NULL;
	(**mh).mShift = diskRec.shift;
	(**mh).mOption = diskRec.option;
	(**mh).mControl = diskRec.control;
	(**mh).mChar = diskRec.ch;
	(**mh).mCode = diskRec.code;
	th = NewPermHandle (diskRec.textLen);
	if (!th) {
		DisposIfHandle (mh);
		Failure (memFullErr, 0);
	}
	HLock (th);
	count = diskRec.textLen;
	err = FSRead (theRefNum, &count, *th);
	HUnlock (th);
	if (err != noErr) {
		DisposMacroRec (mh);
		FailOSErr (err);
	}
	(**mh).mText = th;
	return mh;
}


//------------------------------------------------------------------------------

#pragma segment SMacros

pascal void WriteMacroRec (short theRefNum, MHandle theRec)
{
	MacroDiskRec diskRec;
	long count;
	SignedByte oldState;
	
	diskRec.shift = (**theRec).mShift;
	diskRec.option = (**theRec).mOption;
	diskRec.control = (**theRec).mControl;
	diskRec.filler1 = 0;
	diskRec.ch = (**theRec).mChar;
	diskRec.code = (**theRec).mCode;
	diskRec.textLen = (short) GetHandleSize ((**theRec).mText);
	count = sizeof (diskRec);
	FailOSErr (FSWrite (theRefNum, &count, (Ptr) &diskRec));
	oldState = HGetState ((**theRec).mText);
	HLock ((**theRec).mText);
	count = diskRec.textLen;
	FailOSErr (FSWrite (theRefNum, &count, *(**theRec).mText));
	HSetState ((**theRec).mText, oldState);
}


//------------------------------------------------------------------------------

#pragma segment SDocDialogs

pascal void SetupDialog (TMUDDoc *aMUDDoc, TWindow *aWindow)
{
	TMacroTEView *aMacroTEView;
	TMacroPopup *aMacroPopup;
	TNewMacButton *aNewMacButton;
	TRemoveButton *aRemoveButton;
	Handle h;
	MHandle mh;
	Str255 keyName;
	short item;
	
	aMacroPopup = (TMacroPopup *) aWindow->FindSubView ('popu');
	aMacroPopup->fMUDDoc = aMUDDoc;
	gNumKeyMacros = 0;
	item = aMUDDoc->fLastMacItem;
	if (item == iLogin)
		h = (**(aMUDDoc->fLogin)).mText;
	else if (item == iLogout)
		h = (**(aMUDDoc->fLogout)).mText;
	else
		h = NULL;
	item -= iSeparator;
	mh = aMUDDoc->fMacroList;
	while (mh) {
		GetKeyName (mh, keyName);
		InsMenuItem (aMacroPopup->fMenuHandle, keyName, iSeparator +
			gNumKeyMacros);
		SetItem (aMacroPopup->fMenuHandle, iSeparator + gNumKeyMacros + 1,
			keyName);
		gNumKeyMacros += 1;
		if (--item == 0) h = (**mh).mText;
		mh = (**mh).mNext;
	}
	aMacroPopup->AdjustBotRight ();
	aMacroPopup->SetCurrentItem (aMUDDoc->fLastMacItem, kDontRedraw);
	aNewMacButton = (TNewMacButton *) aWindow->FindSubView ('new ');
	aNewMacButton->fMUDDoc = aMUDDoc;
	aNewMacButton->DimState (gNumKeyMacros >= kMaxItems, kDontRedraw);
	aRemoveButton = (TRemoveButton *) aWindow->FindSubView ('remv');
	aRemoveButton->fMUDDoc = aMUDDoc;
	aRemoveButton->DimState (aMUDDoc->fLastMacItem < iSeparator, kDontRedraw);
	aMacroTEView = (TMacroTEView *) aWindow->FindSubView ('text');
	if (h) aMacroTEView->StuffText (h);
	aMacroTEView->RecalcText ();
}


//------------------------------------------------------------------------------

#pragma segment SDocDialogs

pascal void EditMacros (TDocument *aDocument)
{
	TWindow *aWindow;
	TDialogView *aDialogView;
	TMacroPopup *aMacroPopup;
	
	aWindow = NewTemplateWindow (kMacrosID, NULL);
	FailNIL (aWindow);
	SetupDialog ((TMUDDoc *) aDocument, aWindow);
	aDialogView = (TDialogView *) aWindow->FindSubView ('DLOG');
	aDialogView->PoseModally ();
	aMacroPopup = (TMacroPopup *) aWindow->FindSubView ('popu');
	((TMUDDoc *) aDocument)->fLastMacItem = aMacroPopup->GetCurrentItem ();
	aWindow->Close ();
	aDocument->fChangeCount += 1;
}


//------------------------------------------------------------------------------

#pragma segment MAInit

pascal void InitMacros (void)
{
	gCmdInhibit = FALSE;
	if (gDeadStripSuppression) {
		TMacroTEView *aMacroTEView = new TMacroTEView;
		TMacroPopup *aMacroPopup = new TMacroPopup;
		TNewMacButton *aNewMacButton = new TNewMacButton;
		TRemoveButton *aRemoveButton = new TRemoveButton;
		TNewDialogView *aNewDialogView = new TNewDialogView;
	}
}


//------------------------------------------------------------------------------

#pragma segment SMacros

pascal long GetCmdStr (Handle theText, long pos, Str255 cmd, Str255 parm)
{
	long size;
	unsigned char **th, ch;
	int i;
	
	size = GetHandleSize (theText);
	th = (unsigned char **) theText;
	ch = *(*th + pos);
	while ((pos < size) && ((ch == '#') || isspace (ch))) {
		if ((pos < size) && (ch == '#')) {
			ch = *(*th + ++pos);
			while ((pos < size) && (ch != '\n')) ch = *(*th + ++pos);
		}
		while ((pos < size) && isspace (ch)) ch = *(*th + ++pos);
	}
	ch = toupper (ch);
	i = 0;
	while ((pos < size) && (i < 255) && !isspace (ch)) {
		cmd [++i] = ch;
		ch = toupper (*(*th + ++pos));
	}
	cmd [0] = (unsigned char) i;
	while ((pos < size) && ((ch == ' ') || (ch == '\t'))) ch = *(*th + ++pos);
	i = 0;
	while ((pos < size) && (i < 255) && (ch != '\n')) {
		parm [++i] = ch;
		ch = *(*th + ++pos);
	}
	parm [0] = (unsigned char) i;
	while ((pos < size) && (ch != '\n')) ch = *(*th + ++pos);
	return pos;
}


//------------------------------------------------------------------------------

#pragma segment SMacros

pascal void ParseCmdHandler (short , long , void * )
{
	gCmdInhibit = FALSE;
}

pascal long ParseCmd (TMUDDoc *doc, Handle theText, long pos, int *cmd,
		Str255 parm)
{
	Str255 cmdstr, parmstr, tmp;
	int i, j, k, l, len;
	FailInfo fi;
	unsigned char ch, *cp, c;
	
	pos = GetCmdStr (theText, pos, cmdstr, parmstr);
	if (cmdstr [0] == 0) {
		*cmd = kCmdNone;
		parm [0] = 0;
		return pos;
	}
	for (i = 1; i <= kLastCmd; i++) {
		GetIndString (tmp, kCmdStrings, (short) i);
		if (CompareStrings (cmdstr, tmp) == 0) break;
	}
	if (i > kLastCmd) {
		CatchFailures (&fi, ParseCmdHandler, kDummyLink);
		gCmdInhibit = TRUE;
		ParamText (cmdstr, "\p", "\p", "\p");
		MacAppAlert (phCmdErrID, NULL);
		gCmdInhibit = FALSE;
		Success (&fi);
		*cmd = kCmdNone;
		parm [0] = 0;
		return GetHandleSize (theText);
	}
	*cmd = i;
	len = parmstr [0];
	i = 0;
	j = 0;
	while ((i < 255) && (j < len)) {
		ch = parmstr [++j];
		if (ch == '$') {
			ch = parmstr [++j];
			l = 0;
			if (j <= len) {
				switch (ch) {
				case 'n':
				case 'N':
					cp = doc->fPlayer;
					l = *cp++;
					break;
				case 'p':
				case 'P':
					cp = doc->fPasswd;
					l = *cp++;
					break;
				case '$':
					parm [++i] = ch;
					break;
				}
			}
			if (l > 0) {
				k = 0;
				while ((i < 255) && (k < l)) {
					parm [++i] = *cp++;
					k += 1;
				}
			}
		} else if (ch == '\\') {
			ch = parmstr [++j];
			if (j <= len) {
				switch (ch) {
				case 'b':
					ch = '\b';
					break;
				case 't':
					ch = '\t';
					break;
				case 'n':
					ch = '\n';
					break;
				case 'f':
					ch = '\f';
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
					c = ch - '0';
					ch = parmstr [++j];
					if ((j <= len) && (ch >= '0') && (ch <= '7'))
						c = c * 8 + ch - '0';
					else
						j -= 1;
					ch = parmstr [++j];
					if ((j <= len) && (ch >= '0') && (ch <= '7'))
						c = c * 8 + ch - '0';
					else
						j -= 1;
					ch = c;
					break;
				}
				parm [++i] = ch;
			}
		} else
			parm [++i] = ch;
	}
	parm [0] = (unsigned char) i;
	return pos;
}


//------------------------------------------------------------------------------

#pragma segment SMacros

pascal void TMacro::DoEcho (void)
{
	Str255 tmp;
	long act, count;
	
	CopyStr255 (fParm, tmp);
	act = 1;
	count = tmp [0] + 1;
	while (act < count)
		act += ((TMUDDoc *) fDocument)->fEchoFilter->Filter (tmp + act,
			count - act);
}


//------------------------------------------------------------------------------

#pragma segment SMacros

pascal void TMacro::DoIdle (void)
{
	int cmd;
	Str255 parm, tmp;
	long size;
	
	if (gCmdInhibit) return;
	if (!((TMUDDoc *) fDocument)->fConnected || gStop) {
		((TMUDDoc *) fDocument)->fMacroFilter->fScan = FALSE;
		MacroDone ();
	} else {
		size = GetHandleSize ((**fMRec).mText);
		do {
			if (fCmdDone && (fMPos < size)) {
				fMPos = ParseCmd ((TMUDDoc *) fDocument, (**fMRec).mText,
					fMPos, &cmd, parm);
				fCmd = cmd;
				CopyStr255 (parm, fParm);
			}
			switch (fCmd) {
			case kCmdSend:
				DoSend ();
				break;
			case kCmdMatch:
				DoMatch ();
				break;
			case kCmdWait:
				DoWait ();
				break;
			case kCmdPasswd:
				DoPasswd ();
				break;
			case kCmdQuiet:
				DoQuiet ();
				break;
			case kCmdEcho:
				DoEcho ();
				break;
			}
		} while (fCmdDone && (fMPos < size));
		if (fCmdDone && (fMPos >= size)) MacroDone ();
	}
}


//------------------------------------------------------------------------------

#pragma segment SMacros

pascal void TMacro::DoMatch (void)
{
	TMacroFilter *filter;
	
	if (fCmdDone && (fParm [0] > 0)) {
		filter = ((TMUDDoc *) fDocument)->fMacroFilter;
		CopyStr255 (fParm, filter->fStr);
		filter->fPos = 1;
		filter->fScan = TRUE;
		fCmdDone = FALSE;
	}
}


//------------------------------------------------------------------------------

#pragma segment SMacros

pascal void TMacro::DoPasswd (void)
{
	TWindow *aWindow;
	TDialogView *aDialogView;
	TEditText *aEditText;
	Str255 tmp;
	FailInfo fi;
	
	aWindow = NewTemplateWindow (kPasswordID, NULL);
	FailNIL (aWindow);
	if (fParm [0] > 0) {
		aEditText = (TEditText *) aWindow->FindSubView ('text');
		CopyStr255 (fParm, tmp);
		aEditText->SetText (tmp, kDontRedraw);
	}
	CatchFailures (&fi, ParseCmdHandler, kDummyLink);
	gCmdInhibit = TRUE;
	aDialogView = (TDialogView *) aWindow->FindSubView ('DLOG');
	aDialogView->PoseModally ();
	aEditText = (TEditText *) aWindow->FindSubView ('pass');
	aEditText->GetText (tmp);
	aWindow->Close ();
	gCmdInhibit = FALSE;
	Success (&fi);
	if (tmp [0] < 255) tmp [++tmp [0]] = chReturn;
	((TMUDDoc *) fDocument)->Send (tmp + 1, tmp [0]);
}


//------------------------------------------------------------------------------

#pragma segment SMacros

pascal void TMacro::DoQuiet (void)
{
	Str255 tmp, off;
	int i, len;
	
	CopyStr255 (fParm, tmp);
	len = tmp [0];
	i = 0;
	while ((i < len) && !isspace (tmp [i + 1])) i += 1;
	tmp [0] = i;
	for (i = 1; i <= len; i++) tmp [i] = toupper (tmp [i]);
	GetIndString (off, kMUDStrings, kmsOff);
	if (CompareStrings (tmp, off) == 0)
		((TMUDDoc *) fDocument)->fQuiet = FALSE;
	else
		((TMUDDoc *) fDocument)->fQuiet = TRUE;
}


//------------------------------------------------------------------------------

#pragma segment SMacros

pascal void TMacro::DoSend (void)
{
	Str255 tmp;
	
	CopyStr255 (fParm, tmp);
	((TMUDDoc *) fDocument)->Send (tmp + 1, tmp [0]);
}


//------------------------------------------------------------------------------

#pragma segment SMacros

pascal void TMacro::DoWait (void)
{
	Str255 tmp;
	int i;
	
	if (fCmdDone) {
		CopyStr255 (fParm, tmp);
		if (tmp [0] == 0)
			i = 0;
		else {
			if (tmp [0] < 255)
				tmp [tmp [0] + 1] = 0;
			else
				tmp [255] = 0;
			i = atoi ((char *) (tmp + 1));
		}
		fWaitEnd = TickCount () + 60 * i;
		fCmdDone = FALSE;
	} else if (TickCount () >= fWaitEnd)
		fCmdDone = TRUE;
}


//------------------------------------------------------------------------------

#pragma segment SMacros

pascal void TMacro::Free (void)
{
	TMacro *aMacro;
	
	if (((TMUDDoc *)fDocument)->fRunList == this)
		((TMUDDoc *)fDocument)->fRunList = fNext;
	else {
		aMacro = ((TMUDDoc *)fDocument)->fRunList;
		while (aMacro && (aMacro->fNext != this)) aMacro = aMacro->fNext;
		if (aMacro) aMacro->fNext = fNext;
	}
	((TMUDDoc *)fDocument)->fRunning =
		(((TMUDDoc *)fDocument)->fRunList != NULL);
	((TMUDDoc *) fDocument)->fQuiet = FALSE;
	InvalidateMenuBar ();
	gApplication->SetupTheMenus ();
	inherited::Free ();
}


//------------------------------------------------------------------------------

#pragma segment SMacros

pascal void TMacro::IMacro (TDocument *itsDoc, MHandle itsRec)
{
	TMacro *aMacro;
	
	fNext = NULL;
	IObject ();
	fDocument = itsDoc;
	fMRec = itsRec;
	fMPos = 0;
	fCmd = kCmdNone;
	fParm [0] = 0;
	fCmdDone = TRUE;
	if (((TMUDDoc *)itsDoc)->fRunList) {
		aMacro = ((TMUDDoc *)itsDoc)->fRunList;
		while (aMacro->fNext) aMacro = aMacro->fNext;
		aMacro->fNext = this;
	} else
		((TMUDDoc *)itsDoc)->fRunList = this;
	((TMUDDoc *)fDocument)->fRunning = TRUE;
	gBusyState = 0;
	gLastBusyTick = 0;
	gApplication->InvalidateCursorRgn ();
	InvalidateMenuBar ();
	gApplication->SetupTheMenus ();
}


//------------------------------------------------------------------------------

#pragma segment SMacros

pascal void TMacro::MacroDone (void)
{
	Free ();
}


//------------------------------------------------------------------------------

#pragma segment SMacros

pascal void TLoginMacro::DoIdle (void)
{
	OSErr err;
	IOBlock *block;
	
	if (!fOpenDone) {
		block = ((TMUDDoc *)fDocument)->fBlock;
		if (gStop) {
			PBKillIO ((ParamBlockRec *) &block->pb, FALSE);
			block->pb.ioNamePtr = NULL;
			block->pb.ioCRefNum = gTCPRef;
			block->pb.tcpStream = ((TMUDDoc *)fDocument)->fTCPStream;
			block->pb.csCode = TCPAbort;
			PBControl ((ParamBlockRec *) &block->pb, FALSE);
			((TMUDDoc *)fDocument)->fConnected = FALSE;
			Free ();
			return;
		} else if ((block->pb.ioResult != 1) || (block->ioError != noErr)) {
			if (block->ioError != noErr)
				err = block->ioError;
			else
				err = block->pb.ioResult;
			if (err != noErr) {
				if (TickCount () - fStartTicks + 30 > kTCPTimeout * 60)
					err = errOpenTimeout;
				Free ();
				FailOSErr (err);
			}
			fOpenDone = TRUE;
			((TMUDDoc *)fDocument)->fConnected = TRUE;
			((TMUDDoc *)fDocument)->fDoConnect = TRUE;
		}
	}
	if (fOpenDone) inherited::DoIdle ();
}


//------------------------------------------------------------------------------

#pragma segment SMacros

pascal void TLoginMacro::IMacro (TDocument *itsDoc, MHandle itsRec)
{
	inherited::IMacro (itsDoc, itsRec);
	fOpenDone = ((TMUDDoc *)itsDoc)->fUseCTB;
}


//------------------------------------------------------------------------------

#pragma segment SMacros

pascal void TLogoutMacro::DoIdle (void)
{
	TCloseWindowCommand *aCloseWindowCommand;
	TQuitCommand *aQuitCommand;
	IOBlock *block;
	
	if (fMacroDone || gStop) {
		block = ((TMUDDoc *)fDocument)->fBlock;
		if (!fClosing && !gStop) {
			block->pb.ioCompletion = NULL;
			block->pb.ioNamePtr = NULL;
			block->pb.ioCRefNum = gTCPRef;
			block->pb.tcpStream = ((TMUDDoc *)fDocument)->fTCPStream;
			block->pb.csCode = TCPClose;
			block->pb.csParam.open.ulpTimeoutValue = kTCPTimeout;
			block->pb.csParam.open.ulpTimeoutAction = 1;
			block->pb.csParam.open.validityFlags = timeoutValue + timeoutAction;
			block->pb.csParam.open.commandTimeoutValue = kTCPTimeout;
			block->pb.csParam.open.userDataPtr =
				(Ptr) ((TMUDDoc *)fDocument)->fBlock;
			PBControl ((ParamBlockRec *) &block->pb, TRUE);
			fClosing = TRUE;
		} else if (gStop) {
			if (fClosing) PBKillIO ((ParamBlockRec *) &block->pb, FALSE);
			block->pb.ioResult = 0;
		}
		if ((block->pb.ioResult != 1) || (block->ioError != noErr))
			((TMUDDoc *)fDocument)->Disconnect ();
		if (!((TMUDDoc *)fDocument)->fConnected) {
			if (fDoneCmd == cClose) {
				aCloseWindowCommand = new TCloseWindowCommand;
				if (aCloseWindowCommand) { // don't fail if NIL
					aCloseWindowCommand->ICloseWindowCommand (cClose,
						((TMUDDoc *) fDocument)->fLogWindow);
					gApplication->PostCommand (aCloseWindowCommand);
				}
			} else if (fDoneCmd == cQuit) {
				aQuitCommand = new TQuitCommand;
				if (aQuitCommand) { // don't fail if NIL
					aQuitCommand->IQuitCommand (cQuit);
					gApplication->PostCommand (aQuitCommand);
				}
			}
			Free ();
		}
	} else
		inherited::DoIdle ();
}


//------------------------------------------------------------------------------

#pragma segment SMacros

pascal void TLogoutMacro::Free (void)
{
	((TMUDDoc *)fDocument)->fClosing = FALSE;
	inherited::Free ();
}


//------------------------------------------------------------------------------

#pragma segment SMacros

pascal void TLogoutMacro::IMacro (TDocument *itsDoc, MHandle itsRec)
{
	inherited::IMacro (itsDoc, itsRec);
	fMacroDone = FALSE;
	fClosing = FALSE;
	fDoneCmd = cNoCommand;
	((TMUDDoc *)itsDoc)->fClosing = TRUE;
}


//------------------------------------------------------------------------------

#pragma segment SMacros

pascal void TLogoutMacro::MacroDone (void)
{
	fMacroDone = TRUE;
}


//------------------------------------------------------------------------------
