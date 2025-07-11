/*
	Public domain by Zig Zichterman.
*/
/*
	CDEF++.cp
	
	Main entry point for a CDEF that needs A4 for globals
	(such as anything that uses C++ vtables and destructor
	chains)
	
	12/20/94	zz	h	initial write
*/

// required for using A4 for globals, which C++ vtables
// and local destructor chains are
#include <A4Stuff.h>

#include "LCDEFControl.h"

pascal long
main(
	short			inVariation,
	ControlHandle	ioControl,
	short			inMsg,
	long			ioParam)
{
	// save the value that the A4 register had upon entry.
	// We'll restore it later. SetCurrentA4() also sets
	// A4 to a value we can use for our globals
	long	saveA4	=	::SetCurrentA4();
	
	// let someone else dispatch the call to the correct 
	// CDEF variant and message
	long	returnValue	= LCDEFControl::Main(inVariation,ioControl,inMsg,ioParam);
		
	// restore the A4 register to the value it had upon entry
	::SetA4(saveA4);
	
	// return whatever the CDEF returned
	return returnValue;
}
