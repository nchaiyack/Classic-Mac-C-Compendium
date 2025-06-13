/*
	Harvest C
	Copyright 1992 Eric W. Sink.  All rights reserved.
	
	This file is part of Harvest C.
	
	Harvest C is free software; you can redistribute it and/or modify
	it under the terms of the GNU Generic Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.
	
	Harvest C is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with Harvest C; see the file COPYING.  If not, write to
	the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
	
	Harvest C is not in any way a product of the Free Software Foundation.
	Harvest C is not GNU software.
	Harvest C is not public domain.

	This file may have other copyrights which are applicable as well.

*/

/******************************************************************************
 CWarningsArray.c

		
	SUPERCLASS = CArray
	
	Copyright © 1991 Symantec Corporation. All rights reserved.
	

 ******************************************************************************/

#include "CWarningsArray.h"
#include "stdlib.h"
#include "Packages.h"

	
/******************************************************************************
 IWarningsArray
******************************************************************************/

void CWarningsArray::IWarningsArray( CHarvestOptions *itsOpts)
{
	CArray::IArray(sizeof(struct Warning));
	theOpts = itsOpts;
	WarningCount = 0;
	
}	/* CWarningsArray::IWarningsArray */

void CWarningsArray::GetWarningEntry(char *t,int n) {
	struct Warning theWarning;
	GetItem(&theWarning,n);
	CopyPString(theWarning.desc,t);
	if (theOpts->warnings[theWarning.num]) {
		CopyPString("\p ON   ",t);
	}
	else {
		CopyPString("\p OFF ",t);
	}
	ConcatPStrings(t,theWarning.desc);
}

void CWarningsArray::AddWarning(unsigned char *s,int n,Boolean defValue)
{
	struct Warning w;
	CopyPString(s,w.desc);
	w.num = n;
	w.isOn = defValue;
	WarningCount++;
	InsertAtIndex((Ptr) &w,WarningCount);
	theOpts->warnings[n] = defValue;
}
