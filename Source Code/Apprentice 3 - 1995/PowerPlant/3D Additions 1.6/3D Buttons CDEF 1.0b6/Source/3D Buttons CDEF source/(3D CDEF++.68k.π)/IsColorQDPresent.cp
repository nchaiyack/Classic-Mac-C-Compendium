/*
	Public domain by Zig Zichterman.
*/
/*
	IsColorQDPresent()
	
	A boolean that returns TRUE if color quickdraw is installed
	
	12/27/94	zz	h	initial write
*/
#include "IsColorQDPresent.h"

#include <Gestalt.h>

Boolean
IsColorQDPresent(void)
{
	long	result	= 0;
	OSErr	err		= ::Gestalt(gestaltQuickdrawFeatures,&result);
	if ((err != noErr) || ((result & (1 << gestaltHasColor)) == 0)) {
		return false;
	} else {
		return true;
	}
}

