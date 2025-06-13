// Dialog Utilities.c v1.0
//
// by Kenneth Worley
// Public Domain
//
// Contact me at:   America Online: KNEworley
//			internet: KNEworley@aol.com or kworley@fascination.com
//
// Functions to make working with dialog items, controls, and
// text fields (and the same in control panels) easier and
// more convenient.
//

#include "Dialog Utilities.h"

Handle GetItemHandle( DialogPtr dlg, short itemNo )
{
	short			itemType;
	Handle			itemHandle;
	Rect			itemRect;
	
	GetDialogItem( dlg, itemNo, &itemType, &itemHandle, &itemRect );
	
	return itemHandle;
}

Boolean GetOnOff( DialogPtr dlg, short itemNo )
{
	return (Boolean)GetControlValue( (ControlHandle)
		GetItemHandle( dlg, itemNo ) );
}

void SetOnOff( DialogPtr dlg, short itemNo, Boolean turnOn )
{
	SetControlValue( (ControlHandle)
		GetItemHandle( dlg, itemNo ), turnOn );
}

void Toggle( DialogPtr dlg, short itemNo )
{
	SetOnOff( dlg, itemNo, !GetOnOff( dlg, itemNo ) );
}

short GetRadioButton( DialogPtr dlg, short first, short last )
{
	short		aButton;
	short		buttonOn = 0;
	
	// Make sure first < last.
	
	if ( first > last )
		return 0;
		
	// Find which radio button is on.
	
	for ( aButton = first; aButton <= last; aButton++ )
		if ( GetOnOff( dlg, aButton ) )
		{
			buttonOn = aButton;
			break;
		}
	
	return buttonOn;
}

void SetRadioButton( DialogPtr dlg, short first, short last, short target )
{
	short		aButton;
	short		buttonOn = 0;
	
	// Make sure first < last and target is in range.
	
	if ( (first > last) || (target < first) || (target > last) )
		return;
	
	// Find which radio button is on.
	
	for ( aButton = first; aButton <= last; aButton++ )
		if ( GetOnOff( dlg, aButton ) )
		{
			buttonOn = aButton;
			break;
		}
	
	// If none is on, just act like the first one is on.
	
	if ( !buttonOn )
		buttonOn = first;
		
	// Animate the switch to the new target.
	
	if ( target > buttonOn )
	{
		long		ignore;
		
		for ( aButton = buttonOn; aButton < target; aButton++ )
		{
			SetOnOff( dlg, aButton, true );
			Draw1Control( (ControlHandle)GetItemHandle( dlg, aButton ) );
			Delay( 5, &ignore );
			Toggle( dlg, aButton );
			Draw1Control( (ControlHandle)GetItemHandle( dlg, aButton ) );
			Delay( 5, &ignore );
		}
	}
	else
	{
		long		ignore;
		
		for ( aButton = buttonOn; aButton > target; aButton-- )
		{
			SetOnOff( dlg, aButton, true );
			Draw1Control( (ControlHandle)GetItemHandle( dlg, aButton ) );
			Delay( 5, &ignore );
			Toggle( dlg, aButton );
			Draw1Control( (ControlHandle)GetItemHandle( dlg, aButton ) );
			Delay( 5, &ignore );
		}
	}
	
	// Now turn the target on.
	
	SetOnOff( dlg, target, true );
	
	// Make sure no other buttons (other than the target) are on.
	// We only need to check the buttons that weren't between the
	// original button and the target button because the animation
	// would have turned off any excess buttons in that range.
	
	if ( target > buttonOn )
	{
		for ( aButton = target+1; aButton <= last; aButton++ )
			SetOnOff( dlg, aButton, false );
		for ( aButton = buttonOn-1; aButton >= first; aButton-- )
			SetOnOff( dlg, aButton, false );
	}
	else
	{
		for ( aButton = target-1; aButton >= first; aButton-- )
			SetOnOff( dlg, aButton, false );
		for ( aButton = buttonOn+1; aButton <= last; aButton++ )
			SetOnOff( dlg, aButton, false );
	}
}

void SetChars( DialogPtr dlg, short itemNo, char *str, short len )
{
	Str255			aString;
	
	BlockMove( str, &aString[1], len );
	aString[0] = len;
	
	SetDialogItemText( GetItemHandle( dlg, itemNo), aString );
}

void GetChars( DialogPtr dlg, short itemNo, char *str, short *len )
{
	Str255			aString;
	
	GetDialogItemText( GetItemHandle( dlg, itemNo ), aString );
	
	BlockMove( &aString[1], str, aString[0] );
	(*len) = aString[0];
}

void SetText( DialogPtr dlg, short itemNo, Str255 str )
{
	SetDialogItemText( GetItemHandle( dlg, itemNo ), str );
}

void GetText( DialogPtr dlg, short itemNo, Str255 str )
{
	GetDialogItemText( GetItemHandle( dlg, itemNo ), str );
}

void ActivateControl( DialogPtr dlg, short itemNo, Boolean activate )
{
	if ( activate )
		HiliteControl( (ControlHandle)
			GetItemHandle( dlg, itemNo ), 0 );
	else
		HiliteControl( (ControlHandle)
			GetItemHandle( dlg, itemNo ), 255 );		// disable
}

