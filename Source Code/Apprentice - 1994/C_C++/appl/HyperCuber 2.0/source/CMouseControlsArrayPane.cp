//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CMouseControlsArrayPane.cp
//|
//| This implements the scrolling list in the mouse controls
//| window.
//|_________________________________________________________


#include "CMouseControlsArrayPane.h"
#include "Mouse.h"

#include <stdio.h>
#include <string.h>


//======================== Prototypes ======================\\

extern void DrawModifiers(short modifiers, short h, short v);



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CMouseControlsArrayPane::IMouseControlsArrayPane
//|
//| Purpose: Initialize the mouse controls array pane.
//|
//| Parameters: passed to superclass
//|______________________________________________________________________________

void CMouseControlsArrayPane::IMouseControlsArrayPane(CView *anEnclosure,
							CBureaucrat *aSupervisor,
							short aWidth, short aHeight,
							short aHEncl, short aVEncl,
							SizingOption aHSizing, SizingOption aVSizing)
{

	CArrayPane::IArrayPane(anEnclosure, aSupervisor, aWidth,
							aHeight, aHEncl, aVEncl, aHSizing, aVSizing);

}	//==== CMouseControlsArrayPane::IMouseControlsArrayPane() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CMouseControlsArrayPane::DrawCell
//|
//| Purpose: Initialize the about dialog.
//|
//| Parameters: cell: the cell to draw
//|             rect: rectangle of the cell
//|______________________________________________________________________________

void CMouseControlsArrayPane::DrawCell(Cell cell, Rect *rect)
{

	mouse_control_struct mouse_control;
	itsArray->GetItem(&mouse_control, cell.v+1);		//  Get the mouse_control control

	char table_string[100];

	char angle_string[5];
	if (mouse_control.angle == 0)						//  Build the angle string
		strcpy(angle_string, "P");
	else
		sprintf(angle_string, "%d", mouse_control.angle);

	sprintf(table_string, "Track [%d:%s] %s",			//  Build the description string
			mouse_control.dimension, angle_string,
			mouse_control.horiz ?
					"horizontally" : "vertically");
	
	TextFont(systemFont);								//  Set font to Chicago
	MoveTo(rect->left+3, rect->bottom-5);
	DrawString(CtoPstr(table_string));					//  Display the description string

	char multiplier_string[10];							//  Draw the multiplier string
	if (mouse_control.multiplier != 1)
		{
		DrawString("\p (");
		Move(2, -2);
		TextFont(monaco);
		TextSize(9);
		DrawChar('x');
		Move(2, 2);
		TextFont(systemFont);
		TextSize(12);
		sprintf(multiplier_string, "%d)",
					mouse_control.multiplier);
		DrawString(CtoPstr(multiplier_string)
		);
		}

	if (mouse_control.modifiers)	
		DrawModifiers(mouse_control.modifiers, 280,
									rect->bottom-14);	//  Draw the modifiers
	else
		{
		MoveTo(250, rect->bottom-5);					//  No modifiers; draw "none"
		TextFont(systemFont);
		DrawString("\pnone");
		TextFont(applFont);
		}

	TextFont(applFont);									//  Switch back to application font

}	//==== CMouseControlsArrayPane::DrawCell() ====\\
