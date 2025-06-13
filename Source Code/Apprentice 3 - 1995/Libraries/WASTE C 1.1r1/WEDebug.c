// { WASTE PROJECT }
// { Debugging Routines }

// { Copyright © 1993-1994 Marco Piovanelli }
// { All Rights Reserved }

#include "WASTEIntf.h"

#ifdef WASTE_DEBUG
pascal void _WEAssert(Boolean condition, StringPtr message)
{
	if (condition == false)
	{
//				message := Concat('Assertion Failed: ', message);
		DebugStr(message);
	}
} // { _WEAssert }

pascal void _WESanityCheck(WEHandle hWE)
{
	// { _WESanityCheck performs several checks on two key data structures: }
	// { the run array and the style table, verifying a number of assertions. }
	// { This routine made it possible to identify many subtle bugs during development. }

	WEPtr pWE;
	RunArrayPtr	pRuns;
	StyleTablePtr pStyles;
	long i, j, refCount;

	// { we aren't going to move memory }
	pWE = *hWE;
	pRuns = *(pWE->hRuns);
	pStyles = *(pWE->hStyles);

	// { check the consistency of the run array }
	// { first runStart must be zero }
	_WEAssert(pRuns[0].runStart == 0, "\pFirst run array element is bad");

	// { last (dummy) runStart must be textLength + 1 and styleIndex must be -1 }
	_WEAssert((pRuns[pWE->nRuns].runStart == pWE->textLength + 1) && 
		(pRuns[pWE->nRuns].styleIndex == -1), "\pLast run array element is bad");

	// { all runs must be at least one character long }
	for (i = pWE->nRuns - 1; i>=0; i--)
		_WEAssert(pRuns[i + 1].runStart - pRuns[i].runStart > 0, "\pRun length less than one");

	// { no two consecutive runs may reference the same style }
	for (i = pWE->nRuns - 1; i>=0; i--)
		_WEAssert(pRuns[i + 1].styleIndex != pRuns[i].styleIndex, "\pSpurious run boundary");

	// { all run array elements (except the last dummy entry) must reference an existing style }
	j = pWE->nStyles;
	for (i = pWE->nRuns - 1; i>=0; i--)
		_WEAssert((pRuns[i].styleIndex >= 0) && (pRuns[i].styleIndex < j),
		 	"\pInvalid style index");

	// { the number of runs referencing each style in the style table }
	// { must match the style reference count }
	for (j = pWE->nStyles - 1; j>=0; j--)
	{
		refCount = 0;
		for (i = pWE->nRuns - 1; i>=0; i--)
		{
			if (pRuns[i].styleIndex == j)
					refCount = refCount + 1;
		}
		_WEAssert(pStyles[j].refCount == refCount, "\pBad style reference count");
	}

/* the following code crashes for some reason -- Dan Crevier 2/13/95
	// { there may not be two identical entries in the style table (except for unused entries) }
	for (i = pWE->nStyles - 1; i>0; i--)
	{
		if (pStyles[i].refCount == 0)
			continue;
		for (j = i - 1; j>=0; j--)
		{
			if (pStyles[j].refCount == 0)
				continue;
			_WEAssert(_WEBlockCmp((Ptr)&pStyles[i].info, (Ptr)&pStyles[j].info,
				sizeof(pStyles[i].info)) == false, "\pDuplicate entry in style table");
		}
	}
*/
} // { _WESanityCheck }

#endif