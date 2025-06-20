/******************************************************************************
 CSICNPane.c

	Creates a pane containing a SICN. See class CSizeBox for code to show/hide
	the sicn on activation/deactivation. (NOTE: I think there is an update bug
	in the code, but you will have to work this out.)
	
	BROWN UNIVERSITY AND ANDREW JAMES GILMARTIN GIVE NO WARRANTY, EITHER
	EXPRESS OR IMPLIED, FOR THE PROGRAM AND/OR DOCUMENTATION PROVIDED,
	INCLUDING, WITHOUT LIMITATION, WARRANTY OF MERCHANTABILITY AND
	WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE.

	AUTHOR: Andrew_Gilmartin@Brown.Edu
	MODIFIED: 93-04-14

******************************************************************************/

//#include <TCLHeaders>
#include "CSICNPane.h"



/******************************************************************************
 ISICNPane

	Initialize CSICNPane. Pane does not act like a button by default.
******************************************************************************/

void CSICNPane::ISICNPane
	( CView *anEnclosure
	, CBureaucrat *aSupervisor
	, short aHEncl
	, short aVEncl
	, SizingOption aHSizing
	, SizingOption aVSizing
	, short SICNid
	, short SICNindex )
{
	LongRect enclFrame;
	
	anEnclosure->GetFrame( &enclFrame );
	
	CPane::IPane
		( anEnclosure
		, aSupervisor
		, kSICNWidth, kSICNHeight
		, aHEncl, aVEncl
		, aHSizing, aVSizing );
	
	fId = SICNid;
	fIndex = SICNindex;
	
} /* ISICNPane */



/******************************************************************************
 Draw

	Draw the SICN.
******************************************************************************/

void CSICNPane::Draw( Rect *area )
{
	Point thePoint;

	LongToQDPt( &topLeftL( frame ), &thePoint );
	DrawSICN( fId, fIndex, thePoint );

} /* Draw */
