/*
	Terminal 2.2
	"CancelDialog.c"
*/

#ifdef THINK_C
#include "MacHeaders"
#endif
#ifdef applec
#pragma load ":(Objects):MacHeadersMPW"
#pragma segment Main2
#endif

#include "CancelDialog.h"
#include "Text.h"
#include "Main.h"
#include "Document.h"
#include "Strings.h"
#include "Utilities.h"
#include "FormatStr.h"

#define PROGRESS_CANCEL	1		/* Cancel button */
#define PROGRESS_ACTION	2		/* Action text */
#define PROGRESS_NAME	3		/* File name */
#define PROGRESS_INFO	4		/* Additional info */
#define PROGRESS_TIME	5		/* Estimated time left */
#define PROGRESS_MARK	6		/* Current file mark */
#define PROGRESS_MAX	7		/* Maximum number of bytes */
#define PROGRESS_USER	8		/* Progress indicator */
#define PROGRESS_BLOCK	9		/* Current block number */
#define PROGRESS_TEXT	10		/* Error message */
#define PROGRESS_ERR	11		/* Error count */

static DialogPtr Dialog = 0;	/* Current dialog */
static Rect Box;				/* Progress indicator rectangle */
static short Progress;			/* Current progress value (pixels) */

/* ----- Convert seconds to minutes ------------------------------------ */

Byte *SecondsToString(register long seconds, register Byte *num)
{
	return FormatStr(num, (Byte *)"\p%l'%02l\"",
		seconds / 60, seconds % 60);
}

/* ----- Display statistics message ------------------------------------ */

void Statistics(
	register long bytes,		/* Number of bytes transferred */
	register long seconds,		/* Time for transfer in seconds */
	register short error)		/* Error code */
{
	register Byte s[256];
	Byte sec[256];

	SysBeep(1);
	MakeMessage(TerminalWindow,
		FormatStr(s, MyString(STR_M, M_TRANSFER),
			bytes, SecondsToString(seconds, sec),
			seconds ? bytes/seconds : bytes, error));
}

/* ----- Remove cancel/progress dialog window -------------------------- */

void RemoveCancelDialog(void)
{
	Point position;

	if (Dialog) {
		position = topLeft(((GrafPtr)Dialog)->portRect);
		SetPort(Dialog);
		LocalToGlobal(&position);
		if (position.h != Settings.progressWindow.h ||
				position.v != Settings.progressWindow.v ) {
			Settings.progressWindow = position;
			Settings.dirty = TRUE;
		}
		DisposDialog(Dialog);
		Dialog = 0;
		RedrawDocument();
		DisableItem(GetMenu(EDIT), SHOWPW);
	}
}

/* ----- Check if canceled --------------------------------------------- */

Boolean CheckCancel(void)
{
	EventRecord event;
	DialogPtr dialog;
	short button;

	do {
		if (WNE)
			WaitNextEvent(everyEvent, &event, 0, 0);
		else {
			SystemTask();
			GetNextEvent(everyEvent, &event);
		}
		if (IsDialogEvent(&event) &&
				DialogSelect(&event, &dialog, &button) &&
				button == PROGRESS_CANCEL) {
			Transfer = 0;
			return TRUE;	/* Cancel by button */
		}
		DoEvent(&event);
		if (!Transfer)
			return TRUE;	/* Cancel by menu command */
	} while (event.what != nullEvent);
	return FALSE;			/* Not yet canceled */
}

/* ----- Draw progress indicator (user item) --------------------------- */

static pascal void DrawProgress(
	register WindowPtr window,
	register short itemNo)
{
#pragma unused(window, itemNo)
	Rect box;
	register short i;

	box = Box;
	if ((i = box.left + Progress) > box.right)
		i = box.right;
	box.right = i;
	PaintRect(&box);
	box = Box;
	box.left = i;
	EraseRect(&box);
	FrameRect(&Box);
}

/* ----- Set file name in progress dialog ------------------------------ */

void NameProgress(Byte *name)
{
	SetEText(Dialog, PROGRESS_NAME, name);
}

/* ----- Set info text in progress dialog ------------------------------ */

void InfoProgress(Byte *info)
{
	SetEText(Dialog, PROGRESS_INFO, info);
}

/* ----- Setup and draw progress dialog window ------------------------- */

void DrawProgressDialog(
	register short prompt,
	register Byte *name)
{
	register DialogTHndl h;
	register Rect *p;
	short type;
	Handle item;

	MakeMessage(TerminalWindow, MyString(STR_M, M_PROGRESS));
	Progress = 0;
	if (h = (DialogTHndl)GetResource('DLOG', DLOG_PROGRESS)) {
		p = &(**h).boundsRect;
		OffsetRect(p, Settings.progressWindow.h - p->left,
			Settings.progressWindow.v - p->top);
	}
	if (Dialog = GetNewDialog(DLOG_PROGRESS, 0L, (WindowPtr)-1L)) {
		SetEText(Dialog, PROGRESS_ACTION, MyString(STR_P, prompt));
		SetEText(Dialog, PROGRESS_NAME, name);
		GetDItem(Dialog, PROGRESS_USER, &type, &item, &Box);
		SetDItem(Dialog, PROGRESS_USER, userItem, (Handle)DrawProgress,
			&Box);
		SetEText(Dialog, PROGRESS_INFO, EmptyStr);
		DrawDialog(Dialog);
		EnableItem(GetMenu(EDIT), SHOWPW);
	}
}

/* ----- Update of progress dialog ------------------------------------- */

void UpdateProgress(
	register long progress,
	register long maximum,
	long time,
	long block,
	long error,
	Byte *message)
{
	Byte num[10];

	if (Dialog) {
		SetPort(Dialog);

		if (maximum)
			Progress = ((Box.right - Box.left) * progress) / maximum;
		DrawProgress(Dialog, PROGRESS_USER);

		NumToString(progress, num);
		SetEText(Dialog, PROGRESS_MARK, num);
		NumToString(maximum, num);
		SetEText(Dialog, PROGRESS_MAX, num);

		SecondsToString(time, num);
		SetEText(Dialog, PROGRESS_TIME, num);

		NumToString(block, num);
		SetEText(Dialog, PROGRESS_BLOCK, num);
		NumToString(error, num);
		SetEText(Dialog, PROGRESS_ERR, num);
		SetEText(Dialog, PROGRESS_TEXT, message);
	}
}

/* ----- Bring progress window to front -------------------------------- */

void SelectCancelDialog(void)
{
	if (Dialog)
		SelectWindow(Dialog);
}
