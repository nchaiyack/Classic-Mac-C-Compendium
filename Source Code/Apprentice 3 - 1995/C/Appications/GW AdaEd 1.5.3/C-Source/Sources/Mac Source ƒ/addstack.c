/* Demonstration of how to change your stack size and actually
	 prove it happened. By Mark Y. Geschelin, Symantec Technical Support.
    Special thanks to Jorg Brown for help in the methodology of proving
     the stack actually is changed in size 
     � Mark Y. Geschelin Symantec Corp. 1989,1990

	
    Created   3-Dec-89  	MYG
	revised	  4-Dec-89      MYG   Removed assembly language and replaced it 
								  with MemError function to test the result
								  of SetApplLimit()
	revised	  15-Dec-89 	MYG	  Removed toolbox inits as a  precursor to 
								  setstack or add stack 
									
	Revised    5-July-90    MYG   Made more Clear 
	
	Revised    1-AUG-90		MYG   Fixed setstack and changed to call setstack instead of
								  addstack
	Revised    ?-DEC-94		FAB   use the new LowMem calls [Fabrizio Oddone]
*/

#include <stdio.h>
#include "addstack.h"


/******  Stack expansion function 1 
	expands stack based on current stack size */
	
OSErr addtostack(size_t nbytes)
{
	
	SetApplLimit(GetApplLimit() - nbytes);	 
	return(MemError());
}



/***  Set stack sets the stack to the size you want it   
		A much better way to do things since it is entirely
 		portable*/
 
OSErr setstack (size_t nbytes)
{
//	size_t *CurStackBase;
// go use the new LowMem calls [Fabrizio Oddone]

	SetApplLimit(LMGetCurStackBase() - nbytes);
	return(MemError());
}


#ifdef TEST
#define TEST

main()
{
/****  Change this variable to add more space  ****/
OSErr error;
		
	if ((error = setstack(32*1024L)) == noErr) {
	
	/*Call MaxApplZone to shove the heap right to the top of where we said it can live.
	 So that when we play with a7 in tmon while in test's infinite loop we can 
	  demonstrate that the stack size has changed */
	  
		MaxApplZone();
	/******************************
	add all your code in here
	*/
	}
	else
	{	printf("couldn't get stack space\n  %d",error);
	}
}

#endif