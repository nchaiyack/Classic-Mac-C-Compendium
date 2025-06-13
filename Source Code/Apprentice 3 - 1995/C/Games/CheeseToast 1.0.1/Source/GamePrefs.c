/************************************************************************************
 * GamePrefs.c
 *
 * CheeseToast by Jim Bumgardner
 *
 ************************************************************************************/

#include "CToast.h"

// Data Structure that holds preferred keyboard mapping, sound level
// and player name
//
PrefsData		gPrefs = 
	{6,0x02,0x31, 0,		// fire
	 6,0x80,0x37, 0,		// thrust
	 15,0x08,0x7B, 0,		// left
	 15,0x10,0x7C, 0,		// right
	 15,0x40,0x3A, 0,		// shield
//	Sound Level
	 0,
	 "\pWinner",
	 ""};

void LoadPreferences(void)
{
	Handle	h;
	long	size;
	h = GetResource(PrefsResType,128);
	if (h) {
		size = GetHandleSize(h);
		BlockMove(*h, &gPrefs, size);
		ReleaseResource(h);
	}
}

void SavePreferences(void)
{
	Handle	h;
	while ((h = GetResource(PrefsResType, 128)) != NULL) {
		RmveResource(h);
		DisposHandle(h);
	}
	h = NewHandle(sizeof(PrefsData));
	BlockMove(&gPrefs, *h, sizeof(PrefsData));
	AddResource(h,PrefsResType,128,"\pPreferences");
	WriteResource(h);
	ReleaseResource(h);
}

// Dialog for Remapping Keys

#define KeysDLOG	129
#define FireItem	2
#define ThrustItem	3
#define ShieldItem	4
#define LeftItem	5
#define RightItem	6

short	gLastKeyCode,gLastAsciiCode;

void FeedbackKey(DialogPtr dp, short itemNbr, short keyCode, short asciiCode)
{
	short		t;
	Handle		h;
	Rect		r;
	StringPtr	p, asciiStr=(StringPtr) "  ";

	p = NULL;
	GetDItem(dp, itemNbr, &t,&h,&r);

	// Convert Non-Printable Keys to Words
	switch (keyCode) {
	case 0x24:	p = "\pReturn";		break;
	case 0x31:	p = "\pSpace";		break;
	case 0x33:	p = "\pDelete";		break;
	case 0x35:	p = "\pEsc";		break;
	case 0x37:	p = "\pCmd";		break;
	case 0x38:	p = "\pShift";		break;
	case 0x3D:
	case 0x3A:	p = "\pOption";		break;
	case 0x3E:
	case 0x3B:	p = "\pControl";	break;
	case 0x47:	p = "\pClear";		break;
	case 0x4C:	p = "\pEnter";		break;
	case 0x7B:	p = "\pLeft";		break;
	case 0x7C:	p = "\pRight";		break;
	case 0x7E:	p = "\pUp";			break;
	case 0x7D:	p = "\pDown";		break;
	default:	p = asciiStr;
				asciiStr[0] = 1;
				asciiStr[1] = asciiCode;
				break;
	
	}
	SetIText(h, p);
	SelIText(dp, itemNbr, 0,32767);
}

void RegisterKey(short itemNbr, short keyCode, short asciiCode)
{
	switch (itemNbr) {
	case FireItem:	
		gPrefs.fireKeyByte = keyCode >> 3;
		gPrefs.fireKeyBit = (1 << (keyCode & 7));
		gPrefs.fireKeyCode = keyCode;
		gPrefs.fireKeyAscii = asciiCode;
		break;
	case ThrustItem:
		gPrefs.thrustKeyByte = keyCode >> 3;
		gPrefs.thrustKeyBit = (1 << (keyCode & 7));
		gPrefs.thrustKeyCode = keyCode;
		gPrefs.thrustKeyAscii = asciiCode;
		break;
	case LeftItem:
		gPrefs.leftKeyByte = keyCode >> 3;
		gPrefs.leftKeyBit = (1 << (keyCode & 7));
		gPrefs.leftKeyCode = keyCode;
		gPrefs.leftKeyAscii = asciiCode;
		break;
	case RightItem:
		gPrefs.rightKeyByte = keyCode >> 3;
		gPrefs.rightKeyBit = (1 << (keyCode & 7));
		gPrefs.rightKeyCode = keyCode;
		gPrefs.rightKeyAscii = asciiCode;
		break;
	case ShieldItem:
		gPrefs.shieldKeyByte = keyCode >> 3;
		gPrefs.shieldKeyBit = (1 << (keyCode & 7));
		gPrefs.shieldKeyCode = keyCode;
		gPrefs.shieldKeyAscii = asciiCode;
		break;
	}
}

pascal	Boolean	RemapDialogHook(WindowPtr dp,EventRecord *ep,int *ip)
{
	short	stroke;
	short	editField;

	switch(ep->what) {
		case keyDown:
			if ((ep->message & charCodeMask) != '\t') {
				// record actual keystroke
				gLastKeyCode = (ep->message & keyCodeMask) >> 8;
				gLastAsciiCode = (ep->message & charCodeMask);
				editField = ((DialogPeek) dp)->editField+1;
				RegisterKey(editField, gLastKeyCode, gLastAsciiCode);
				FeedbackKey(dp, editField, gLastKeyCode, gLastAsciiCode);
				return TRUE;
			}
			return FALSE;
		case updateEvt:
			break;
		case nullEvent:
			// On idle, check for the following keys
			stroke = 0;
			if (ep->modifiers & cmdKey) {
				stroke = 0x37;
			}
			else if (ep->modifiers & shiftKey) {
				stroke = 0x38;
			}
			else if (ep->modifiers & optionKey) {
				stroke = 0x3A;
			}
			else if (ep->modifiers & controlKey) {
				stroke = 0x3B;
			}
			if (stroke) {
				editField = ((DialogPeek) dp)->editField+1;
				gLastKeyCode = stroke;	// !! Record It
				RegisterKey(editField, gLastKeyCode, 0);
				FeedbackKey(dp, editField, gLastKeyCode, 0);
			}
			break;
		default:
			break;
	}
	return FALSE;
}

void RemapKeys(void)
{
	GrafPtr		savePort;
	DialogPtr	dp;
	short		itemHit,t;
	Handle		h;
	Rect		r;
	Str63		theText;

	GetPort(&savePort);
	if ((dp = GetNewDialog(KeysDLOG,NULL,(WindowPtr) -1)) == NULL)
		return;


	ShowWindow(dp);
	SetPort(dp);

	ShowCursor();

	FeedbackKey(dp, FireItem, gPrefs.fireKeyCode, gPrefs.fireKeyAscii);
	FeedbackKey(dp, ThrustItem, gPrefs.thrustKeyCode, gPrefs.thrustKeyAscii);
	FeedbackKey(dp, LeftItem, gPrefs.leftKeyCode, gPrefs.leftKeyAscii);
	FeedbackKey(dp, RightItem, gPrefs.rightKeyCode, gPrefs.rightKeyAscii);
	FeedbackKey(dp, ShieldItem, gPrefs.shieldKeyCode, gPrefs.shieldKeyAscii);

	SelIText(dp, FireItem, 0,32767);

	do {
		ModalDialog((ProcPtr) RemapDialogHook, &itemHit);
		switch (itemHit) {
		case FireItem:
		case ThrustItem:
		case ShieldItem:
		case LeftItem:
		case RightItem:
			SelIText(dp, itemHit, 0, 32767);
			break;
		default:
			break;
		}
	} while (itemHit != OK);

	HideCursor();

	DisposDialog(dp);

	SetPort(savePort);

	SavePreferences();
}

