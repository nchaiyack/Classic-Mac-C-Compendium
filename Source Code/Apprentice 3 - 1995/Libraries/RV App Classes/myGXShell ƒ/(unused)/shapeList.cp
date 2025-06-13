THIS CODE IS UNFINISHED AND UNTESTED

#include <math types.h>
#include <graphics routines.h>

#include "shapeList.h"

shapeList::shapeList( long maxItems)
{
	if( maxItems <= 0)
	{
		DebugStr( "\pshapeList::shapeList : maxItems <= 0");
	}
	numItems = 0;
	maxNumItems = maxItems;
	theItems = new gxShape[ maxNumItems];
	numIterators = 0;
}

shapeList::~shapeList()
{
	if( numIterators != 0)
	{
		DebugStr( "\pkilling an shapeList which is in use!");
	}
	delete theItems;
}

void shapeList::Add( const gxShape item)
{
	if( numIterators != 0)
	{
		DebugStr( "\pAdding to an shapeList which is in use!");
	}
	Boolean isPresent;
	const long newindex = indexFor( item, &isPresent);
	if( isPresent)
	{
		DebugStr( "\pAdding an item which is already there!");
	} else {
		InsertItemAt( item, newindex);
	}
}

void shapeList::Remove( const gxShape item)
{
	if( numIterators != 0)
	{
		DebugStr( "\pRemoving from an shapeList which is in use!");
	}
	Boolean isPresent;
	const long newindex = indexFor( item, &isPresent);
	if( isPresent)
	{
		RemoveItemAt( newindex);
	} else {
		DebugStr( "\pRemoving an item which is already there!");
	}
}

Boolean shapeList::Contains( const gxShape item) const
{
	Boolean isPresent;
	const long newindex = indexFor( item, &isPresent);
	return isPresent;	
}

long shapeList::indexFor( const gxShape item, Boolean *isPresent) const
{
	//
	// for now, use sequential search (the number of items is small)
	//
	int result = 0;
	while( (result < numItems) && (theItems[ result] <= item))
	{
		result += 1;
	}
	*isPresent = ((result < numItems) && (theItems[ result] == item));
	return result;
}

void shapeList::InsertItemAt( const gxShape item, long index)
{
	if( numIterators != 0)
	{
		DebugStr( "\pInsertItemAt for an shapeList which is in use!");
	}
	if( numItems == maxNumItems)
	{
		//
		// Reallocate table twice the size:
		//
		gxShape *oldItems = theItems;
		theItems = new gxShape[ 2 * maxNumItems];
		for( int i = 0; i < maxNumItems; i++)
		{
			theItems[ i] = oldItems[ i];
		}
		maxNumItems *= 2;
		delete oldItems;
	}
	for( int i = numItems - 1; i > index; i--)
	{
		theItems[ i] = theItems[ i - 1];
	}
	theItems[ index] = item;
	numItems += 1;
}

void shapeList::RemoveItemAt( long index)
{
	if( numIterators != 0)
	{
		DebugStr( "\pRemoveItemAt from an shapeList which is in use!");
	}
	for( int i = index; i < numItems - 1; i++)
	{
		theItems[ i] = theItems[ i + 1];
	}
}

shapeList_iterator::shapeList_iterator( shapeList &theList)
{
	the_shapeList = &theList;
	currentItem = the_shapeList->numItems - 1;
	the_shapeList->numIterators += 1;
}

shapeList_iterator::~shapeList_iterator()
{
	the_shapeList->numIterators -= 1;
}

Boolean shapeList_iterator::operator()( gxShape *item)
{
	Boolean result = false;
	if( currentItem >= 0)
	{
		currentItem -= 1;
		*item = the_shapeList->theItems[ currentItem];
		result = true;
	}
	return result;
}
