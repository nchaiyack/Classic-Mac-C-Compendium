
#include "DebugUtil.h"


/* if you don't know about assertions, I demand (suggest is too light a word to use in this case)
	that you read "Writing Solid Code" by Steve Maguire.
	This function will display theString in your debugger */
void
my_assert ( Boolean passed, const char *theString )
{
	if ( !passed )
	{
		DebugStr ( (StringPtr)theString );
	}
}
