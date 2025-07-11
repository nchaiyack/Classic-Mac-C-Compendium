/******************************************************************************
 CStringArray.h

		
	SUPERCLASS = CArray
	
	Copyright � 1991 Symantec Corporation. All rights reserved.
	

 ******************************************************************************/

#define _H_CStringArray

#include "CArray.h"
#include <size_t.h>

class CStringArray : public CArray
{
public:
	void IStringArray( short maxStringLength);
	
	virtual void IRes( short strListID, short maxStringLength);
	
	virtual long FindString( StringPtr targetString);
	
	virtual void Sort( Boolean fAscending);
	
protected:

	static Boolean			cCompAscending;
	static CStringArray		*cCurrArray;

	static void swap( size_t i, size_t j);
	static int CompareStrings( size_t index1, size_t index2);

};