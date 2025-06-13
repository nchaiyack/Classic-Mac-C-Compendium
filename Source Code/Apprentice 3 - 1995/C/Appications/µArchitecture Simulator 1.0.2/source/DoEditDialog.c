/*
Copyright й 1993,1994,1995 Fabrizio Oddone
еее еее еее еее еее еее еее еее еее еее
This source code is distributed as freeware:
you may copy, exchange, modify this code.
You may include this code in any kind of application: freeware,
shareware, or commercial, provided that full credits are given.
You may not sell or distribute this code for profit.
*/

//#pragma load "MacDump"

#include	"UtilsSys7.h"
#include	"Globals.h"
#include	"Animation.h"
#include	"DoEditDialog.h"
#include	"DoMenu.h"
#include	"Main.h"
#include	"MovableModal.h"
#include	"Registers.h"
#include	"SimUtils.h"
#include	"Independents.h"
#include	"Conversions.h"

#if defined(FabSystem7orlater)

#pragma segment Rare

static short EditValue(ConstStr255Param str0, ConstStr255Param str1, short edit, short base);
static void EditUserProc(DialogPtr myDPtr, Handle itemHandle, short itemHit);
static Boolean EditNumPreProcessKey(EventRecord *thEv, DialogPtr theD);

static short	lastChoice;

//static pascal Boolean EditNumFilter(DialogPtr, EventRecord *, short *);

/* DoEditDump: we shall edit a memory location */

Boolean DoEditDump(short *addr, short wordn)
{
Str27	tempStr0;
Str15	tempStr1;
register short	newv;
register Boolean changed;

GetIndHString((StringPtr)&tempStr0, Get1Resource('STR#', kValueFollow), kAtMem);
ShortToHexString(wordn, tempStr1);
newv = EditValue(tempStr0, tempStr1, *addr, kPOP_HEX);
if (changed = (newv != *addr))
	*addr = newv;
return changed;
}

/* DoEditDialog: we shall edit a register or an internal processor part */

Boolean DoEditDialog(short item, short pclass, short base)
{
Str27	tempStr0;
Str31	tempStr1;
register short *oldv;
register short	newv;
register Boolean changed;

GetIndHString((StringPtr)&tempStr0, Get1Resource('STR#', kValueFollow), kInThe);
GetIndHString((StringPtr)&tempStr1, Get1Resource('STR#', pclass), item);
oldv = pclass == kPART_NAMES ? &gParts[item] : &gRegs[item];
newv = EditValue(tempStr0, tempStr1, *oldv, base);
if (changed = (newv != *oldv)) {
	*oldv = newv;
	if (pclass == kPART_NAMES)
		ChangedBox(item);
	}
return changed;
}

enum {
kItemEditValue = 3,
kItemPopupBase
};

static short EditValue(ConstStr255Param str0, ConstStr255Param str1, short edit, short base)
{

enum {
kDLOG_EDITVALUE = 261
};

Str255	EditStr;
long	tempLong;

dialogItems	things[] = {{ ok, 0, 1L },
						{ cancel, 0, 0L },
						{ kItemEditValue, 0, 0L },
						{ kItemPopupBase, 0, 0L },
						{ 0, 0, 0L}
						};

things[kItemEditValue-1].refCon = (long)&EditStr;
things[kItemPopupBase-1].refCon = base;
tempLong = edit;
lastChoice = base;
ParamText(str0, str1, nil, nil);
FromNumToString(EditStr, edit, base);
//things[kItemEditValue-1].refCon = ;

if (HandleMovableModalDialog(things, gPrefs.remembWind ? &gPrefs.editValueTL : nil, nil, nil, nil, nil, EditUserProc,
	AdjustMenus,
	Handle_My_Menu,
	DomyKeyEvent,
	EditNumPreProcessKey,
	nil,
	DoUpdate,
	DoActivate,
	DoHighLevelEvent,
	DoOSEvent,
	DoIdle,
	ULONG_MAX,
	kDLOG_EDITVALUE) == ok) {
	tempLong = 0L;
	FromStringToNum((ConstStr255Param)&EditStr, &tempLong, lastChoice);
	}
return tempLong;
}

void EditUserProc(DialogPtr myDPtr, Handle itemHandle, short itemHit)
{
Str255	tempStr;
Rect	tempRect;
Handle	tempH;
long	tempnum;
short	null;
register short	newChoice;

if (itemHit == kItemPopupBase)
	if ((newChoice = GetControlValue((ControlHandle)itemHandle)) != lastChoice) {
		GetDialogItem(myDPtr, kItemEditValue, &null, &tempH, &tempRect);
		GetDialogItemText(tempH, tempStr);
		tempnum = 0L;
		FromStringToNum((ConstStr255Param)&tempStr, &tempnum, lastChoice);
		FromNumToString(tempStr, (short)tempnum, newChoice);
		lastChoice = newChoice;
		SetDialogItemText(tempH, tempStr);
		SelectDialogItemText(myDPtr, kItemEditValue, 0, 0x7FFF);
		}
}

Boolean EditNumPreProcessKey(EventRecord *thEv, DialogPtr theD)
{
short	iHit;
unsigned char	keypressed;
Boolean	result = true;

keypressed = CHARFROMMESSAGE(thEv->message);
if (lastChoice != kPOP_TEXT) {
	if ((keypressed >= 'a') && (keypressed <= 'z')) {
		keypressed -= 'a' - 'A';	/* cambiare! */
		CHARFROMMESSAGE(thEv->message) = keypressed;
		}
	iHit = ((DialogPeek)theD)->editField + 1;
	if (keypressed >= 32 && ((thEv->modifiers & cmdKey) == 0)) {
		switch( lastChoice ) {
			case kPOP_DEC:	return( Munger((Handle)GetString(kSTR_DECALLOWED), 1L,
									&keypressed, 1L, 0L, 0L) >= 0L );
							break;
			case kPOP_HEX:	return( Munger((Handle)GetString(kSTR_HEXALLOWED), 1L,
									&keypressed, 1L, 0L, 0L) >= 0L );
							break;
			case kPOP_OCT:	return( Munger((Handle)GetString(kSTR_OCTALLOWED), 1L,
									&keypressed, 1L, 0L, 0L) >= 0L );
							break;
			case kPOP_BIN:	return( Munger((Handle)GetString(kSTR_BINALLOWED), 1L,
									&keypressed, 1L, 0L, 0L) >= 0L );
							break;
			}
		}
	}
return result;
}

/* EditNumFilter: the filter filters entered characters, of course */
/*
static pascal Boolean EditNumFilter(DialogPtr theD, EventRecord *thEv, short *iHit)
{
GrafPtr	savePort;
unsigned char	keypressed;
register Boolean	retVal;

switch(thEv->what) {
	case keyDown	:
	case autoKey	:
		keypressed = CHARFROMMESSAGE(thEv->message);
		if (lastChoice != kPOP_TEXT) {
			if ((keypressed >= 'a') && (keypressed <= 'z')) {
				keypressed -= 'a' - 'A';
				CHARFROMMESSAGE(thEv->message) = keypressed;
				}
			if ((keypressed >= 32) && (keypressed != 0x7F) && ((thEv->modifiers & cmdKey) == 0)) {
				*iHit = kItemEditValue;
				switch( lastChoice ) {
					case kPOP_DEC:	return( Munger((Handle)GetString(kSTR_DECALLOWED), 1L,
											&keypressed, 1L, 0L, 0L) < 0L );
									break;
					case kPOP_HEX:	return( Munger((Handle)GetString(kSTR_HEXALLOWED), 1L,
											&keypressed, 1L, 0L, 0L) < 0L );
									break;
					case kPOP_OCT:	return( Munger((Handle)GetString(kSTR_OCTALLOWED), 1L,
											&keypressed, 1L, 0L, 0L) < 0L );
									break;
					case kPOP_BIN:	return( Munger((Handle)GetString(kSTR_BINALLOWED), 1L,
											&keypressed, 1L, 0L, 0L) < 0L );
									break;
					}
				}
			}
		break;
	case updateEvt:
		if (theD != (DialogPtr)thEv->message) {
			DoUpdate(thEv);
			*iHit = kfakeUpdateItem;
			return true;
			}
		break;
	case activateEvt:
		if (theD != (DialogPtr)thEv->message) {
			DoActivate(thEv);
			*iHit = kfakeUpdateItem;
			return true;
			}
		break;
	}
GetPort(&savePort);
SetPort(theD);
retVal = StdFilterProc(theD, thEv, iHit);
SetPort(savePort);
return retVal;
}
*/
#endif

