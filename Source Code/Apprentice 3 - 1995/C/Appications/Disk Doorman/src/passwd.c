/* passwd.c */

#include <Processes.h>

#define rTwoItemDialog			200
#define rDifferentFontDialog	201
#define rInternalBufferDialog	202
#define rDisplayPasswordDialog	300

#define kPasswdDLOGid 128

extern ProcessSerialNumber	gPSN;

void			DisplayPassword(char * password);

char *			TwoItemDialog(void);
pascal Boolean	TwoItemFilter(DialogPtr dlog,EventRecord *event,short *itemHit);

char *			DifferentFontDialog(void);
pascal void		ChicagoTextItem(WindowPtr wind,short item);

char *			InternalBufferDialog(void);
pascal Boolean	InternalBufferFilter(DialogPtr dlog,EventRecord *event,short *itemHit);
void			DeleteRange(char *buffer,short start,short end);
void			InsertChar(char *buffer,short pos,char c);

//
//	Password
//	 Sample of three different ways to implement a password dialog
//
//	Tim Dierks, UK Mac DTS
//	August, 1991
//

//	This file contains all the interesting password stuff:
//   Sample.c is just the application shell, derived from CSample

#include <Types.h>
#include <Memory.h>
#include <Resources.h>
#include <OSUtils.h>
#include <Quickdraw.h>
#include <Fonts.h>
#include <Events.h>
#include <OSEvents.h>
#include <Windows.h>
#include <Menus.h>
#include <Dialogs.h>
#include <TextEdit.h>

/****************************************************************
void
DisplayPassword(char *password)
{	DialogPtr	dlog;
	short		item;
	
	dlog = GetNewDialog(rDisplayPasswordDialog,0L,(WindowPtr) -1L);
	
	ParamText(password,0L,0L,0L);
	
	do
	{	ModalDialog(0L,&item);
	} while (item != 1);			// Until the OK button gets hit
	
	DisposDialog(dlog);
}

char *
TwoItemDialog()
{	static char		password[256];
	DialogPtr		dlog;
	Handle			itemH;
	ControlHandle	chkBox;
	short			item,itemType,chkVal;
	Rect			box;
	Point			size;
	
	dlog = GetNewDialog(rTwoItemDialog,0L,(WindowPtr) -1L);
	
	do
	{	ModalDialog(TwoItemFilter,&item);
		if (item == 4)											// Hide/show checkbox
		{	GetDItem(dlog,4,&itemType,(Handle*)&chkBox,&box);	// Get check value
			chkVal = !GetCtlValue(chkBox);
			SetCtlValue(chkBox,chkVal);							// Invert it
			size.v = dlog->portRect.bottom - dlog->portRect.top;
			size.h = dlog->portRect.right - dlog->portRect.left;
			if (chkVal)
				size.v += 35;
			else
				size.v -= 35;
			
			SizeWindow(dlog,size.h,size.v,true);		// Resize window
		}
	} while (item != 1);			// Until the OK button is hit
	
	GetDItem(dlog,3,&itemType,&itemH,&box);		// Get text from hidden dialog item
	GetIText(itemH,password);
	
	DisposDialog(dlog);
	
	return password;
}

pascal Boolean
TwoItemFilter(DialogPtr dlog,EventRecord *event,short *itemHit)
{	DialogPtr	evtDlog;
	short		selStart,selEnd;
	
	if (event->what == keyDown || event->what == autoKey)
	{	switch (event->message & charCodeMask)
		{	case '\n':			// Return  (hitting return or enter is the same as hitting the OK button)
			case '\003':		// Enter
				*itemHit = 1;		// OK Button
				return true;		// We handled the event
			case '\t':			// Tab
				event->what = nullEvent;	// Do nothing (don't let the user tab to the hidden field)
				return false;
			case '\034':		// Left arrow  (Keys that just change the selection)
			case '\035':		// Right arrow
			case '\036':		// Up arrow
			case '\037':		// Down arrow
				return false;			// Let ModalDialog handle them
			default:
				selStart = (**((DialogPeek)dlog)->textH).selStart;		// Get the selection in the visible item
				selEnd = (**((DialogPeek)dlog)->textH).selEnd;
				SelIText(dlog,3,selStart,selEnd);				// Select text in invisible item
				DialogSelect(event,&evtDlog,itemHit);			// Input key
				SelIText(dlog,2,selStart,selEnd);				// Select same area in visible item
				if ((event->message & charCodeMask) != '\010')	// If it's not a backspace (backspace is the only key that can affect both the text and the selection- thus we need to process it in both fields, but not change it for the hidden field.
					event->message = '¥';						// Replace with character to use
				DialogSelect(event,&evtDlog,itemHit);			// Put in fake character
				return true;
		}
	}
	
	return false;			// For all non-keyDown events
}

char *
DifferentFontDialog()
{	static char		password[256];
	DialogPtr		dlog;
	Handle			itemH;
	short			item,itemType,font;
	Rect			box;
	
	GetFNum("\p.Pwd",&font);		// Get the font number for our password font (it begins with a period, so AddResMenu won't add it)
	SetDAFont(font);				// Use this font for static and edit text items in further dialogs
	
	dlog = GetNewDialog(rDifferentFontDialog,0L,(WindowPtr) -1L);
	
	GetDItem(dlog,3,&itemType,&itemH,&box);			// Because SetDAFont affects static items, too, we've got to use user items to draw our prompts
	SetDItem(dlog,3,itemType,(Handle)ChicagoTextItem,&box);
	GetDItem(dlog,4,&itemType,&itemH,&box);
	SetDItem(dlog,4,itemType,(Handle)ChicagoTextItem,&box);
	
	do
	{	ModalDialog(0L,&item);
	} while (item != 1);			// Until the OK button is hit
	
	GetDItem(dlog,2,&itemType,&itemH,&box);		// Get text from TE item
	GetIText(itemH,password);
	
	DisposDialog(dlog);
	
	SetDAFont(0);		// Set the dialog font back to the System font
	
	return password;
}

pascal void
ChicagoTextItem(WindowPtr wind,short item)
{	short		fontStore,sizeStore;
	Handle		itemH;
	short		itemType;
	Rect		box;
	char		*text;
	
	SetPort(wind);
	
	fontStore = wind->txFont;	// Remember the current font & size
	sizeStore = wind->txSize;
	
	TextFont(0);		// Set to default System font & size
	TextSize(0);
	
	GetDItem(wind,item,&itemType,&itemH,&box);
	
	if (item == 3)			// These strings would probably be in a resource or somesuch in an actual program.
		text = "\pPlease enter your password:";
	if (item == 4)
		text = "\pSpecial Font Password Dialog";
	
	TextBox(text+1,*text,&box,teJustLeft);	// Draw the prompt
	
	TextFont(fontStore);		// Restore the font & size
	TextSize(sizeStore);
}
****************************************************************/
char *
InternalBufferDialog()
{	static char		password[256];
	DialogPtr		dlog;
	short			item;
	GrafPtr			oldPort;								//-d's addition
	short 			i;										//-d's addition
//	ProcessSerialNumber		tempPSN;						//-d's addition
//	Boolean 				tempBool;						//-d's addition
	
	for( i=0 ; i<256 ; i++ )								//-d's addition
		password[i] = (char) 0;								//-d's addition
	
	SetFrontProcess( &gPSN );								//-d's addition
	
//	tempBool = false;										//-d's addition
//	while( !tempBool )										//-d's addition
//	{
//		GetCurrentProcess( &tempPSN );						//-d's addition
//		SameProcess( &gPSN, &tempPSN, &tempBool );			//-d's addition
//	}
	
	GetPort( &oldPort );									//-d's addition
	dlog = GetNewDialog(kPasswdDLOGid,0L,(WindowPtr) -1L);
	ShowWindow( dlog );										//-d's addition
	SetPort( dlog );										//-d's addition
	
	*password = '\0';					// Zero out the buffered password
	SetWRefCon(dlog,(long)password);	// Stash the buffer's address
	
	do
	{	ModalDialog(InternalBufferFilter,&item);
	} while (item != 1);			// Until the OK button is hit
	
	DisposDialog(dlog);
		
	SetPort( oldPort );										//-d's addition
	
	return password;
}

pascal Boolean
InternalBufferFilter(DialogPtr dlog,EventRecord *event,short *itemHit)
{	char	key;
	short	start,end;
	char	*buffer;
	
	if (event->what != keyDown && event->what != autoKey)
		return false;				// We don't want to deal with them
	
	key = event->message & charCodeMask;
	
	switch (key)
	{	case '\n':			// Return
		case '\r':			// the "return" key!! -- d's addition
		case '\003':		// Enter
			*itemHit = 1;		// OK Button
			return true;		// We handled the event
		case '\t':			// Tab
		case '\034':		// Left arrow
		case '\035':		// Right arrow
		case '\036':		// Up arrow
		case '\037':		// Down arrow
			return false;		// Let ModalDialog handle them
		default:			// Everything else falls through to be dealt with
			break;			//	below
	}
	
	start = (**((DialogPeek)dlog)->textH).selStart;	// Get the current selection
	end = (**((DialogPeek)dlog)->textH).selEnd;
	
	buffer = (char*)GetWRefCon(dlog);		// Get the buffer's address
	
	if (start != end)					// If there's a selection, delete it
		DeleteRange(buffer,start,end);
	
	if (key == '\010')	// Backspace
	{	if (start != 0)
			DeleteRange(buffer,start-1,start);	// Delete the character to the left
	}
	else
	{	InsertChar(buffer,start,key);		// Insert the real key into the buffer
		event->message = '¥';				// Character to use in field
	}
	
	return false; 		// Let ModalDialog insert the fake char
}

void
DeleteRange(char *buffer,short start,short end)
{	register char	*src,*dest,*last;
	
	last = buffer + *buffer;
	
	src = buffer + end + 1;
	dest = buffer + start + 1;
	
	while (src <= last)			// Shift character to the left over the removed characters
		*(dest++) = *(src++);
	
	(*buffer) -= (end-start);	// Adjust the buffer's length
}

void
InsertChar(char *buffer,short pos,char c)
{	register short	index,len;
	
	len = *buffer;
	
	if (len == 0xFF)		// if the string is full, return
		return;
	
	for (index = len;index > pos;index--)	// Shift characters to the right to make room
		buffer[index+1] = buffer[index];
	
	buffer[pos+1] = c;		// Fill in the new character
	
	(*buffer)++;			// Add one to the length of the string
}