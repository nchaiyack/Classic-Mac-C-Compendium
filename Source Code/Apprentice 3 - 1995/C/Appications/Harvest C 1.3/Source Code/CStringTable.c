/******************************************************************************
 CStringTable.c

		
	SUPERCLASS = CArrayPane
	
	Copyright © 1991 Symantec Corporation. All rights reserved.
	

 ******************************************************************************/

#include "CStringTable.h"
#include "CStringArray.h"


/******************************************************************************
 IStringTable
******************************************************************************/

void CStringTable::IStringTable( CView *anEnclosure, CBureaucrat *aSupervisor,
						short aWidth, short aHeight,
						short aHEncl, short aVEncl,
						SizingOption aHSizing, SizingOption aVSizing)
{
	CArrayPane::IArrayPane( anEnclosure, aSupervisor, aWidth, aHeight,
					aHEncl, aVEncl, aHSizing, aVSizing);

}	/* CStringTable::IStringTable */

/******************************************************************************
 GetCellText
******************************************************************************/

void CStringTable::GetCellText( Cell aCell, short availableWidth, StringPtr itsText)
{
	ASSERT( member( itsArray, CStringArray));
	
	if (itsArray)
	{
		itsArray->GetItem( itsText, aCell.v+1);
	}

}	/* CStringTable::GetCellText */
