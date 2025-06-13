/******************************************************************************
 TrackMouseClick.c

	BROWN UNIVERSITY AND ANDREW JAMES GILMARTIN GIVE NO WARRANTY, EITHER
	EXPRESS OR IMPLIED, FOR THE PROGRAM AND/OR DOCUMENTATION PROVIDED,
	INCLUDING, WITHOUT LIMITATION, WARRANTY OF MERCHANTABILITY AND
	WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE.

	AUTHOR: Andrew_Gilmartin@Brown.Edu
	MODIFIED: 93-04-14

******************************************************************************/

#include "TrackMouseClick.h"



/******************************************************************************
 TrackMouseClick

	A utilitiy routine to track the mouse within a rect. I the mouse is in
	the rect then the rect is inverted. If flash is true when the user
	mouse ups in the rect the rect is inverted MenuFlash times.
******************************************************************************/

Boolean TrackMouseClick( Rect* rect, Boolean flash )
{
	Point where;
	short i;
	long finalTick;
	Boolean inverted;
	
		/* NOTE: TCL users should first "LongToQDRect( &frame, &rect );" */
	
	InsetRect( rect, 1, 1 );

		/* Track mouse and invert rect as needed */
		
	inverted = FALSE;
	
	do
	{
		GetMouse( &where );
		
		if ( PtInRect( where, rect ) )
		{
			if ( ! inverted )
			{
				InvertRect( rect );
				inverted = TRUE;
			}
		}
		else
		{
			if ( inverted )
			{
				InvertRect( rect );
				inverted = FALSE;
			}
		}

	} while ( Button() );

		/* Did user mouse up outside of rect? */
		
	if ( ! inverted )
		return FALSE;
		
		/* Otherwise, flash rect */
		
	SetHiliteMode(); /* invert with user's hilite color */
	InvertRect( rect ); /* off */

	if ( flash )
		for ( i = MenuFlash; i > 0; i-- )
		{
			Delay( 4, &finalTick );
			SetHiliteMode();
			InvertRect( rect ); /* on */
			Delay( 4, &finalTick );
			SetHiliteMode();
			InvertRect( rect ); /* off */
		}

	return TRUE;
	
} /* TrackMouseClick */