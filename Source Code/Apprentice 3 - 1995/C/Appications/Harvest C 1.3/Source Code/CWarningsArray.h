/******************************************************************************
 CWarningsArray.h

		
	SUPERCLASS = CArray
	
	Copyright © 1991 Symantec Corporation. All rights reserved.
	

 ******************************************************************************/

#define _H_CWarningsArray

#include "CArray.h"
#include "CHarvestOptions.h"

#include <size_t.h>

struct Warning {
	Str255 desc;
	int num;
	Boolean isOn;
};


class CWarningsArray : public CArray
{
public:
	CHarvestOptions *theOpts;
	int WarningCount;
	void IWarningsArray( CHarvestOptions *);
	
	void GetWarningEntry(char *,int);
	void AddWarning(unsigned char *,int,Boolean);
};