/*************************************************************************************

 CStringArray.c
	
		Just a simple array of Str255's.  Nothing fancy.
	
	SUPERCLASS = CArray
	
		© 1992 Dave Harkness

*************************************************************************************/


#include "CStringArray.h"


/*************************************************************************************
 IStringArray
*************************************************************************************/

void
CStringArray::IStringArray( void)

{
	CArray::IArray( sizeof(Str255));

}  /* CStringArray::IStringArray */


/******************************************************************************
 IRes
******************************************************************************/

void
CStringArray::IRes( short strListID)

{
	Handle		strList;
	Str255		str;
	short		i, numStrings;
	
	IArray( sizeof(Str255));
	
	strList = GetResource( 'STR#', strListID);
	FailNILRes( strList);
	
	numStrings = *(short*) *strList;
	Resize( numStrings);
	
	for (i = 1; i <= numStrings; i++)
	{
		GetIndString( str, strListID, i);
		InsertAtIndex( str, i);
	}

}  /* CStringArray::IRes */
