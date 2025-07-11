#include "Is_suite_empty.h"

/*	------------------------------------------------------------------
	Is_suite_empty		Test whether an icon suite has any icons.
	------------------------------------------------------------------
*/
static pascal OSErr Test_handle( ResType theType, Handle *theIcon,
			void *yourDataPtr )
{
	if (*theIcon != NULL)
		*(Boolean *)yourDataPtr = false;	// not empty!
	
	return noErr;
}

Boolean		Is_suite_empty( Handle the_suite )
{
	Boolean		retval;
	
	retval = true;
	ForEachIconDo( the_suite, svAllAvailableData, Test_handle, &retval );
	
	return retval;
}


