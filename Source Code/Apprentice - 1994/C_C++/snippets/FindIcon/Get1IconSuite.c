#include <exceptions.h>
#include "Get1IconSuite.h"

/*	--------------------------------------------------------------------
	Get1IconSuite			Like GetIconSuite, but only looks in
							the current resource file.
	
	In case you're wondering why it would be necessary to ensure that
	icons come from only one file, suppose you're looking at a
	file that has its custom icon bit set, but for some reason does
	not contain a custom icon, or at least not a full family.
	Way down the resource chain, there may be another file, say a
	font file, that does have a full family of custom icons. 
	So you get an unexpected icon.
	--------------------------------------------------------------------
*/

static pascal OSErr Get_1_icon(
/* --> */	ResType the_type,
/* <-> */	Handle *the_icon,
/* --> */	short	*res_id );


pascal OSErr Get1IconSuite(
/* <-- */	Handle *theSuite,
/* --> */	short theID,
/* --> */	long theSelector
)
{
	OSErr		err;
	
	err = NewIconSuite( theSuite );
	forbid( err, NewIconSuite );
	
	err = ForEachIconDo( *theSuite, theSelector,
		(IconAction) Get_1_icon, &theID );
	
NewIconSuite:
	return err;
}

static pascal OSErr Get_1_icon(
/* --> */	ResType the_type,
/* <-> */	Handle *the_icon,
/* --> */	short	*res_id )
{
	*the_icon = Get1Resource( the_type, *res_id );
	
	return noErr;
}
