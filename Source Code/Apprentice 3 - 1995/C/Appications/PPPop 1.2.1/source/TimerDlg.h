// Dialog Items

#pragma once

#include "Timer.h"
#include "PPPop.h"

#define rTimeDlg	128

#define	iDate		4	// param text
#define iDaily		6	// radio
#define iMonthly	7	// radio
#define iNever		8	// radio
#define iNow		9	// button
#define kFirstRadio	6
#define kLastRadio	8
#define iDayText	11
#define iDayEText	12

#define	kEnter	(char) 0x03
#define kReturn	(char) 0x0D
#define kEscape	(char) 0x1B
#define kPeriod	'.'

void	DoTimeDlg(void);
pascal 	Boolean OurFilter(DialogPtr dlg, EventRecord *event, short *itemHit);
void	FlashDialogItem(DialogPtr dlg, short itemToFlash);
void	SetDialogItemState(DialogPtr dlg, short controlNumber, short value);
short	GetDialogItemState(DialogPtr dlg, short controlNumber);
void	SetRadioButton(DialogPtr dlg, short buttonNumber);
void 	DoResetDay(DialogPtr dlg, short item);
void 	SetupDay(DialogPtr dlg, short item);

extern	short	gCurrentRadio;
extern	short	gResetDay;

