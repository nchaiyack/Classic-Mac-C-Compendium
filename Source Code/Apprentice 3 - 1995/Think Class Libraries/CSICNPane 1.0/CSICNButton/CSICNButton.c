/******************************************************************************
 CSICNButton.c

	Creates a pane containing a SICN. The pane can act as a button simply by
	setting SetClickCmd() to something other then cmdNull.
	
	BROWN UNIVERSITY AND ANDREW JAMES GILMARTIN GIVE NO WARRANTY, EITHER
	EXPRESS OR IMPLIED, FOR THE PROGRAM AND/OR DOCUMENTATION PROVIDED,
	INCLUDING, WITHOUT LIMITATION, WARRANTY OF MERCHANTABILITY AND
	WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE.

	AUTHOR: Andrew_Gilmartin@Brown.Edu
	MODIFIED: 93-04-14

******************************************************************************/

//#include <TCLHeaders>
#include "CSICNButton.h"
#include "TrackMouseClick.h"



/******************************************************************************
 ISICNButton

	Initialize CSICNButton. Pane does not act like a button by default.
******************************************************************************/

void CSICNButton::ISICNButton
	( CView *anEnclosure
	, CBureaucrat *aSupervisor
	, short aHEncl
	, short aVEncl
	, SizingOption aHSizing
	, SizingOption aVSizing
	, short SICNid
	, short SICNindex )
{
	ISICNPane
		( anEnclosure
		, aSupervisor
		, aHEncl
		, aVEncl
		, aHSizing
		, aVSizing
		, SICNid
		, SICNindex );
		
	SetClickCmd( cmdNull );
	
} /* ISICNButton */



/******************************************************************************
 DoClick

	When acting as a button, track mouse. Call DoGoodClick() if successful.
******************************************************************************/

void CSICNButton::DoClick( Point hitPt, short modifierKeys, long when )
{
	Rect theFrame;

	LongToQDRect( &frame, &theFrame );
	
	if ( TrackMouseClick( &theFrame, FALSE /* Don't flash */ ) )
		DoGoodClick( 0 );

} /* DoClick */



/******************************************************************************
 DoGoodClick

	Send the command to the supervisor
******************************************************************************/

void CSICNButton::DoGoodClick( short whichPart )
{
	itsSupervisor->DoCommand( fCommand );

} /* DoGoodClick */



/******************************************************************************
 SetClickCmd

	Set the command to be sent upon a good click.
******************************************************************************/

void CSICNButton::SetClickCmd( long command )
{
	fCommand = command;

	SetWantsClicks( fCommand != cmdNull );

} /* SetClickCmd */



/******************************************************************************
 GetClickCmd

	Get the current good click command.
******************************************************************************/

long CSICNButton::GetClickCmd( void )
{
	return fCommand;

} /* GetClickCmd */

