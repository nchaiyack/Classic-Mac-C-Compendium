/*
	This routine gets called for each item (which could be either a file or a folder)
	that the caller wants dropped.  The determining factor is the definition of the 
	qWalkFolder compiler directive.   Either way, the item in question should be
	processed as a single item and not "dissected" into component units (like subfiles
	of a folder!)
*/
#include <Files.h>

OSErr ProcessItem(FSSpecPtr myFSSPtr) {
	OSErr	err = noErr;
	
	// do something here
	
	return(err);
}
