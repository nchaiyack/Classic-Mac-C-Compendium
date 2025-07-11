/***********************
** morphutils.c
**
** This file has the utilities/routines to manipulate
** the 'morf' resources.  Called from main().
************************/

#include <MacHeaders>
#include "globals.h"
#include "morphutils.h"

void MorphAddRes(void)  // adds a resource to the "Type" menu & program file
{
}


void MorphDelRes(void)  // deletes a rez from the "Type" menu & program file
{
}


void MorphClearProc(void)  // frees the current procedure, if a resource
{
	if ( (gMorphProcH != NULL) &&		// if it points to *something*...
		(gMorphProcH != gDefaultProcH)) // AND it's not the builtin, then...
	{
		HUnlock((Handle)gMorphProcH);
		ReleaseResource((Handle)gMorphProcH);
		gMorphProcH = NULL;
	}
} // MorphClearProc()