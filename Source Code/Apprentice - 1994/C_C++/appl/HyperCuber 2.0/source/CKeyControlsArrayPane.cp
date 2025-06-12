//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CKeyControlsArrayPane.cp
//|
//| This implements the scrolling list in the key controls
//| window.
//|_________________________________________________________


#include "CKeyControlsArrayPane.h"
#include "Keys.h"

#include <stdio.h>
#include <string.h>


//======================== Prototypes ======================\\

extern void DrawKey(short key_code, short modifiers, short h, short v);



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CKeyControlsArrayPane::IKeyControlsArrayPane
//|
//| Purpose: Initialize the key controls array pane.
//|
//| Parameters: passed to superclass
//|______________________________________________________________________________

void CKeyControlsArrayPane::IKeyControlsArrayPane(CView *anEnclosure,
							CBureaucrat *aSupervisor,
							short aWidth, short aHeight,
							short aHEncl, short aVEncl,
							SizingOption aHSizing, SizingOption aVSizing)
{

	CArrayPane::IArrayPane(anEnclosure, aSupervisor, aWidth,
							aHeight, aHEncl, aVEncl, aHSizing, aVSizing);

}	//==== CKeyControlsArrayPane::IKeyControlsArrayPane() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CKeyControlsArrayPane::DrawCell
//|
//| Purpose: Initialize the about dialog.
//|
//| Parameters: cell: the cell to draw
//|             rect: rectangle of the cell
//|______________________________________________________________________________

void CKeyControlsArrayPane::DrawCell(Cell cell, Rect *rect)
{

	key_control_struct key;
	itsArray->GetItem(&key, cell.v+1);			//  Get the key control

	char table_string[100];

	char angle_string[5];
	if (key.angle == 0)
		strcpy(angle_string, "P");
	else
		sprintf(angle_string, "%d", key.angle);

	sprintf(table_string, "%s [%d:%s] by %d",	//  Build the description string
			(key.increment >= 0) ?
						"Increase" : "Decrease",
			key.dimension, angle_string,
			(key.increment >= 0) ?
				key.increment : -key.increment);
	
	TextFont(systemFont);						//  Set font to Chicago
	MoveTo(rect->left+3, rect->bottom-5);
	DrawString(CtoPstr(table_string));			//  Display the description string

	DrawKey(key.key_code, key.modifiers,
				220, rect->bottom-5);			//  Draw the key

	TextFont(applFont);							//  Switch back to application font

}	//==== CKeyControlsArrayPane::DrawCell() ====\\
