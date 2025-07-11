/*
	MyPatch.c

	Written by Ken Worley, 10/04/94, using Symantec Think C 7.0.
	Copyright 1994. All Rights Reserved.
	
	Feel free to use this code in a patch of your own.  Please don't
	publish or distribute this code without giving me proper credit.
	
	This code is set up to use Apple's "Universal Headers."  If you're not
	using those, some slight modifications may be necessary (changing
	pointer types).
	
	This is the 'task' or 'patch' portion of the code.  The INIT or extension
	portion of the code runs at system startup time, loads this piece of code into
	memory, and installs it as a patch to a trap.
	
	This code shares a data structure with the extension portion of the code,
	the control panel code, and possibly with other code as well.  One of the fields
	in this shared data is called CPon.  This field is initially set by the
	extension based on the values in the 'pref' resource.  The control panel code
	may modify it later based on the user's choices in the control panel.  The
	field determines whether we take action here, or jump to the original trap
	without making any modifications.
	
	A pointer here is assigned to the address 0x12345678.  When the extension code
	loads this code into memory, it replaces that bogus address with the address
	of the shared data structure in memory.
	
	Using routines in the ANSI-A4 library (like vsprintf) requires that we set up
	the A4 register here for access to globals.  Curiously, when I failed to do
	that, the end result worked fine on my Performa 405 and crashed my Classic.
	
	BUILDING THE CODE RESOURCE
	
		This file should be included in a project of type 'code resource' with
		MacTraps and ANSI-A4.  The resource type should be whatever is specified
		in the MyExtension.c file (by a #define) and the ID should also be
		whatever is specified in the MyExtension.c file (by a #define). The
		resource attributes should indicate that this resource goes in the system
		heap and that it should NOT be locked.
*/

/* PROTOTYPES */

void	main( void );
Boolean	StrCompare( Str255 str1, Str255 str2 );

/* INCLUDED FILES */

#include "Notifier.h"		/* prototypes and includes for StrFormat and StrNotify */

#include "SharedData.h"		/* The definition of our shared data structure. */
							/* Includes definitions of myDataStruct, myDataPtr, */
							/* and myDataHandle to refer to this data. */

#include <Traps.h>			/* includes trap names like _BlockMove */

#include <SetUpA4.h>

/* FUNCTIONS */

void	main( void )
{
	myDataPtr		myData;			/* a pointer to our shared data */
	long			oldTrapLoc;		/* local var to hold the old trap routine addr */
	long			pBytes, rBytes;	/* vars for # of bytes in params & return val */
	Boolean			callOriginal;	/* this determines whether we call orig trap */
	Ptr				paramAddr;		/* set to the addr of the params on the stack */
	Ptr				returnAddr;		/* set to the addr of the ret val on the stack */
	Boolean			takeAction;		/* this flag determines whether this trap has */
									/* any real effect */
	
	/*	Set up the A4 register for access to statics and globals. */
	
		RememberA0();
		SetUpA4();
		
	/*	Here's where we assign a bogus address to our data structure pointer.
	 *	Again, this will be edited by the extension code so that it becomes a
	 *	valid address (using the Munger routine).  You won't need to change this.
	 */
		
		myData = (myDataPtr)(0x12345678);
	
	/*	Now wait until the shared data structure isn't in use, then flag it as
	 *	in use until we're through with it.
	 */
	 
	 	while ( myData->inUse ) {}
	 	myData->inUse = true;
	
	/*	Set the takeAction flag which will determine if our modifications get
	 *	executed or the old trap gets called and no modified code is executed.
	 */
	 
	 	takeAction = myData->CPon;
	 	
	/*	Get the old trap location from the shared data structure.  Also get the
	 *	size of the parameters and return value for the trap.  You shouldn't need
	 *	to change this.
	 */
	
		oldTrapLoc = (long)myData->oldTrap;
		pBytes = myData->paramBytes;
		rBytes = myData->returnBytes;
	
	/*	Get the addresses of the parameters sent to the trap and the space reserved
	 *	on the stack for the return value.  We can find these as an offset from A6.
	 *	The address of the parameters goes into paramAddr and the address of the
	 *	space reserved for the return value goes into returnAddr.  You shouldn't
	 *	need to change this (even if it's a register based routine in which case the
	 *	values would have no meaning).
	 */
	 
	 	asm {
	 		MOVE.L	A6,A0			;copy A6 into A0
	 		ADD.L	#8,A0			;add 8 to get addr of original params
	 		MOVE.L	A0,paramAddr	;copy addr of params into local var paramAddr
	 		ADD.L	pBytes,A0		;add # of param bytes to get addr of ret value
	 		MOVE.L	A0,returnAddr	;copy addr of ret value into local var returnAddr
	 	}

	/*	Any changes to the original parameters should be done here (usually if the
	 *	original trap is going to be called).  Also, changes to the return value
	 *	can be made here (usually when the original trap is NOT going to be called).
	 *	The local var returnAddr is a pointer to the place in the stack where the
	 *	return value is.  Remember, before we call the original trap there is no
	 *	return value...only a space for one so the value is undefined.  If we're not
	 *	going to call the original trap, then we can set the return value here to
	 *	whatever we want to be returned to the calling program.  If you modify the
	 *	return value here, then call the original trap, the value will be overwritten
	 *	with the value returned from the original trap.  The easiest way to access
	 *	the return value would be to declare a local variable as a pointer to
	 *	whatever the return type is, then assign returnAddr to that variable casting
	 *	it to a pointer to the correct type.
	 */
	
		if ( takeAction )
		{
		
			/*	I don't do anything with the parameters before the original trap
			 *	is called in this case :-).
			 *
			 *	Example:  If the return value was a Point (same size as a long, 4 bytes)
			 *	then we could easily access it as follows:
			 *
			 *	Declare a local variable at the beginning of main like so:
			 *
			 *		Point	*myPoint;
			 *
			 *	Then assign returnAddr to myPoint casting it as a pointer to a Point:
			 *
			 *		myPoint = (Point*)returnAddr;
			 *		
			 *	Now we can access the value:
			 *
			 *		myPoint->h = whatever;
			 */
			 
	 	}
	
	/*	Set callOriginal so that the original trap will be called.  You may want to
	 *	change this so that callOriginal is set based on certain conditions.  I just
	 *	set it to true here so that the original trap is called.  You may want to
	 *	just set it to false to avoid calling the original trap if you set the
	 *	return value yourself or just don't want the trap to have any effect (when
	 *	there is no return value).
	 */
	
		callOriginal = true;
	
	/*	If the callOriginal flag is true OR if we're not supposed to take any action
	 *	here, call the original trap.
	 */
	 
	if ( callOriginal || !takeAction )
	{
		Ptr	topOfStack;
		
		/*	Make space for a return value (if any) and copy the original parameters to
		 *	the top of the stack.  Then jump to the old trap location using JSR so
		 *	that execution will return here.  You shouldn't need to change this.
		 */
	 
	 		asm {
	 			SUB.L	rBytes,SP		;make space on stack for return value
	 			SUB.L	pBytes,SP		;make space for parameters on stack
	 			
	 			MOVE.L	SP,topOfStack	;save addr of top of stack in a local var
	 		}
	 		
	 		BlockMove( paramAddr, topOfStack, pBytes ); /* copy orig params to */
	 													/* top of stack */
	 		asm {
	 			MOVE.L	oldTrapLoc,A0	;put addr of orig trap into A0
	 			JSR		(A0)			;jump to the original trap
	 		}

		/*	Execution returns here after calling the original trap and the function
	 	 *	result is left on top of the stack.  Copy the new result into the space
	 	 *	allocated for the result when we were originally called, then pop the
	 	 *	new result value off the stack.
	 	 *	This code only executes if there is a return value (if rBytes is nonzero).
	 	 *	You shouldn't need to change this.
	 	 */
	 	 	
	 	 	if ( rBytes )
	 	 	{
	 		 	asm {
	 		 		MOVE.L	SP,topOfStack		;get addr of top of stack (source)
	 		 	}
	 		 	
	 		 	BlockMove( topOfStack, returnAddr, rBytes );

				asm {
					ADD.L	rBytes,SP			;pop new return value off stack
	 	 		}
	 	 	}
	 }	/* end if ( callOriginal || !takeAction ) */
	 
	/*	If you need to modify the return value, do so here.  Its addr is still in
	 *	returnAddr.  If the original trap was called, the value it returned (if any)
	 *	was copied to returnAddr.  Also, paramAddr does still point to the parameters
	 *	if you need to access them still.  Modifying them at this point, however, would
	 *	have no effect since we're just about to pop them off the stack into oblivion.
	 *	Remember if it was a pointer or handle that was sent as a parameter, that value
	 *	won't have changed, but the data that it points to may have.  If you did NOT
	 *	call the original trap and do NOT modify the return value here, and there IS
	 *	a return value, you'll be returning GARBAGE to the calling program (bad idea).
	 *
	 *	I believe that taking action at this point is what's known as a "tail patch"
	 *	which is generally considered to be "ill-advised."  The way this is set up,
	 *	however, I don't see how anything unforseen could happen as long as we're
	 *	careful.
	 */
	 
	 if ( takeAction )
	 {
	 
	 	/*	The return value from MenuSelect is a long integer value whose high word
	 	 *	contains the menu id (zero if none selected) and whose low word contains
	 	 *	the item number selected in that menu (undefined if none selected).
	 	 *	Here I check to see if the menu item selected was "About This Macintosh..."
	 	 *	and if it was, I put up a Notification to brag about our hacking prowess.
	 	 *	Then I return the return value unchanged and the Macintosh info dialog
	 	 *	will come up.
	 	 */
	 	 
	 	 	{	/* isolate our code and variables here in a block for neatness' sake */
	 	 	
	 	 		short		*menuInfo;
	 	 		MenuHandle	menuH;
	 	 		Str255		itemString;
	 	 		Str255		compString = "\pAbout This MacintoshÉ";
	 	 		
	 	 		/* By the way, the three dots at the end of some menu items are NOT */
	 	 		/* three periods, they are one character--an option semi-colon */
	 	 		
	 	 	/*	Assign returnAddr to menuInfo - a pointer to a short */
	 	 	
	 	 		menuInfo = (short*)returnAddr;
	 	 	
	 	 	/*	Now we can use menuInfo[0] to access the high word (menu id#)
	 	 	 *	and menuInfo[1] to access the low word (the item#).
	 	 	 */
	 	 	 	if ( menuInfo[0] )	/* if a menu item was selected, will be nonzero */
	 	 	 	{
	 	 	 		menuH = GetMHandle( menuInfo[0] );	/* get a handle to the menu */
	 	 	 		GetItem( menuH, menuInfo[1], itemString );	/* get menu item text */
	 	 	 		
	 	 	 		if ( StrCompare( itemString, compString ) )
	 	 	 		{
	 	 	 			/* Here's an example of using the Notification manager through
	 	 	 			 * the StrFormat and StrNotify routines.  StrFormat works like
	 	 	 			 * printf.  Do NOT put a "\p" at the beginning of the string.
	 	 	 			 */
						StrFormat( myData->str, "Ha! Ha!  Hackers do it "
							"at system level!  "
							"Menu #%d, Item #%d.", menuInfo[0], menuInfo[1] );
						StrNotify( &(myData->nm), &(myData->str) );
	 	 	 		}
	 	 	 	}
	 	 	}
	 	 	
	 }	/* end if ( myData->patchAction )
	 
	 /*	Now we're through with the shared data */
	 
	 	myData->inUse = false;
	 
	 /* Restore the A4 register to its previous value */
	 
	 	RestoreA4();
	 	
	 /*	Finish up by unlinking A6 (Think C automatically links A6 on entry
	  *	to the routine.  It also unlinks it automatically at the end of the routine,
	  *	but we're going to jump out before the actual end of the routine.),
	  *	removing the original parameters from the stack, and jumping to the return
	  *	address.  You shouldn't need to change this.
	  */
	 	 
	 	 asm {
	 	 	MOVE.L	pBytes,D0	;save the number of bytes in the parameters into D0
	 	 	UNLK	A6			;unlinking leaves return addr at the top of the stack
	 	 	MOVE.L	(SP)+,A0	;move return address to register A0
	 	 	ADD.L	D0,SP		;pop old parameters off stack
	 	 	JMP		(A0)		;jump to the return address
	 	 }
}

#include "Notifier.c"	/* contains StrFormat and StrNotify */

Boolean	StrCompare( Str255 str1, Str255 str2 )
{
	/*	This function returns true if the two strings sent are equal.  The comparison
	 *	is case sensitive and trailing blanks are ignored.  If one string is longer
	 *	and contains something OTHER than blanks after the end of the other string,
	 *	the function will return false.
	 */

	short	shorter,	/* which string is shorter? */
			endShort,	/* last character of the shorter string */
			endLong;	/* last character of the longer string */
	Boolean	result;
	
	result = true;	/* assume they're equal until we find otherwise */
	
	shorter = 1;
	endShort = str1[0];
	endLong = str2[0];
	
	if ( str2[0] < str1[0] )
	{
		shorter = 2;
		endShort = str2[0];
		endLong = str1[0];
	}
	
	{				/* check each letter to see if they're equal */
		short x;
		for ( x = 1; x <= endShort; x++ )
		{
			if ( str1[x] != str2[x] )
			{
				result = false;	/* not equal */
				break;
			}
		}
	}
	
	if ( result )	/* check for non-blank trailing characters */
	{
		short x;
		for ( x = endShort+1; x <= endLong; x++ )
		{
			if ( shorter == 1 )
			{
				if ( str2[x] != ' ' )
				{
					result = false;	/* not a blank */
					break;
				}
			}
			else
			{
				if ( str1[x] != ' ' )
				{
					result = false; /* not a blank */
					break;
				}
			}
		}
	}
	return result;
}

