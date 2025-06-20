/* NoTrace - Routines which must be compiled with -trace off                  */

#include "NoTrace.h"


		// � Implementation use
#ifndef __GLOBALS__
#include "Globals.h"
#endif

#ifndef __MUDDOC__
#include "MUDDoc.h"
#endif

#ifndef __UMUDDWELLER__
#include "UMUDDweller.h"
#endif


//------------------------------------------------------------------------------

const int kLargeHist = 256;

//------------------------------------------------------------------------------

#pragma segment MARes

pascal long GrowZone (long needed)
{
	long count, result;

	if (gMemReserve && (gMemReserve != GetGZMoveHnd ()) &&
			(gMemReserve != GetGZRootHnd ())) {
		DisposIfHandle (gMemReserve);
		gMemReserve = NULL;
		gMemIsLow = TRUE;
		return 1;
	}
	if (gMemReserve2 && (gMemReserve2 != GetGZMoveHnd ()) &&
			(gMemReserve2 != GetGZRootHnd ())) {
		DisposIfHandle (gMemReserve2);
		gMemReserve2 = NULL;
		gMemIsLow = TRUE;
		return 1;
	}
	count = GetHandleSize ((Handle) gMyDocList) / sizeof (Handle);
	result = 0;
	while ((count-- > 0) && (result == 0))
		result = ((TMUDDoc *) ((**gMyDocList) [count]))->GrowZone (needed);
	return result;
}

//------------------------------------------------------------------------------

#pragma segment MARes

pascal long TCommandView::GrowZone (long )
{
	Handle mvh, roh, h;
	int i;
	
	if (fMaxLines > 0) {
		mvh = GetGZMoveHnd ();
		roh = GetGZRootHnd ();
		i = fFirstIndex;
		while (i != fCurIndex) {
			h = *(*fHandList + i);
			if ((h != mvh) && (h != roh) && (GetHandleSize (h) >= kLargeHist)) {
				SetHandleSize (h, 0);
				gPurgedHist = TRUE;
				return 1;
			}
			i = (i + 1) % fMaxLines;
		}
	}
	return 0;
}

//------------------------------------------------------------------------------

#pragma segment MARes

pascal long TMUDDoc::GrowZone (long needed)
{
	if ((fLogWindow != NULL) && (fLogWindow->fCommandView != NULL))
		return fLogWindow->fCommandView->GrowZone (needed);
	else
		return 0;
}

//------------------------------------------------------------------------------

#pragma segment MAInit

pascal void InitNoTrace (void)
{
	gMemIsLow = FALSE;
	gMemReserve = NewPermHandle (kRsrvSize);
	gMemReserve2 = NewPermHandle (kRsrvSize);
	gMyDocList = (HandleListHandle) NewPermHandle (0);
	FailNIL (gMyDocList);
	gAppGrowZone = GrowZone;
	gPurgedHist = FALSE;
}

//------------------------------------------------------------------------------
