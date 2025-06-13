/* PPPop, verison 1.2  June 6, 1995  Rob Friefeld
   Reset Time Dialog

   Dialog Handler, from Macintosh Programming Secrets by Knaster and Rollin
*/

#include <Packages.h>
#include "TimerDlg.h"

short	gCurrentRadio;				/*	Item number of the currently selected
										radio button. */

short	gResetDay;					/*  If monthly reset, which day */

/*******************************************************************************/
void	DoTimeDlg(void)
{
	short		itemHit;
	DialogPtr	ourDialog;
	short		oldRadio, oldDay;
	Str255		dateString, timeString;

	ourDialog = GetNewDialog(rTimeDlg, nil, (WindowPtr) -1);

	if ((gCurrentRadio < kFirstRadio) || (gCurrentRadio > kLastRadio))
		gCurrentRadio = kLastRadio;
	oldRadio = gCurrentRadio;		// for undo if cancelled
	SetRadioButton(ourDialog, gCurrentRadio);
	
	SetupDay(ourDialog, iDayEText);
	oldDay = gResetDay;
	if (gCurrentRadio != iMonthly) {
		HideDItem(ourDialog, iDayText);
		HideDItem(ourDialog, iDayEText);
	}
			
	IUDateString(gResetTime, shortDate, dateString);
	IUTimeString(gResetTime, FALSE, timeString);
	ParamText(dateString, timeString, "\p", "\p");

	ShowWindow(ourDialog);
	do {
		ModalDialog(OurFilter, &itemHit);
		if ((itemHit >= kFirstRadio) && (itemHit <= kLastRadio))
			SetRadioButton(ourDialog, itemHit);
		else if (itemHit == iNow)
			ResetCumulativeTime();
		else if (itemHit == iDayEText)
			DoResetDay(ourDialog, itemHit);
	} while ((itemHit != ok) && (itemHit != cancel) && (itemHit != iNow));
	
	if (itemHit == cancel) {
		gCurrentRadio = oldRadio;
		gResetDay = oldDay;
	}

	DisposeDialog(ourDialog);
}


/*******************************************************************************/
pascal Boolean OurFilter(DialogPtr dlg, EventRecord *event, short *itemHit)
{
	short	iKind;
	Handle	iHandle;
	Rect	iRect;
	char	key;
	short	radius;

	switch (event->what) {

		case keyDown:
		case autoKey:
			key = event->message & charCodeMask;
			if (event->modifiers & cmdKey) {		/* Command key down */
				if (key == kPeriod) {
					*itemHit = cancel;
					FlashDialogItem(dlg, *itemHit);
					return TRUE;
				}
				else
					return FALSE;
			} else {
				if ((key == kReturn) || (key == kEnter)) {
					*itemHit = ok;
					FlashDialogItem(dlg, *itemHit);
					return TRUE;
				}
				if (key == kEscape) {
					*itemHit = cancel;
					FlashDialogItem(dlg, *itemHit);
					return TRUE;
				}
			}
			return FALSE;

		case updateEvt:
			SetPort(dlg);
			GetDItem(dlg, ok, &iKind, &iHandle, &iRect);
			InsetRect(&iRect, -4, -4);
			radius = (iRect.bottom - iRect.top) / 2;
			if (radius < 16)
				radius = 16;
			PenNormal();
			PenSize(3,3);
			FrameRoundRect(&iRect, radius, radius);

			return FALSE;

		default:
			return FALSE;
	}
}


/*******************************************************************************/

void	FlashDialogItem(DialogPtr dlg, short itemToFlash)
{
	short	iKind;
	Handle	iHandle;
	Rect	iRect;
	long	ignored;

	GetDItem(dlg, itemToFlash, &iKind, &iHandle, &iRect);
	HiliteControl((ControlHandle) iHandle, 1);
	Delay(8, &ignored);
	HiliteControl((ControlHandle) iHandle, 0);
}


/*******************************************************************************/
void	SetDialogItemState(DialogPtr dlg, short controlNumber, short value)
{
	short	iKind;
	Handle	iHandle;
	Rect	iRect;

	GetDItem(dlg, controlNumber, &iKind, &iHandle, &iRect);
	SetCtlValue((ControlHandle) iHandle, value);
}


/*******************************************************************************/
short	GetDialogItemState(DialogPtr dlg, short controlNumber)
{
	short	iKind;
	Handle	iHandle;
	Rect	iRect;

	GetDItem(dlg, controlNumber, &iKind, &iHandle, &iRect);
	return GetCtlValue((ControlHandle) iHandle);
}

/*******************************************************************************/
void	SetRadioButton(DialogPtr dlg, short buttonNumber)
{
		SetDialogItemState(dlg, gCurrentRadio, 0);
		gCurrentRadio = buttonNumber;
		if ((gCurrentRadio >= kFirstRadio) && (gCurrentRadio <= kLastRadio)) {
			SetDialogItemState(dlg, gCurrentRadio, 1);
			if (gCurrentRadio != iMonthly) {
				HideDItem(dlg, iDayText);
				HideDItem(dlg, iDayEText);
			}
			else {
				SetupDay(dlg, iDayEText);
				ShowDItem(dlg, iDayText);
				ShowDItem(dlg, iDayEText);
				SelIText(dlg, iDayEText, 0, 32767);
			}
		}
}

/*******************************************************************************/
void SetupDay(DialogPtr dlg, short item)
{
	Handle	iHandle;
	Rect	iRect;
	short	iType;
	Str255	resetDay;
	
	GetDItem(dlg, item, &iType,  &iHandle, &iRect);
	NumToString((long)gResetDay, resetDay);
	if ((gResetDay < 1) || (gResetDay > 31))
		gResetDay = 1;
	SetIText(iHandle, resetDay);
	SelIText(dlg, item, 0, 32767);		// Preselect all
}
	

/*******************************************************************************/
void DoResetDay(DialogPtr dlg, short item)
{
	Handle	iHandle;
	short	iType;
	Rect	iRect;
	Str255	iText;
	long	day;
	
	GetDItem(dlg, item, &iType, &iHandle, &iRect);
	GetIText(iHandle, iText);
	StringToNum(iText, &day);
	if ((day < 1L ) || (day > 31L)) {
		SysBeep(5);
		SetupDay(dlg, item);
	}
	else
		gResetDay = (short)day;
}

		
	