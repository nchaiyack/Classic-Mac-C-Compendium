//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CFunctionsArrayPane.cp
//|
//| This implements the scrolling list of functions in the
//| equation window.
//|_________________________________________________________


#include "CFunctionsArrayPane.h"
#include "CHyperCuberPrefs.h"
#include "Parameter.h"

#include <stdio.h>
#include <string.h>


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CFunctionsArrayPane::IFunctionsArrayPane
//|
//| Purpose: Initialize the functions array pane.
//|
//| Parameters: passed to superclass
//|______________________________________________________________________________

void CFunctionsArrayPane::IFunctionsArrayPane(CView *anEnclosure,
							CBureaucrat *aSupervisor,
							short aWidth, short aHeight,
							short aHEncl, short aVEncl,
							SizingOption aHSizing, SizingOption aVSizing)
{

	CArrayPane::IArrayPane(anEnclosure, aSupervisor, aWidth,
							aHeight, aHEncl, aVEncl, aHSizing, aVSizing);

}	//==== CFunctionsArrayPane::IFunctionsArrayPane() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CFunctionsArrayPane::DrawCell
//|
//| Purpose: Draw a cell.  This is the same as the superclass, except that it
//|          explicitly used Chicago font.  There ought to be a better way of
//|          specifying the font, but I didn't find it in my brief look.
//|
//| Parameters: cell: the cell from which to get the text
//|             rect: the cell's boundary rectangle
//|______________________________________________________________________________

void CFunctionsArrayPane::DrawCell(Cell cell, Rect *rect)
{
	Str255	text;
	short	availWidth;
	
	availWidth = rect->right - rect->left - indent.h;
	
	GetCellText(cell, availWidth, text);
	
	if (text[0] > 0)
	{
		MoveTo(rect->left + indent.h, rect->top + indent.v);
		TextFont(systemFont);											//  Use chicago
		DrawString(text);
		TextFont(applFont);												//  Switch back to application font
	}

}	//==== CFunctionsArrayPane::DrawCell() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CFunctionsArrayPane::GetCellText
//|
//| Purpose: Get the text of a cell.
//|
//| Parameters: cell:  the cell from which to get the text
//|             width: width of the cell
//|             text:  receives the text of the cell
//|______________________________________________________________________________

void CFunctionsArrayPane::GetCellText(Cell cell, short availableWidth, StringPtr text)
{

	char function[MAX_FUNCTION_LENGTH];
	itsArray->GetArrayItem(function, cell.v+1);				//  Get the function text

	sprintf((char *) text, "x%d = %s",
							cell.v+1, function);		//  Build the function string as "xi = f"
	CtoPstr((char *) text);

}	//==== CFunctionsArrayPane::GetCellText() ====\\
