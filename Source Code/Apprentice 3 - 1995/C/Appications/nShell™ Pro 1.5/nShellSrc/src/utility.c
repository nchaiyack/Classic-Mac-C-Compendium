/* ==========================================

	utility.c
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

#include <Traps.h>
#include <GestaltEqu.h>

#include "multi.h"
#include "resource.h"

#include "pipe.proto.h"
#include "str_utl.proto.h"
#include "utility.proto.h"

int TestResError( void )
{
	short error;
	
	error = ResError();
	
	if (error)
		error_note("\presource operation");
	
	return(error);
}

int	TestGestaltBit( OSType selector, int bit )
{
	OSErr	error;
	long	response;
	
	if ( error = Gestalt( selector, &response ) )
		return(false);
		
	if ( response & ( 1L << bit ) )
		return(true);
	else
		return(false);
}

void SimpleModal(int ID)
{
	DialogPtr	theDialog;
	short		itemHit;
	GrafPtr		SavPort;
	
	GetPort(&SavPort);
	
	theDialog = GetNewDialog( ID, NULL, (WindowPtr) -1 );

	if (theDialog) {
		SetCursor(&arrow);
		SetPort( theDialog );
		ModalDialog( NULL, &itemHit );
		DisposDialog( theDialog );
		}
	else
		error_note("\pmissing dialog resource");
	
	SetPort( SavPort );
}

void outline_item(DialogPtr theDialog, short theItem)
{
	int			radius;
	short		itemKind;
	Rect		itemRect;
	Handle		itemHandle;

	GetDItem( theDialog, theItem, &itemKind, &itemHandle, &itemRect );
	InsetRect( &itemRect, -4, -4 );
	radius = ( itemRect.bottom - itemRect.top ) / 2;
	if ( radius > 16 ) radius = 16;
	PenNormal();
	PenSize( 3, 3);
	FrameRoundRect( &itemRect, radius, radius );
}

void myModal(int ID)
{
	#define		ITEM_OK		1
	
	DialogPtr	theDialog;
	GrafPtr		SavPort;
	short		itemHit;
	
	GetPort(&SavPort);
	
	theDialog = GetNewDialog( ID, NULL, NULL );
	
	if (theDialog) {
		SetCursor(&arrow);
		SetPort( theDialog );
		outline_item(theDialog, ITEM_OK);
		do
			ModalDialog( NULL, &itemHit );
		while (itemHit != ITEM_OK);
		DisposDialog( theDialog );
		}
	else
		error_note("\pmissing dialog resource");
	
	SetPort( SavPort );
}

void error_note(Str255 what)
{
	ParamText("\pERROR - ", what, "\p.", "\p");
	SetCursor(&arrow);
	Alert(ErrorNote, 0L);
}

int Ask(Str255 s, int size)
{
	int	AlertID;
	
	switch (size) {
		case 0 :
			AlertID = AskSmall;
			break;
		case 2 :
			AlertID = AskLarge;
			break;
		default :
			AlertID = AskMedium;
			break;
		}
		
	ParamText(s, "\p", "\p", "\p");
	SetCursor(&arrow);
	return (Alert(AlertID, 0L));
}

void Notify(Str255 s, int size)
{
	int	AlertID;
	
	switch (size) {
		case 0 :
			AlertID = NotifySmall;
			break;
		case 2 :
			AlertID = NotifyLarge;
			break;
		default :
			AlertID = NotifyMedium;
			break;
		}
		
	ParamText(s, "\p", "\p", "\p");
	SetCursor(&arrow);
	Alert(AlertID, 0L);
}

