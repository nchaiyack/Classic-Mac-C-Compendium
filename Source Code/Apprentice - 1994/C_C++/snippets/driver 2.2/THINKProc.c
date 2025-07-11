/*
 *	THINKProc.c by Pete Resnick. This source file should be compiled in
 *	THINK C in a seperate project as a code resource. The resource type
 *	is defined in driver.c as 'PROC' and the resource ID is defined as 128
 *	in drvrincludes.h. The ID canbe changed if necessary. This code
 *	actually replaces the call to GetResource in THINK C's driver glue.
 *	Instead of the driver DATA resource and multi-segment DCOD resources
 *	being in the driver's resource file, they are detached handles in the
 *	system heap and are literally "tacked on" to the bottom of this piece
 *	of code: attached to the end of the pointer containing *THIS* code is
 *	an array of handles and resource ID's. The code returns the handle to
 *	the correct detached resource.
 */

#include "drvrincludes.h"
#include <SetUpA4.h>

/*
 *	Global variables are stored at the end of the code segment. With
 *	resRec, we have a zero-filled structure at the end of the code segment
 *	which can server as a marker.
 */
RsrcRec resRec = {nil, 0, 0};

pascal Handle main(ResType theTyp, short theID)
{
	Size theSize;
	RsrcRec *resPtr;
	Ptr codePtr;
	
	RememberA0();
	SetUpA4();
	
	/* Get the starting address of the code segment */
	asm { move.l a4, codePtr }
	
	/* Figure out how big the pointer is and start at the end */
	theSize = GetPtrSize(codePtr);
	resPtr = (RsrcRec *)(codePtr + theSize);
	
	/* Look for the handle with the correct resource ID */
	while(((--resPtr)->rsrc != nil) &&
	      ((resPtr->id != theID) || (resPtr->typ != theTyp)))
		/* Do nothing */ ;
	RestoreA4();
	return(resPtr->rsrc);
}
