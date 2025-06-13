/* Cell Proj 1.0 */

#include "Cell_Proto.h"
#include "Cell_Definitions.h"
#include "Cell_Variables.h"

static	char	lifeSum[ NUMBER_OF_CELLS ];

/* count the number of cells with life adjoining each cell */
static void SumCells(void) {
	register int row,column;

	memset(lifeSum,0,sizeof(lifeSum));	// zero out the sums

	for(row=0;row<CELL_HEIGHT;row++) {
		register int rowOffset = row * CELL_WIDTH;
		register int prevRowOffset = (row == 0) ?
			(CELL_HEIGHT-1) * CELL_WIDTH :
			(row-1) * CELL_WIDTH;
		register int nextRowOffset = (row == CELL_HEIGHT - 1) ?
			0 : (row + 1) * CELL_WIDTH;

		// handle the column = 0 case, so we don't do the if as often -DaveP
		{
			lifeSum[rowOffset] += gCellStatus[prevRowOffset-1];
			lifeSum[rowOffset] += gCellStatus[prevRowOffset];
			lifeSum[rowOffset] += gCellStatus[prevRowOffset+1];
			if (rowOffset > 0)
				lifeSum[rowOffset] += gCellStatus[rowOffset-1];
			else
				lifeSum[rowOffset] += gCellStatus[NUMBER_OF_CELLS-1];
			lifeSum[rowOffset] += gCellStatus[rowOffset+1];
			lifeSum[rowOffset] += gCellStatus[nextRowOffset-1];
			lifeSum[rowOffset] += gCellStatus[nextRowOffset];
			lifeSum[rowOffset] += gCellStatus[nextRowOffset+1];
		}

		for(column=1;column<CELL_WIDTH - 1;column++) {
			lifeSum[rowOffset+column] += gCellStatus[prevRowOffset+column-1];
			lifeSum[rowOffset+column] += gCellStatus[prevRowOffset+column];
			lifeSum[rowOffset+column] += gCellStatus[prevRowOffset+column+1];
			lifeSum[rowOffset+column] += gCellStatus[rowOffset+column-1];
			lifeSum[rowOffset+column] += gCellStatus[rowOffset+column+1];
			lifeSum[rowOffset+column] += gCellStatus[nextRowOffset+column-1];
			lifeSum[rowOffset+column] += gCellStatus[nextRowOffset+column];
			lifeSum[rowOffset+column] += gCellStatus[nextRowOffset+column+1];
		}
		
		// handle the column = (CELL_WIDTH - 1) case,
		// so we don't do the if as often -DaveP
		{
			column = CELL_WIDTH - 1;
			lifeSum[rowOffset+CELL_WIDTH - 1] += gCellStatus[prevRowOffset+CELL_WIDTH - 2];
			lifeSum[rowOffset+CELL_WIDTH - 1] += gCellStatus[prevRowOffset+CELL_WIDTH - 1];
			lifeSum[rowOffset+CELL_WIDTH - 1] += gCellStatus[prevRowOffset+CELL_WIDTH];
			lifeSum[rowOffset+CELL_WIDTH - 1] += gCellStatus[rowOffset+CELL_WIDTH - 2];
			if (rowOffset+CELL_WIDTH - 1 < (NUMBER_OF_CELLS-1))
				lifeSum[rowOffset+CELL_WIDTH - 1] += gCellStatus[rowOffset+CELL_WIDTH];
			else
				lifeSum[rowOffset+CELL_WIDTH - 1] += gCellStatus[0];
			lifeSum[rowOffset+CELL_WIDTH - 1] += gCellStatus[nextRowOffset+CELL_WIDTH - 2];
			lifeSum[rowOffset+CELL_WIDTH - 1] += gCellStatus[nextRowOffset+CELL_WIDTH - 1];
			lifeSum[rowOffset+CELL_WIDTH - 1] += gCellStatus[nextRowOffset+CELL_WIDTH];
		}
	}
}

/* encodes the rules for the cellular tomato */
static void CalcResults(void) {
	register int count;
	for ( count = 0; count < NUMBER_OF_CELLS; count++ )	{
		switch(lifeSum[count]) {
			case 2:	/* gCellStatus doesn't change */	break;
			case 3:	gCellStatus[count] = ON;			break;
//			case 0: // fall through
//			case 1: // fall through
//			case 4: // fall through
//			case 5: // fall through
//			case 6: // fall through
//			case 7: // fall through
//			case 8: // fall through
			default:gCellStatus[count] = OFF;			break;
		}
	}
}

GWorldPtr		gLifeGWorld;

/* I'm thinking drawCells would be better as a funky copybits,
* or by drawing directly to the GWorld, but haven't figured
* out how to code it yet. Even poking values straight into the
* GWorld ought to improve performance. Straight MoveTo LineTo
* is getting 10-15 fps for 100x200 on a 7100/66, though. Could
* be worse. 5/27/95
* This is now the MAJOR time-sink in the program. -DaveP
*/
static void drawCells(void) {
	register int row,column;
	for (row = 0; row < CELL_HEIGHT; row++ ) {
		register int rowbase = row * CELL_WIDTH;
		for(column = 0; column < CELL_WIDTH; column++) {
			if (gCellStatus[rowbase + column] == ON) {
				MoveTo (column, row);
				LineTo (column, row);
			}
		}
	}
}

static void GraphResults(void)
{
	Rect	theSource = gLifeGWorld->portRect;
	Rect	theDest = gCellWindow->portRect;

	SetPort((GrafPtr)gLifeGWorld);
	EraseRect(&theSource);
	ForeColor(blackColor);
	drawCells();
	SetPort(gCellWindow);
	OffsetRect(&theDest,gZoomed?10:5,gZoomed?10:5);
	CopyBits(	&(((GrafPtr)gLifeGWorld)->portBits),
				&(((GrafPtr)gCellWindow)->portBits),
				&theSource, &theDest, srcCopy, nil);
}

void DoCellularAutomata(void)
{	
	SumCells();
	CalcResults();
	GraphResults();
}
