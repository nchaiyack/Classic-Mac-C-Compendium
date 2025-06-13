/*************************************************************************************

 CStringEditList.c
	
		A ScrollList that allows array element strings to be edited.
	
	SUPERCLASS = CScrollList
	
		© 1992 Dave Harkness

*************************************************************************************/


#include "CStringEditList.h"
#include "CStringArray.h"


#define kCheckWidth			9	// room for the check mark


extern CBureaucrat		*gGopher;


/*************************************************************************************
 IStringEditList
*************************************************************************************/

void
CStringEditList::IStringEditList( CView *anEnclosure, CBureaucrat *aSupervisor,
								  short aWidth, short aHeight,
								  short aHEncl, short aVEncl,
								  SizingOption aHSizing, SizingOption aVSizing,
								  short fOptions)

{
	CScrollList::IScrollList( anEnclosure, aSupervisor, aWidth, aHeight,
							  aHEncl, aVEncl, aHSizing, aVSizing, fOptions);
	
	
	itsEditString = new CEditString;
	itsEditString->IEditString( this, this, 0, 0, 0, 0,
								sizFIXEDSTICKY, sizFIXEDSTICKY, -1);
	itsEditString->Hide();

}  /* CStringEditList::IStringEditList */


/*************************************************************************************
 DoInsertCell  {OVERRIDE}

	Create a new string in our array.
*************************************************************************************/

void
CStringEditList::DoInsertCell( short beforeCell)

{
	Str255		str;
	
	str[0] = 0;
	
	itsArray->InsertAtIndex( str, beforeCell);

}  /* CStringEditList::DoInsertCell */


/*************************************************************************************
 BeginEditing  {OVERRIDE}

	Create the CEditString pane.  If the editString hasn't been sized yet, it sets
	its size to the correct values.
*************************************************************************************/

void
CStringEditList::BeginEditing( void)

{
	Rect		tempRect;
	
	inherited::BeginEditing();
	
	SetRect( &tempRect, 0, 0, width - (2 * indent.h) + 2
			 + (kCheckWidth * ((listOptions & kSLCheckable) != 0)) - itsEditString->width,
			 cellHeight - 1 - itsEditString->height);
	itsEditString->ChangeSize( &tempRect, kNoRedraw);
	itsEditString->Place( indent.h - 1, cellHeight * (selectedCell - 1) + 1, kDontRedraw);
	itsEditString->Show();
	itsEditString->BecomeGopher( TRUE);
	itsEditString->SelectAll( TRUE);

}  /* CStringEditList::BeginEditing */


/*************************************************************************************
 SetupCellData  {OVERRIDE}

	Place the string from the StringArray into the edit pane.
*************************************************************************************/

void
CStringEditList::SetupCellData( void)

{
	Str255		str;
	
	((CStringArray *)itsArray)->GetItem( str, selectedCell);
	itsEditString->SetTextString( (StringPtr)str);

}  /* CStringEditList::SetupCellData */


/*************************************************************************************
 RetrieveCellData  {OVERRIDE}

	Take the new string from the edit pane and store it into the StringArray.
*************************************************************************************/

void
CStringEditList::RetrieveCellData( void)

{
	Str255		str;
	
	itsEditString->GetTextString( (StringPtr)str);
	((CStringArray *)itsArray)->SetItem( str, selectedCell);

}  /* CStringEditList::RetrieveCellData */


/*************************************************************************************
 DoneEditing  {OVERRIDE}

	Remove the CEditString pane.
*************************************************************************************/

void
CStringEditList::DoneEditing( void)

{
	itsEditString->Hide();
	inherited::DoneEditing();						// Now retrieve data

}  /* CStringEditList::DoneEditing */


/*************************************************************************************
 DrawCell  {OVERRIDE}

	Draw the string into the specified cell rect.
*************************************************************************************/

void
CStringEditList::DrawCell( short theCell, Rect *cellRect)

{
	Str255	cellText;
	
	inherited::DrawCell( theCell, cellRect);
	
	((CStringArray *)itsArray)->GetItem( cellText, theCell);
	
	if (cellText[0] > 0)
	{
		MoveTo( cellRect->left + indent.h, cellRect->top + indent.v);
		DrawString( cellText);
	}

}  /* CStringEditList::DrawCell */


/*************************************************************************************
 ProviderChanged  {OVERRIDE}

	If our editString is done, call DoneEditing.
*************************************************************************************/

void
CStringEditList::ProviderChanged( CCollaborator *aProvider, long reason, void* info)

{
	if ( reason == editStringDoneEditing && aProvider == itsEditString &&
				gGopher != itsEditString && fEditing )
		DoneEditing();
	else
		inherited::ProviderChanged( aProvider, reason, info);

}  /* CStringEditList::ProviderChanged */
