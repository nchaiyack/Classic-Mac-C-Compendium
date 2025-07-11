/******************************************************************************
 CStringArray.c

		
	SUPERCLASS = CArray
	
	Copyright � 1991 Symantec Corporation. All rights reserved.
	

 ******************************************************************************/

#include "CStringArray.h"
#include "stdlib.h"
#include "Packages.h"

	
Boolean			CStringArray::cCompAscending;	// compare for ascending sort if true
CStringArray	*CStringArray::cCurrArray;		// array being sorted

/******************************************************************************
 IStringArray
******************************************************************************/

void CStringArray::IStringArray( short maxStringLength)
{
	CArray::IArray( Min( maxStringLength, sizeof( Str255)));
	
}	/* CStringArray::IStringArray */

/******************************************************************************
 IRes
******************************************************************************/

void CStringArray::IRes( short strListID, short maxStringLength)
{
	Handle	strList;
	Str255	str;
	short	i, numStrings;
	
	IArray( maxStringLength);
	
	/* ensure the STR# resource exists	*/
	
	strList = GetResource( 'STR#', strListID);
	FailNILRes( strList);
	
	/* resize once to accomodate all the strings	*/
	
	numStrings = *(short*) *strList;
	Resize( numStrings);
	
	for (i = 1; i < numStrings; i++)
	{
		GetIndString( str, strListID, i);
		str[0] = Min( str[0], elementSize);
		InsertAtIndex( str, i);
	}

}	/* CStringArray::IRes */

/******************************************************************************
 FindString
******************************************************************************/

	static int EqualStrings( void* str1, void* str2)
	{
		return IUEqualString( str1, str2);
	}

long CStringArray::FindString( StringPtr targetString)
{
	cCompAscending = TRUE;

	return Search( targetString, EqualStrings);

}	/* CStringArray::FindString */

/******************************************************************************
 Sort
******************************************************************************/

	void CStringArray::swap( size_t i, size_t j)
	{
		CStringArray::cCurrArray->Swap( i+1, j+1);
	}
	
	int CStringArray::CompareStrings( size_t index1, size_t index2)
	{
		unsigned char *items = (unsigned char*) *CStringArray::cCurrArray->hItems;
		
		if (!cCompAscending)
		{
			long tmp = index1;
			index1 = index2;
			index2 = tmp;
		}
		return IUCompString( items + CStringArray::cCurrArray->ItemOffset( index1+1), 
						items + CStringArray::cCurrArray->ItemOffset( index2+1));
	}


void CStringArray::Sort( Boolean fAscending)
{
	SignedByte	state = HGetState( hItems);
	
		/* qsort won't move memory, but LoadSeg can		*/
		
	cCompAscending = fAscending;
	cCurrArray = this;
	
	HLock( hItems);
	
	_qsort( numItems, CompareStrings, swap);
	
	HSetState( hItems, state);
	
	cCurrArray = NULL;

}	/* CStringArray::Sort */

