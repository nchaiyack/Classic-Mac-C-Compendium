/*************************************************************************************

 CEditString.c
	
		Class for editing strings stored in the CStringArray class.
	
	SUPERCLASS = CDialogText
	
		� 1992 Dave Harkness

*************************************************************************************/


#include "CEditString.h"
#include "CStringEditList.h"
#include <Global.h>
#include <Constants.h>


extern CBureaucrat		*gGopher;


/*************************************************************************************
 IEditString

	Initialize the CDialogText and then do our own.
*************************************************************************************/

void
CEditString::IEditString( CView *anEnclosure, CView *aSupervisor,
						  short aWidth, short aHeight,
						  short aHEncl, short aVEncl,
						  SizingOption aHSizing, SizingOption aVSizing,
						  short aLineWidth)

{
	CDialogText::IDialogText( anEnclosure, aSupervisor, aWidth, aHeight,
							  aHEncl, aVEncl, aHSizing, aVSizing, aLineWidth);
	
	SetFontNumber( geneva);
	SetFontSize( 9);
	SetConstraints( FALSE, 255);
	validateOnResign = FALSE;							// no validation

}  /* CEditString::IEditString */


/*************************************************************************************
 MakeBorder  {OVERRIDE}

	We want our border to only have a one pixel margin.
*************************************************************************************/

void
CEditString::MakeBorder( void)

{
	SetBorder( NULL);

}  /* CEditString::MakeBorder */


/*************************************************************************************
 DoKeyDown  {OVERRIDE}

	Traps the clear key (for deleting the cell.
*************************************************************************************/

void
CEditString::DoKeyDown( char theChar, Byte keyCode, EventRecord *macEvent)
{
	if ( theChar == kEscapeOrClear )
		itsSupervisor->DoKeyDown( theChar, keyCode, macEvent);
	else
		inherited::DoKeyDown( theChar, keyCode, macEvent);

}  /* CEditString::DoKeyDown */


/*************************************************************************************
 BecomeGopher  {OVERRIDE}

	If we are resigning as gopher, broadcast that we are done editing.
*************************************************************************************/

Boolean
CEditString::BecomeGopher( Boolean fBecoming)

{
	if ( fBecoming && !visible )
		return itsSupervisor->BecomeGopher( TRUE);
	
	if ( !inherited::BecomeGopher( fBecoming) )
		return FALSE;
	
	if ( !fBecoming )
		BroadcastChange( editStringDoneEditing, NULL);
	
	return TRUE;

}  /* CEditString::BecomeGopher */
