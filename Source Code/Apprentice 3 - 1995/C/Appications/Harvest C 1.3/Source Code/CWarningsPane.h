/******************************************************************************
 CWarningsPane.h

		Interface for WarningsPane class.
		
	SUPERCLASS = CArrayPane
	
	Copyright © 1991 Symantec Corporation. All rights reserved.
	

 ******************************************************************************/

#define _H_CWarningsPane

#include "CArrayPane.h"

class CHarvestOptions;

class CWarningsPane : public CArrayPane
{
public:
	CHarvestOptions *theOpts;
	void IWarningsPane( CView *anEnclosure, CBureaucrat *aSupervisor,
							short aWidth, short aHeight,
							short aHEncl, short aVEncl,
							SizingOption aHSizing, SizingOption aVSizing);
	
	virtual void GetCellText( Cell aCell, short availableWidth, StringPtr itsText);


};