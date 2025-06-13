//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CParametersArrayPane.cp
//|
//| This implements the scrolling list of parameters in the
//| equation window.
//|_________________________________________________________


#include "CParametersArrayPane.h"
#include "Parameter.h"

#include <stdio.h>
#include <string.h>


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CParametersArrayPane::IParametersArrayPane
//|
//| Purpose: Initialize the parameters array pane.
//|
//| Parameters: passed to superclass
//|______________________________________________________________________________

void CParametersArrayPane::IParametersArrayPane(CView *anEnclosure,
							CBureaucrat *aSupervisor,
							short aWidth, short aHeight,
							short aHEncl, short aVEncl,
							SizingOption aHSizing, SizingOption aVSizing)
{

	CArrayPane::IArrayPane(anEnclosure, aSupervisor, aWidth,
							aHeight, aHEncl, aVEncl, aHSizing, aVSizing);

}	//==== CParametersArrayPane::IParametersArrayPane() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CParametersArrayPane::DrawCell
//|
//| Purpose: Draw a cell.  This is the same as the superclass, except that it
//|          explicitly used Chicago font.  There ought to be a better way of
//|          specifying the font, but I didn't find it in my brief look.
//|
//| Parameters: cell: the cell from which to get the text
//|             rect: the cell's boundary rectangle
//|______________________________________________________________________________

void CParametersArrayPane::DrawCell(Cell cell, Rect *rect)
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

}	//==== CParametersArrayPane::DrawCell() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CParametersArrayPane::GetCellText
//|
//| Purpose: Get the text of a cell.
//|
//| Parameters: cell:  the cell from which to get the text
//|             width: width of the cell
//|             text:  receives the text of the cell
//|______________________________________________________________________________

void CParametersArrayPane::GetCellText(Cell cell, short availableWidth, StringPtr text)
{

	Parameter parameter;
	itsArray->GetArrayItem(&parameter, cell.v+1);			//  Get the parameter text

	sprintf((char *) text, "%s: [%g, %g] step %g",
							parameter.name,
							parameter.start,
							parameter.end,
							parameter.step);			//  Build the parameter string as
														//   "name: [min, max] step s"
	CtoPstr((char *) text);

}	//==== CParametersArrayPane::GetCellText() ====\\
