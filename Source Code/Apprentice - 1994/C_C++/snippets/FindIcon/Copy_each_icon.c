#include "Copy_each_icon.h"

static pascal OSErr Copy_one_icon(
/* --> */	ResType theType,
/* <-> */	Handle *theIcon,
/* --- */	void *yourDataPtr );

/*	------------------------------------------------------------------
	Copy_each_icon			This procedure makes copies of the icon
							handles in a suite, so that they will not
	be resource handles and will not be purgeable.  Note that
	if the originals are resources in a file that is in use by other
	programs, then DetachResource would not be appropriate.
	------------------------------------------------------------------
*/
OSErr Copy_each_icon(
/* <-> */	Handle the_suite
)
{
	return ForEachIconDo( the_suite, svAllAvailableData, Copy_one_icon, NULL );
}

static pascal OSErr Copy_one_icon(
/* --> */	ResType theType,
/* <-> */	Handle *theIcon,
/* --- */	void *yourDataPtr )
{
	OSErr	err;
	
	if (*theIcon != NULL)
	{
		LoadResource( *theIcon );
		err = HandToHand( theIcon );
		if (err != noErr)
			*theIcon = NULL;
	}
	
	return noErr;
}
