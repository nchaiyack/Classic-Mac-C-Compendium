/******************************************************************************
 CBrowserEditText.c
 	Parent class:	CEditText

	Methods to adapt CEditText to support resizeable panes.
	
  	Copyright � 1992 J. Robert Boonstra, II.  All rights reserved.
 ******************************************************************************/


#include "CBrowserEditText.h"


/******************************************************************************
 IBrowserEditText
 ******************************************************************************/

void	CBrowserEditText::IBrowserEditText(
	CView			*anEnclosure,
	CBureaucrat		*aSupervisor,
	short			aWidth,
	short			aHeight,
	short			aHEncl,
	short			aVEncl,
	SizingOption	aHSizing,
	SizingOption	aVSizing,
	short			aLineWidth)
{
	inherited::IEditText(anEnclosure,aSupervisor,aWidth,aHeight,
			aHEncl,aVEncl,aHSizing,aVSizing,aLineWidth);	
}

void	CBrowserEditText::CalcTERects()
{
	LongRect	interior;	
	short		hDelta;
	short		vDelta;
	
	GetInterior( &interior);

	hDelta = position.h * hScale - interior.left + (**macTE).destRect.left;
	vDelta = position.v * vScale - interior.top + (**macTE).destRect.top;
	
	frame.left += hDelta;
	frame.right += hDelta;
	frame.top += vDelta;
// JRB - why isn't the following line in CEditText?
	frame.bottom += vDelta;		
		
	hOrigin += hDelta;
	vOrigin += vDelta;


	(**macTE).destRect.right = (lineWidth > 0) ?
									(**macTE).destRect.left + lineWidth :
									interior.right;
	
// JRB - the itsMessage pane was incorrectly offset without this change.
	OffsetLongRect(&interior, (long)hDelta, (long)vDelta);	
	
	LongToQDRect( &interior, &(**macTE).viewRect);

	TECalText( macTE);
}
