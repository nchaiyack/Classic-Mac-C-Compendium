/*
	Terminal 2.2
	"MySF.c"
*/

#ifdef THINK_C
#include "MacHeaders"
#endif
#ifdef applec
#pragma load ":(Objects):MacHeadersMPW"
#pragma segment Main2
#endif

#include "MySF.h"
#include "Strings.h"
#include "Utilities.h"
#include "Text.h"
#include "Main.h"
#include "Document.h"

static Byte *DialogOpenSave;	/* Text for Open/Save button */
static Byte *Suffix;			/* File name suffix */
static OSType Creator;			/* Creator nor wanted in SFGetFile */

/* ----- File filter for SF -------------------------------------------- */

static pascal Boolean MyFileFilter(register ParamBlockRec *pb)
{
	if (Creator && pb->fileParam.ioFlFndrInfo.fdCreator == Creator)
		return TRUE;
	return Suffix ?
		!CheckSuffix((Byte *)pb->fileParam.ioNamePtr,
			Suffix) : FALSE;
}

#ifdef OUTLINE
/* ----- Filter function for SF modal dialog --------------------------- */

/* Outline Open/Save button */

static pascal Boolean MyFilter(dialog, event, item)
register DialogPtr dialog;
register EventRecord *event;
short *item;
{
#pragma unused(item)
	DialogPtr dp;
	short i;

	if (event->what == updateEvt) {
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
	}
	return FALSE;	/* ModalDialog() should handle event */
}
#else
#define MyFilter 0
#endif

/* ----- SF Dialog Hook ------------------------------------------------ */

static pascal short MyHook(
	register short itemno,
	register DialogPtr dialog)
{
	short type;
	Handle item;
	Rect box;

	if (itemno == -1 && DialogOpenSave) {
		/* Set text of Open/Save button */
		GetDItem(dialog, getOpen, &type, &item, &box);
		SetCTitle((ControlHandle)item, DialogOpenSave);
	}
	return itemno;
}

/* ----- Get a file ---------------------------------------------------- */

void MySFGetFile(
	register Byte *open,			/* Text for "Open" button */
	register Byte *suffix,			/* File name suffix required */
	register short numTypes,
	register OSType *typeList,
	register SFReply *reply,
	OSType creator)					/* Creator NOT wanted */
{
	Point where;

	DialogOpenSave = open;
	Suffix = suffix;
	Creator = creator;
	GetDlogOrigin(getDlgID, &where);
	SFPGetFile(where,
		(void *)EmptyStr,
		(FileFilterProcPtr)MyFileFilter,
		numTypes,
		(void *)typeList,
		(DlgHookProcPtr)MyHook,
		reply,
		getDlgID,
		(ModalFilterProcPtr)MyFilter);
}

/* ----- Put a file ---------------------------------------------------- */

void MySFPutFile(
	register Byte *save,			/* Text for "Save" button */
	register Byte *prompt,			/* Prompt text */
	register Byte *name,			/* Default file name */
	register SFReply *reply)
{
	Point where;

	DialogOpenSave = save;
	GetDlogOrigin(putDlgID, &where);
	SFPPutFile(where, prompt, name, MyHook, reply, putDlgID, MyFilter);
}

/* ----- Select a directory -------------------------------------------- */

static pascal Boolean NoFileFilter(register ParmBlkPtr p)
{
	/* Normally, folders are ALWAYS shown, and aren't even passed to
	this file filter for judgement. Under such circumstances, it is	
	only necessary to blindly return TRUE (allow no files whatsoever).
	However, Standard File is not documented in such a manner, and
	this feature may not be TRUE in the future. Therefore, we DO check
	to see if the entry passed to us describes a file or a directory. */

	return (p->fileParam.ioFlAttrib & 0x10) == 0;
}

static pascal short DirectoryHook(
	register short itemno,
	register DialogPtr dialog)
{
	short type;
	Handle item;
	Rect box;
	Point where;

	if (itemno == -1) {
		/* Set text of "Open" button */
		GetDItem(dialog, getOpen, &type, &item, &box);
		SetCTitle((ControlHandle)item, MyString(STR_G, G_SELECT));
	} else if (itemno == 103) {
		/* Double click on folder name or "Select" clicked */
		GetDItem(dialog, 7,  &type, &item, &box);
		GetMouse(&where);
		if (!PtInRect(where, &box))
			itemno = 1;		/* Treat like file open */
	}
	return itemno;
}

Boolean SelectDirectory(
	short *volume,
	long *directory)
{
	Point where;
	SFReply reply;

	GetDlogOrigin(getDlgID, &where);
	SFPGetFile(where, (void *)EmptyStr, (FileFilterProcPtr)NoFileFilter, 1,
		(void *)"!@#$", (DlgHookProcPtr)DirectoryHook, &reply, getDlgID,
		(ModalFilterProcPtr)MyFilter);
	if (reply.good) {
		*volume = -SFSaveDisk;
		*directory = reply.fType;
		/* *directory = CurDirStore; */
	}
	return reply.good;
}
