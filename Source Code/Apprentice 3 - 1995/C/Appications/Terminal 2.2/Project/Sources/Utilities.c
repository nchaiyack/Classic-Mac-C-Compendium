/*
	Terminal 2.2
	"Utilities.c"
*/

#ifdef THINK_C
#include "MacHeaders"
#endif
#ifdef applec
#pragma load ":(Objects):MacHeadersMPW"
#pragma segment Main
#endif

#include "Utilities.h"
#include "Text.h"
#include "Main.h"
#include "Document.h"

/* ----- Calculate number of bytes to make multiple of block size ------ */

short Filler(register short blocksize, register long n)
{
	register short i;

	return (i = n % blocksize) ? blocksize - i : 0;
}

/* ----- Get pointer to string ----------------------------------------- */

Byte *MyString(short id, register short number)
{
	register Handle h;
	register Byte *p;
	register short i;
	register short n;

	h = GetResource('STR#', id);
	p = (Byte *)*h;
	n = *(short *)p;
	p += 2;
	for (i = 1; i <= n; i++, p += *p + 1)
		if (i == number)
			return p;
	return 0;
}

/* ----- Append string2 to string1 ------------------------------------- */

Byte *Append(register Byte *s1, register Byte *s2)
{
	register short d, n, i;

	n = *s1;
	d = *s2;
	i = n + d - 255;
	if (i > 0)
		d -= i;
	*s1 = n + d;
	memcpy(s1 + n + 1, s2 + 1, d);
	return s1;
}

#ifdef OUTLINE
/* ----- Outline button in dialog -------------------------------------- */

void OutLine(dialog, item, mode)
register DialogPtr dialog;
register short item;
register short mode;
{
	short kind;
	Handle h;
	Rect r;
	PenState state;

	SetPort(dialog);
	GetPenState(&state);
	GetDItem(dialog, item, &kind, &h, &r);
	PenMode(mode);
	PenPat(QD(black));
	PenSize(3, 3);
	InsetRect(&r, -4, -4);
	FrameRoundRect(&r, 16, 16);
	SetPenState(&state);
}
#endif

/* ----- Get edit text field in a dialog ------------------------------- */

void GetEText(
	register DialogPtr dPtr,
	register short item,
	register Byte *str)
{
	short theType;
	Handle theItem;
	Rect theBox;

	GetDItem(dPtr, item, &theType, &theItem, &theBox);
	GetIText(theItem, str);
}

/* ----- Set edit text field in a dialog ------------------------------- */

void SetEText(
	register DialogPtr dPtr,
	register short item,
	register Byte *str)
{
	short theType;
	Handle theItem;
	Rect theBox;

	GetDItem(dPtr, item, &theType, &theItem, &theBox);
	SetIText(theItem, str);
}

/* ----- Get the value of a control ------------------------------------ */

short GetCheck(
	register DialogPtr dPtr,
	register short ChkItem)
{
	short theType;
	Handle theItem;
	Rect theBox;

	GetDItem(dPtr, ChkItem, &theType, &theItem, &theBox);
	return GetCtlValue((ControlHandle)theItem);
}

/* ----- Set the value of a control ------------------------------------ */

void SetCheck(
	register DialogPtr dPtr,
	short ChkItem,
	short value)
{
	short theType;
	Handle theItem;
	Rect theBox;

	GetDItem(dPtr, ChkItem, &theType, &theItem, &theBox);
	SetCtlValue((ControlHandle)theItem, value);
}

/* ----- Hilite control ----------------------------------------------- */

void SetHilite(
	register DialogPtr dPtr,
	short ChkItem,
	short Value)
{
	short theType;
	Handle theItem;
	Rect theBox;

	GetDItem(dPtr, ChkItem, &theType, &theItem, &theBox);
	HiliteControl((ControlHandle)theItem, Value);
}

/* ----- Center dialog or alert template ------------------------------- */

void CenterDialog(
	register long templateType,	/* 'ALRT' or 'DLOG' */
	register short templateID)
{
	register Rect *p;
	register Rect **h;	/* Templates start with boundsRect */
	register short width, height;
	Rect r = QD(screenBits.bounds);
	
	if (!(h = (Rect **)GetResource(templateType, templateID)))
		return;
	r.top += MBarHeight;
	p = *h;
	width = p->right - p->left;
	height = p->bottom - p->top;
	p->top = r.top + ((r.bottom - r.top - height) / 3);
	p->bottom = p->top + height;
	p->left = r.left + ((r.right - r.left - width) / 2);
	p->right = p->left + width;
}

/* ----- Calculate where to put a centered dialog box ------------------ */

void GetDlogOrigin(
	register short dlogID,
	register Point *where)
{
	register Rect *p;
	register Rect **h;	/* Templates start with boundsRect */
	Rect r = QD(screenBits.bounds);

	if (!(h = (Rect **)GetResource('DLOG', dlogID))) {
		where->v = where->h = 80;
		return;
	}
	r.top += MBarHeight;
	p = *h;
	where->h = r.left + ((r.right - r.left - (p->right - p->left)) / 2);
	where->v = r.top + ((r.bottom - r.top - (p->bottom - p->top)) / 3);
}

/* ----- Toggle value of checkbox -------------------------------------- */

void ToggleCheckBox(
	register DialogPtr dialog,
	register short item)
{
	short type;
	Handle itemHdl;
	Rect box;

	GetDItem(dialog, item, &type, &itemHdl, &box);
	SetCtlValue((ControlHandle)itemHdl,
		(GetCtlValue((ControlHandle)itemHdl) == 0) ? 1 : 0);
}

/* ----- Set group of radio buttons ------------------------------------ */

Boolean SetRadioButton(
	register DialogPtr dialog,
	register short item1,
	register short item2,
	register short item)
{
	register short i;
	short type;
	Handle itemHdl;
	Rect box;

	if (item < item1 || item > item2)
		return FALSE;
	for (i = item1; i <= item2; i++) {
		GetDItem(dialog, i, &type, &itemHdl, &box);
		SetCtlValue((ControlHandle)itemHdl, (item == i) ? 1 : 0);
	}
	return TRUE;
}

/* ----- Get group of radio buttons ------------------------------------ */

short GetRadioButton(
	register DialogPtr dialog,
	register short item1,
	register short item2)
{
	register short i;
	short type;
	Handle itemHdl;
	Rect box;

	for (i = item1; i <= item2; i++) {
		GetDItem(dialog, i, &type, &itemHdl, &box);
		if (GetCtlValue((ControlHandle)itemHdl))
			return i;
	}
	return 0;
}

#ifdef OUTLINE
/* ----- Filter function for dialog ------------------------------------ */

/*
	Outline the "Ok" button (item #1) and convert "Return" and "Enter"
	keys to item #1.
*/

pascal Boolean OutlineFilter(dialog, event, item)
register DialogPtr dialog;
register EventRecord *event;
short *item;
{
	register unsigned char key;
	DialogPtr dp;
	short i;

	switch (event->what) {
		case updateEvt:
			/* Proper handling of update events to avoid continous calls
			to our filter function with update events (flickering):
			Other dialog boxes may appear on the screen and disappear,
			generating update events both for themselves and for other
			windows allready on the screen. They must be handled
			immediatly  and then cleared via BeginUpdate() / EndUpdate().
			Otherwise the system will keep reporting the same event over
			and over again. */
			if (!SystemEvent(event)) {	/* SystemEvent() does updating */
				if (IsDialogEvent(event)) {			/* Dialog Window? */
					DialogSelect(event, &dp, &i);	/* Update dialog */
					if (dp == dialog)				/* If our dialog */
						OutLine(dialog, 1, patCopy);/* "Ok" button */
				} else
					RedrawDocument();		/* Update application window */
			}
			break;
		case keyDown:
		case autoKey:
			key = event->message & charCodeMask;
			if (key == '\015' || key == 0x03) {	/* "Return", "Enter" */
				*item = 1;						/* Change to "Ok" button */
				return TRUE;					/* We changed the event */
			}
			break;
	}
	return FALSE;	/* ModalDialog() should handle event */
}
#endif

/* ----- Draw user item in dialog -------------------------------------- */

pascal void DrawUserFrame(
	register WindowPtr window,
	register short number)
{
	short type;
	Handle item;
	Rect box, boxT;

	GetDItem(window, number - 1, &type, &item, &boxT);	/* Text */
	GetDItem(window, number, &type, &item, &box);		/* User item */
	PenPat(QD(gray));
	MoveTo(boxT.right + 2, box.top);
	LineTo(box.right, box.top);
	LineTo(box.right, box.bottom);
	LineTo(box.left, box.bottom);
	LineTo(box.left, box.top);
	LineTo(boxT.left - 3, box.top);
	PenPat(QD(black));
}

/* ----- Draw user item in dialog -------------------------------------- */

pascal void DrawUserLine(
	register WindowPtr window,
	register short number)
{
	short type;
	Handle item;
	Rect box;

	GetDItem(window, number, &type, &item, &box);		/* User item */
	PenPat(QD(gray));
	MoveTo(box.left, box.top);
	LineTo(box.right, box.top);
	MoveTo(box.left, box.bottom);
	LineTo(box.right, box.bottom);
	PenPat(QD(black));
}

/* ----- Convert to full path name ------------------------------------- */

static short Path1name(
	register Byte *name,
	register long directory,
	register CInfoPBRec *cat)
{
	register short err;
	register Byte s[66];

	cat->dirInfo.ioNamePtr = (StringPtr)s;
	cat->dirInfo.ioDrDirID = directory;
	if (!(err = PBGetCatInfo(cat, FALSE))) {
		if (directory != 2)
			err = Path1name(name, cat->dirInfo.ioDrParID, cat);
		if (!err) {
			Append(s, (Byte *)"\p:");
			Append(name, s);
		}
	}
	return err;
}

short Pathname(
	register Byte *name,		/* Path name returned */
	register short volume,		/* Volume reference number */
	register long directory)	/* Directory ID */
{
	CInfoPBRec cat;

	memset(&cat, 0, sizeof(cat));
	cat.dirInfo.ioVRefNum = volume;
	cat.dirInfo.ioFDirIndex = -1;
	name[0] = 0;
	return Path1name(name, directory, &cat);
}

/* ----- Normalize volume name and volume reference number ------------- */

short VolumeId(
	register Byte *name,
	register short *volume)
{
	register short err;
	HParamBlockRec p;

	memset(&p, 0, sizeof(p));
	if (*volume) {	/* Get corresponding volume name */
		p.volumeParam.ioNamePtr = (StringPtr)name;
		p.volumeParam.ioVRefNum = *volume;
	} else {		/* Get corresponding volume reference number */
		register Byte s[32];
		memcpy(s, name, name[0] + 1);
		s[++(s[0])] = ':';
		p.volumeParam.ioNamePtr = (StringPtr)s;
		p.volumeParam.ioVolIndex = -1;
	}
	err = PBHGetVInfo(&p, FALSE);
	*volume = p.volumeParam.ioVRefNum;
	return err;
}

/* ----- Open file (data fork) ----------------------------------------- */

short OpenFile(
	register short volume,
	register long directory,
	register Byte *name,
	register short *ref)
{
	register short err;
	HFileParam p;

	memset(&p, 0, sizeof(p));
	p.ioVRefNum = volume;
	p.ioDirID = directory;
	p.ioNamePtr = (StringPtr)name;
	*ref = (err = PBHOpen((HParmBlkPtr)&p, FALSE)) ? 0 : p.ioFRefNum;
	return err;
}

/* ----- Open file (resource fork) ------------------------------------- */

short OpenResource(
	register short volume,
	register long directory,
	register Byte *name,
	register short *ref)
{
	register short err;
	HFileParam p;

	memset(&p, 0, sizeof(p));
	p.ioVRefNum = volume;
	p.ioDirID = directory;
	p.ioNamePtr = (StringPtr)StripAddress((Ptr)name);	/* TN #232 */
	*ref = (err = PBHOpenRF((HParmBlkPtr)&p, FALSE)) ? 0 : p.ioFRefNum;
	return err;
}

/* ----- Create file --------------------------------------------------- */

short CreateFile(
	register short volume,
	register long directory,
	register Byte *name,
	register long creator,
	register long type)
{
	register short err;
	HFileParam p;

	memset(&p, 0, sizeof(p));
	p.ioNamePtr = (StringPtr)name;
	p.ioVRefNum = volume;
	p.ioDirID = directory;
	if (err = PBHCreate((HParmBlkPtr)&p, FALSE))
		return err;

	memset(&p, 0, sizeof(p));
	p.ioNamePtr = (StringPtr)name;
	p.ioVRefNum = volume;
	p.ioDirID = directory;
	if (err = PBHGetFInfo((HParmBlkPtr)&p, FALSE))
		return err;
	p.ioNamePtr = (StringPtr)name;
	p.ioVRefNum = volume;
	p.ioDirID = directory;
	p.ioFlFndrInfo.fdCreator = creator;
	p.ioFlFndrInfo.fdType = type;
	return PBHSetFInfo((HParmBlkPtr)&p, FALSE);
}

/* ----- Delete file --------------------------------------------------- */

short DeleteFile(
	register short volume,
	register long directory,
	register Byte *name)
{
	HFileParam p;

	memset(&p, 0, sizeof(p));
	p.ioNamePtr = (StringPtr)name;
	p.ioVRefNum = volume;
	p.ioDirID = directory;
	return PBHDelete((HParmBlkPtr)&p, FALSE);
}

/* ----- Get file info ------------------------------------------------- */

short InfoFile(
	register short volume,		/* <- Volume reference number */
	register long directory,	/* <- Directory id */
	register Byte *name,		/* <- File name */
	OSType *creator,			/* -> File creator */
	OSType *type,				/* -> File type */
	long *create,				/* -> File creation date */
	long *modif)				/* -> File modification date */
{
	register short err;
	HParamBlockRec p;

	memset(&p, 0, sizeof(p));
	p.fileParam.ioNamePtr = (StringPtr)name;
	p.fileParam.ioVRefNum = volume;
	p.fileParam.ioDirID = directory;
	if (err = PBHGetFInfo(&p, FALSE))
		return err;
	*creator = p.fileParam.ioFlFndrInfo.fdCreator;
	*type = p.fileParam.ioFlFndrInfo.fdType;
	*create = p.fileParam.ioFlCrDat;
	*modif = p.fileParam.ioFlMdDat;
	return err;
}

/* ----- Set file info ------------------------------------------------- */

short SetInfoFile(
	register short volume,		/* <- Volume reference number */
	register long directory,	/* <- Directory id */
	register Byte *name,		/* <- File name */
	OSType creator,				/* <- File creator */
	OSType type,				/* <- File type */
	long create,				/* <- File creation date */
	long modif)					/* <- File modification date */
{
	register short err;
	HParamBlockRec p;

	memset(&p, 0, sizeof(p));
	p.fileParam.ioNamePtr = (StringPtr)name;
	p.fileParam.ioVRefNum = volume;
	p.fileParam.ioDirID = directory;
	if (err = PBHGetFInfo(&p, FALSE))
		return err;
	p.fileParam.ioNamePtr = name;
	p.fileParam.ioVRefNum = volume;
	p.fileParam.ioDirID = directory;
	p.fileParam.ioFlFndrInfo.fdCreator = creator;
	p.fileParam.ioFlFndrInfo.fdType = type;
	p.fileParam.ioFlCrDat = create;
	p.fileParam.ioFlMdDat = modif;
	return PBHSetFInfo(&p, FALSE);
}

/* ----- Rename file --------------------------------------------------- */

short FileRename(
	register short volume,
	register long directory,
	register Byte *oldname,
	register Byte *newname)
{
	HParamBlockRec p;

	memset(&p, 0, sizeof(p));
	p.fileParam.ioNamePtr = (StringPtr)oldname;
	p.fileParam.ioVRefNum = volume;
	p.fileParam.ioDirID = directory;
	p.ioParam.ioMisc = (Ptr)newname;
	return PBHRename(&p, FALSE);
}

/* ----- Set user item in dialogs -------------------------------------- */

void SetUserItem(
	register DialogPtr dialog,
	register short item,
	register ProcPtr function)
{
	short type;
	Handle h;
	Rect box;

	GetDItem(dialog, item, &type, &h, &box);
	SetDItem(dialog, item, userItem, (Handle)function, &box);
}

/* ----- Activate/deactivate controls in dialog ------------------------ */

void ActivateDeactivate(
	register DialogPtr dialog,
	register short item,
	register Boolean activate)
{
	short type;
	Handle h;
	Rect box;

	GetDItem(dialog, item, &type, &h, &box);
	HiliteControl((ControlHandle)h, activate ? 0 : 255);
}

/* ----- Get size of biggest temporary memory -------------------------- */

#define RESERVE1	8192L	/* Leave this in application heap */
#define RESERVE2	32768L	/* Leave this in MF temp memory heap */

unsigned long MaxBuffer(register Boolean *mf)
{
	register unsigned long max1, max2;
	long grow;

	*mf = FALSE;
	max1 = MaxMem(&grow);
	if (max1 < RESERVE1)
		max1 = 0;
	else
		max1 -= RESERVE1;
	if (MFmemory) {
		max2 = MFMaxMem(&grow);
		if (max2 < RESERVE2)
			max2 = 0;
		else
			max2 -= RESERVE2;
		if (max2 > max1) {
			*mf = TRUE;
			max1 = max2;
		}
	}
	return max1;
}

/* ----- Allocate new buffer in temporary memory ----------------------- */

Handle NewBuffer(register unsigned long size, register Boolean mf)
{
	register Handle h;
	short err;

	if (MFmemory && mf) {
		if (h = MFTempNewHandle(size, &err))
			MFTempHLock(h, &err);
	} else {
		if (h = NewHandle(size))
			HLock(h);
	}
	return h;
}

/* ----- Dispose of buffer in temporary memory ------------------------- */

void DisposeBuffer(register Handle h, register Boolean mf)
{
	short err;

	if (h) {
		if (mf)
			MFTempDisposHandle(h, &err);
		else
			DisposHandle(h);
	}
}

/* ----- Check suffix -------------------------------------------------- */

Boolean CheckSuffix(register Byte *name, register Byte *suffix)
{
	register Byte *p;
	register Byte *q;

	for (p = name + *name,
			q = suffix + *suffix; ; --p, --q) {
		if (q == suffix)
			return TRUE;	/* Ok: name ends in suffix */
		if (p == name || *q != *p)
			return FALSE;	/* Sorry */
	}
}

/* ----- Keydown handler for dialog filters ---------------------------- */

Boolean DialogKeydown(
	register DialogPtr dialog,
	register EventRecord *event,
	register short *item)
{
	register Byte key = event->message & 0xFF;
	short type;
	Handle hdl;
	Rect box;
	long ignore;

	if (key == 0x03 || key == 0x0D)	{	/* Enter, return */
		GetDItem(dialog, 1, &type, &hdl, &box);
		HiliteControl((ControlHandle)hdl, 1);
		Delay(3, &ignore);
		HiliteControl((ControlHandle)hdl, 0);
		*item = 1;
		return TRUE;
	}
	return FALSE;
}
