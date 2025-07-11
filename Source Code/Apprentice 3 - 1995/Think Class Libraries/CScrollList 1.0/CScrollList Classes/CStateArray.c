/*************************************************************************************

 CStateArray.c
	
		Implements an array of states (as shorts).
	
	SUPERCLASS = CArray
	
		� 1992 Dave Harkness

*************************************************************************************/


#include "CStateArray.h"


		/* Macros for calculating item offsets and addresses.	*/
		/* They are all one-based indices						*/

#define ASSERT_INDEX( index)	ASSERT( (index > 0)&&(index <= numItems))
#define ITEM_OFFSET( index) ((index-1) * elementSize)
#define ITEM_PTR( index) (&(*hItems)[ ITEM_OFFSET(index) ])


/*************************************************************************************
 IStateArray
*************************************************************************************/

void
CStateArray::IStateArray( long howMany, short fState)

{
	CArray::IArray( sizeof( short));
	
	SetBlockSize( 10);
	numItems = howMany;
	Resize( howMany);
	SetAllStates( fState);

}  /* CStateArray::IStateArray */


/*************************************************************************************
 SetState

	Set the state of the item at index to fState.
*************************************************************************************/

void
CStateArray::SetState( long index, short fState)

{
	ASSERT_INDEX( index);
	
	*((short *)ITEM_PTR( index)) = fState;

}  /* CStateArray::SetState */


/*************************************************************************************
 ToggleState

	Toggle the state of the item at index.
*************************************************************************************/

void
CStateArray::ToggleState( long index)

{
	ASSERT_INDEX( index);
	
	*((short *)ITEM_PTR( index)) = !(*((short *)ITEM_PTR( index)));

}  /* CStateArray::ToggleState */


/*************************************************************************************
 GetState

	Return the state of the item at index.
*************************************************************************************/

short
CStateArray::GetState( long index)

{
	ASSERT_INDEX( index);
	
	return (*((short *)ITEM_PTR( index)));

}  /* CStateArray::GetState */


/*************************************************************************************
 SetAllStates

	Set the state of every item to fState.
*************************************************************************************/

void
CStateArray::SetAllStates( short fState)

{
	long	index;
	
	for ( index = 1; index <= numItems; index++ )
		*((short *)ITEM_PTR( index)) = fState;

}  /* CStateArray::SetAllStates */
