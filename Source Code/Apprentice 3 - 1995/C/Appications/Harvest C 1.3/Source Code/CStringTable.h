/******************************************************************************
 CStringTable.h

		Interface for StringTable class.
		
	SUPERCLASS = CArrayPane
	
	Copyright � 1991 Symantec Corporation. All rights reserved.
	

 ******************************************************************************/

#define _H_CStringTable

#include "CArrayPane.h"

class CStringTable : public CArrayPane
{
public:
	void IStringTable( CView *anEnclosure, CBureaucrat *aSupervisor,
							short aWidth, short aHeight,
							short aHEncl, short aVEncl,
							SizingOption aHSizing, SizingOption aVSizing);
	
	virtual void GetCellText( Cell aCell, short availableWidth, StringPtr itsText);


};