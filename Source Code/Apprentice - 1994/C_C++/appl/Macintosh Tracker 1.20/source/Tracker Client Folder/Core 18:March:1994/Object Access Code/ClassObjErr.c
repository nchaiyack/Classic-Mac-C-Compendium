/* ObjErr.c */

#include "ClassObjErr.h"


#ifdef DEBUG
/******************************************************************************
 __noObject
 
 	Called by oopsDebug if message is sent to a bad object pointer/handler
 ******************************************************************************/

void __noObject(void)
{
	PRERR(ForceAbort,"Method invoked via pointer to something other than object.");
}
/******************************************************************************
 __noMethod
 
 	Called by oopsDebug if a message lookup fails
 ******************************************************************************/

void __noMethod(void)
{
	PRERR(ForceAbort,"Method invokation failed.");
}
#endif
