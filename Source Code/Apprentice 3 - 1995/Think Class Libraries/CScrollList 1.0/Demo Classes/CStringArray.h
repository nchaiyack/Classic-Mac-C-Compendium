/*************************************************************************************

 CStringArray.h
	
		Interface for CStringArray
	
	SUPERCLASS = CArray
	
		� 1992 Dave Harkness

*************************************************************************************/


#define _H_CStringArray

#include <CArray.h>


class CStringArray : public CArray
{
public:

	void				IStringArray( void);
	
	void				IRes( short strListID);

};
