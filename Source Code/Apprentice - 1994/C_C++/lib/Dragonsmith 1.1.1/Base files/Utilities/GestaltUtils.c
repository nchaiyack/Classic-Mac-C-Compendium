/*
	GestaltUtils.c
	
	Created	17 Sep 1992	GestaltBatchCheck and GestaltResultOK
	Modified	

	Copyright � 1992 by Paul M. Hoffman
	Send comments or suggestions to paul.hoffman@umich.edu -or- dragonsmith@umich.edu
	
	This source code may be freely used, altered, and distributed in any way as long as:
		1.	It is GIVEN away rather than sold (except as expressly permitted by the author)
		2.	This statement and the above copyright notice are left intact.

*/

#include	"GestaltUtils.h"

OSErr GestaltBatchCheck (GstCheckList **checkList)
{
	short			n, i;
	GstCheckRec		*checkp;
	OSErr			err = noErr;
	
	if (checkList == NULL || *checkList == NULL)
		return nilHandleErr;
		
	n = (*checkList)->numChecks;
	if (n < 0)
		return nilHandleErr;		// Assume that something bad has happened
	else if (n == 0)
		return noErr;
	
	HLock ((Handle) checkList);
	
	for (i = 0, checkp = (*checkList)->check; i < n; i++, checkp++) {
		if (GestaltResultOK (checkp->selector, checkp->compOp, checkp->compValue) == FALSE) {
			err = checkp->failureError;
			break;
		}
	}
	
	HUnlock ((Handle) checkList);
	
	return err;
}

Boolean GestaltResultOK (OSType selector, short compOp, long compValue)
{
	OSErr	err;
	long		result;
	
	err = Gestalt (selector, &result);
	if (err != noErr)
		return FALSE;
	
	switch (compOp) {
		case opBitAnd:
			return (result & compValue == compValue);
			break;
		case opEqual:
			return (result == compValue);
			break;
		case opNotEqual:
			return (result != compValue);
			break;
		case opGreater:
			return (result > compValue);
			break;
		case opGreaterOrEqual:
			return (result >= compValue);
			break;
		case opLess:
			return (result < compValue);
			break;
		case opLessOrEqual:
			return (result <= compValue);
			break;
		case opNoErrIsOK:
			return TRUE;
			break;
		default:
			return FALSE;
	}
}

