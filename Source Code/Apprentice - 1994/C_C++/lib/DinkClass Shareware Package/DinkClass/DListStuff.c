/*
	File:		DListStuff.cp

	Contains:	xxx put contents here xxx

	Written by:	Mark Gross

	Copyright:	� 1992 by Applied Technical Software, all rights reserved.

	Change History (most recent first):

		 <4>	12/31/92	MTG		making the code conditionaly compiled so         that I am
									always working with a current         version in either think c
									or MPW C++
		 <3>	11/14/92	MTG		Bringing the C++ version up to date WRT the ThinkC version.
		 <2>	 9/20/92	MTG		Binging the C++ verion of DinkClass up to Date with the THINK C
									version

	To Do:
*/


// This is the file implementing the three classes
// Tlink, Tlist, and TIterator

#include "DListStuff.h"


	
DLink* DLink::Init(DLink *n, void *item)
{
	if (n)
		fNext = n;
	else
		fNext = NULL;
	
	fItem = item;
	return this;
}// end of constuctor


DLink*	DLink::GetNext(void)
{
	return	fNext;
}

void*	DLink::GetItem(void)
{
	return	fItem;
}

void	DLink::SetNext(DLink* aLink)
{
	fNext = aLink;
}

void	DLink::SetItem(void* anItem)
{
	fItem = anItem;
}

// end of DLink class definition


	
DList* DList::Init(void)
{
	fLink = NULL;
	fNumItems = 0;
	return this;
}// end of constuctor


void	DList::AddItem(void* item)
{
	fLink = (new DLink)->Init(fLink, item);
	fNumItems++;
}// end of function AddItem



Boolean	DList::ItemInList(void* item)
{
	DLink  *temp;
		
	for(temp = fLink; temp!=NULL; temp = temp->GetNext() )
	{	if(temp->GetItem() == item)
			return true; //item found
	}
	return false; // item not in list!!!
}

Boolean	DList::RemoveItem(void* item)
{
	DLink  *temp, *last;
	
	last = NULL;
	
	for(temp = fLink; temp!=NULL; temp = temp->GetNext() )
	{	
		if(temp->GetItem() == item)
		{
			if(last==NULL) // that happens only if fLink == item
				fLink = temp->GetNext();
			else
				last->SetNext(temp->GetNext());
			
			delete temp;
			fNumItems--;
			return true; //item found and removed
		}
		else
			last = temp;
	}
	return false; // item not in list and or not removed!!!
}// end of function RemoveItem



int		DList::NumItems(void)
{
	return fNumItems;
}

	


DIterator::DIterator(void)
{
	fInUse = FALSE;
}

DIterator::~DIterator(void)
{
	//Stub
}

	
DIterator* DIterator::Init(DList* list)
{
	if(fInUse == TRUE)
		DebugStr("\pMAJOR LOGIC ERROR!! attempting to use an iterator which is in use!!!");
	fInUse = TRUE;
	fCurLink = list->fLink;
	return this;
}// end of constructor


void*	DIterator::GetCurrentThenIncrement(void)
{
	DLink* link = fCurLink;
	
//
// In a loop using an instansiation of DIterator, if the link->GetNext()
// link gets removed from the list in the loop, then your loop will go south becuse GeCurrentThenIncr\ement
// would not be able to know that what IT THINKS is the current link isn't a valid pointer
// any more....
//

	if(fCurLink)
	{
		fCurLink = fCurLink->GetNext();
		return (link->GetItem() );
	}
	else
	{
		fInUse = FALSE;// all done!
		return NULL;
	}
		
}// end of function GetCurrentThenIncrement


