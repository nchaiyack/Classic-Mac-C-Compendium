/*
	This INIT loads the Code 128 resource and uses it to patch _FSDispatch.
	Beware: _FSDispatch takes its parameters in A0 and D0. We must careful not to foul up these registers.

	The Code 128 resource must contain the string "Moof!". In its place we write the original trap address. This mechanism comes
	from a snippet by Jon W�tte. [1994 Note: this is no longer the Right Way� to do it, because writing data into code is not
	clean. Today I would use Gestalt].

	What's more, our patch needs global variables (keeping their value across invocations). We create a Ptr in the System Heap for
	them, and we give its address to the patch using the same [dirty] method (key string is "Data!")	

	To be cute, we display an icon at startup. Displaying an icon is something horribly complicated, so the best way is to use
	a pre-existing code resource, such as ShowIcon7 by James W. Walker.	
	
	New in version 1.1 : a default directory can be specified using the DDIR 0 resource.
*/						

#include "globals.c"

#define _FSDispatch 0xA060

main()
{
	char 				*moof;										/* Pointer used to look for Moof! */
	Handle 			patch;										/* CODE resource handle */
	GlobalsPtr 		Globals;										/* Globals memory block */
	Boolean			ok;											/* True if things went ok */
	Handle			showInit;										/* ShowInit7's code */
	pascal void (*ShowIcon7) (short resid, Boolean adv);					/* ShowIcon7's prototype */
	Handle			ddir;
		
	if (!(patch = GetResource('Code', 128)))  								/* Read in the resource */
		ok = false; 												/* Make sure it succeeded */
	else {
		DetachResource(patch);										/* Mandatory to survive CloseResFile */ 
							
		for (moof = *patch; !strcmp(moof, "Moof!"); moof++); 				/* Look for our signature */
		* (long *) moof = NGetTrapAddress(_FSDispatch, OSTrap);	 		/* Replace it with the old trap address */
		NSetTrapAddress((long) (StripAddress(*patch)), _FSDispatch, OSTrap);	/* Install our routine */
				
		for (moof = *patch; !strcmp(moof, "Data!"); moof++);				/* Look for the other string */						
		if ((Globals = (GlobalsPtr) NewPtrSysClear(globalsSize)) == NULL)		/* Allocate globals */
			ok = false; 
		else {		
			* (long *) moof =  (long) Globals;							/* Write their address into the patch code */
			ok = true;
			
			if (!(ddir = GetResource('DDIR', 0)))							/* Read in the default directory */ 
				Globals->dirID = fsRtDirID;							/* If it doesn't exist, use the root dir */
			else
				Globals->dirID = * (long*) (*ddir);
		}
	}
	
		
	showInit = GetResource('Code', -4048);								/* Load ShowIcon7's code */
	if (showInit != NULL) {											/* Make sure it succeeded */
		ShowIcon7 = (pascal void (*) (short, Boolean)) StripAddress (*showInit);	
		if (ok)															
			ShowIcon7(128, true);									/* Show an icon with or without a cross */
		else 														/* depending on success */
			ShowIcon7(129, true);
	}
}

/* String comparison routines. Have to do it by hand because we don't want to link with ANSI */

int strcmp(char * s1, char * s2) 
{
	while (*s1 == *s2 && *s2) {
		s1++; s2++;
	}
	return (*s1 == *s2);
}