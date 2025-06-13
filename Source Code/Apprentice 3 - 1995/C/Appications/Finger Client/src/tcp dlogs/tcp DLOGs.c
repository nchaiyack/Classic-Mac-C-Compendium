// tcp DLOGs.c
// Darrell Anderson

#include "tcp DLOGs.h"

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

// an event filter, used to replace characters with ¥'s.
// Not in the header file because it is _not_ to be used outside this code set.
pascal Boolean TwoItemFilter(DialogPtr dlog,EventRecord *event,short *itemHit);

// query the user for a hostname and a port number.
// takes a HostNPort struct and fills it in with entered data.
// returns 1 if the user hit OK, 0 if Cancel
int QueryHostNPort( HostNPort *data ) {
	DialogPtr dlog;
	Handle itemH;
	short item,itemType;
	Rect box;
	GrafPtr oldPort;
	Str255 portStr;
	long portNum;
		
	// get the dialog and swap ports
	dlog = GetNewDialog(kHostNPortRSRCid,0L,(WindowPtr) -1L);
	ShowWindow(dlog);
	GetPort(&oldPort);
	SetPort(dlog);

	SetDialogDefaultItem( dlog, ok );
	SetDialogCancelItem( dlog, cancel );
	SetDialogTracksCursor( dlog, true );
	
	// if the hostNport are not nil, use them as the defaults
	if( data->hostname ) {
		GetDItem( dlog, kHost, &itemType, &itemH, &box );
		CtoPstr(data->hostname);
		SetIText( itemH, *(Str255 *)data->hostname );
		PtoCstr(*(Str255 *)data->hostname);
	}
	if( data->port ) {
		NumToString((long)data->port, portStr);
		GetDItem( dlog, kPort, &itemType, &itemH, &box );
		SetIText( itemH, portStr );
	}
	
	// select the first field
	SelIText(dlog,kHost,0,255);
		
	// loop until 'OK' or 'Cancel' is hit, dealing with events in the interim
	do {	
		ModalDialog(0L,&item);
	} while ((item != 1) && (item != 2));

	// get the hostname text
	GetDItem(dlog,kHost,&itemType,&itemH,&box);	
	GetIText(itemH,*(Str255 *)data->hostname);
	PtoCstr(*(Str255 *)data->hostname);
	
	// get the port text
	GetDItem(dlog,kPort,&itemType,&itemH,&box);	
	GetIText(itemH,portStr);
	StringToNum(portStr,&portNum);
	data->port = portNum;
	
	// dispose of the dialog box, restore the port
	HideWindow(dlog);
	SetPort(oldPort);
	DisposDialog(dlog);

	// return true for ok, false otherwise
	return( item == 1 );
}

// query the user for a username and password.
// note that the password is displayed using '¥'s, and not cleartext.
// takes a UserNPass struct, and fills it in with the entered data.
// returns 1 if the user hit OK, 0 if Cancel.
int QueryUserNPass( UserNPass *data ) {	
	DialogPtr dlog;
	Handle itemH;
	short item,itemType;
	Rect box;
	GrafPtr oldPort;

	// get the dialog and swap ports
	dlog = GetNewDialog(kUserNPassRSRCid,0L,(WindowPtr) -1L);
	ShowWindow(dlog);
	GetPort(&oldPort);
	SetPort(dlog);
	
	// draw a rounded rectangle around the ok button (can't use standard procs
	// because we aren't using the normal filter proc)
	GetDItem(dlog,1,&itemType,&itemH,&box);
	PenSize(3,3);
	InsetRect(&box,-4,-4);
	FrameRoundRect(&box,16,16);
	
	// if the userNpass are not nil, use them as the defaults 
	// Sorry, can't default password..
	if( data->username ) {
		GetDItem( dlog, kUsername, &itemType, &itemH, &box );
		CtoPstr(data->username);
		SetIText( itemH, *(Str255 *)data->username );
		PtoCstr(*(Str255 *)data->username);
	}
	
	// select the first field
	SelIText(dlog,kUsername,0,255);
	
	// loop until 'OK' or 'Cancel' is hit, dealing with events in the interim
	// filtering if necessary to prevent the password from showing up on the screen
	do {	
		ModalDialog(TwoItemFilter,&item);
	} while ((item != 1) && (item != 2));

	// get the username text
	GetDItem(dlog,kUsername,&itemType,&itemH,&box);	
	GetIText(itemH,*(Str255 *)data->username);
	PtoCstr(*(Str255 *)data->username);
	
	// get the (hidden) password text
	GetDItem(dlog,kInvis,&itemType,&itemH,&box);	
	GetIText(itemH,*(Str255 *)data->password);
	PtoCstr(*(Str255 *)data->password);
	
	// dispose of the dialog box, restore the port	
	HideWindow(dlog);
	SetPort(oldPort);
	DisposDialog(dlog);

	// return true for ok, false otherwise
	return( item == 1 );
}

pascal Boolean TwoItemFilter(DialogPtr dlog,EventRecord *event,short *itemHit) {
// thanks to Tim Dierks, UK Mac DTS August, 1991 article on password filters..
// this code is loosely based around his suggestions.
	DialogPtr evtDlog;
	short selStart,selEnd;
		
	// if it isn't the password field, disregard everything except a 'return' key event,
	// which is treated as it we hit 'ok'
	if( ((DialogPeek)dlog)->editField != kVis-1 ) {
		if (event->what == keyDown || event->what == autoKey)
		{	switch (event->message & charCodeMask)
			{	case '\n':			// Return  (hitting return or enter is the same as hitting the OK button)
				case '\r':			// the 'return' key!
				case '\003':		// Enter
					*itemHit = 1;	// simulate hitting the OK Button
					return(true);	// We handled the event
			}
		}
		return(false);
	}
	
	if (event->what == keyDown || event->what == autoKey)
	{	switch (event->message & charCodeMask)
		{	case '\n':			// Return  (hitting return or enter is the same as hitting the OK button)
			case '\r':			// the 'return' key!
			case '\003':		// Enter
				*itemHit = 1;	// simulate hitting the OK Button
				return(true);	// We handled the event
			case '\t':			// Tab
				event->what = nullEvent; // Do nothing (don't let the user tab to the hidden field)
				return(false);
			case '\034':		// Left arrow  (Keys that just change the selection)
			case '\035':		// Right arrow
			case '\036':		// Up arrow
			case '\037':		// Down arrow
				return(false);	// Let ModalDialog handle them
			default:
				selStart = (**((DialogPeek)dlog)->textH).selStart; // Get the selection in the visible item
				selEnd = (**((DialogPeek)dlog)->textH).selEnd;
				SelIText(dlog,kInvis,selStart,selEnd);			// Select text in invisible item
				DialogSelect(event,&evtDlog,itemHit);			// Input key
				SelIText(dlog,kVis,selStart,selEnd);			// Select same area in visible item
				if ((event->message & charCodeMask) != '\010')	// If it's not a backspace (backspace is the only key that can affect both the text and the selection- thus we need to process it in both fields, but not change it for the hidden field.
					event->message = '¥';						// Replace with character to use
				DialogSelect(event,&evtDlog,itemHit);			// Put in fake character
				return(true);
		}
	}
	// if we haven't returned yet, it wasn't a keydown..
	return(false); 
}

int QueryUserNHost( UserNPass *user, HostNPort *host ) {
	DialogPtr dlog;
	Handle itemH;
	short item,itemType;
	Rect box;
	GrafPtr oldPort;
		
	// get the dialog and swap ports
	dlog = GetNewDialog(kUserNHostRSRCid,0L,(WindowPtr) -1L);
	ShowWindow(dlog);
	GetPort(&oldPort);
	SetPort(dlog);

	SetDialogDefaultItem( dlog, ok );
	SetDialogCancelItem( dlog, cancel );
	SetDialogTracksCursor( dlog, true );
	
	// if the supplied values are not nil, use them as the defaults
	if( host->hostname ) {
		GetDItem( dlog, kUNHhost, &itemType, &itemH, &box );
		CtoPstr(host->hostname);
		SetIText( itemH, *(Str255 *)host->hostname );
		PtoCstr(*(Str255 *)host->hostname);
	}
	if( user->username ) {
		GetDItem( dlog, kUNHuser, &itemType, &itemH, &box );
		CtoPstr(user->username);
		SetIText( itemH, *(Str255 *)user->username );
		PtoCstr(*(Str255 *)user->username);
	}
	
	// select the first field
	SelIText(dlog,kUNHuser,0,255);
		
	// loop until 'OK' or 'Cancel' is hit, dealing with events in the interim
	do {	
		ModalDialog(0L,&item);
	} while ((item != 1) && (item != 2));

	// get the hostname text
	GetDItem(dlog,kUNHhost,&itemType,&itemH,&box);	
	GetIText(itemH,*(Str255 *)host->hostname);
	PtoCstr(*(Str255 *)host->hostname);
	
	// get the username text
	GetDItem(dlog,kUNHuser,&itemType,&itemH,&box);	
	GetIText(itemH,*(Str255 *)user->username);
	PtoCstr(*(Str255 *)user->username);

	
	// dispose of the dialog box, restore the port
	HideWindow(dlog);
	SetPort(oldPort);
	DisposDialog(dlog);

	// return true for ok, false otherwise
	return( item == 1 );
}

// display a "sorry you messed up" dialog, 
// returns 1 if the user wants to retry, 0 on Cancel.
int QueryFailedAttempt( void ) {
	DialogPtr dlog;
	short item;
	GrafPtr oldPort;
	
	// get the dialog and swap ports
	dlog = GetNewDialog(kUhOhRSRCid,0L,(WindowPtr) -1L);
	ShowWindow(dlog);
	GetPort(&oldPort);
	SetPort(dlog);

	SetDialogDefaultItem( dlog, ok );
	SetDialogCancelItem( dlog, cancel );
	SetDialogTracksCursor( dlog, true );

	// loop until 'OK' or 'Cancel' is hit, dealing with events in the interim
	do {	
		ModalDialog(0L,&item);
	} while ((item != 1) && (item != 2));
	
	// dispose of the dialog box, restore the port
	HideWindow(dlog);
	SetPort(oldPort);
	DisposDialog(dlog);

	// return true for ok, false otherwise
	return( item == 1 );
}