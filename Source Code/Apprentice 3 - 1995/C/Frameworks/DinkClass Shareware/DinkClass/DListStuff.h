/*
	File:		DListStuff.h

	Contains:	xxx put contents here xxx

	Written by:	Mark Gross

	Copyright:	© 1992 by Applied Technical Software, all rights reserved.

	Change History (most recent first):

		 <3>	12/31/92	MTG		making the code conditionaly compiled so         that I am
									always working with a current         version in either think c
									or MPW C++
		 <2>	11/14/92	MTG		Bringing the C++ version up to date WRT the         ThinkC
									version.

	To Do:
*/


// This file contains the class declarations for 
// Tlink Tlist and TIterator
// The are all small classes and that is why
// I've chosen to stuff them all into one
// file.


#ifndef __DLISTSTUFF__
#define __DLISTSTUFF__

#include "DObject.h"


class DLink
{
public:
	
	DLink*	fNext;
	void*	fItem;
	
	
	DLink*	Init(DLink *n, void *item);
	DLink*	GetNext(void);
	void*	GetItem(void);
	void	SetNext(DLink* aLink);
	void	SetItem(void* anItem);
};// end of DLink class declaration


class	DList	// of DObject instances!
{
public:
	
	DLink*	fLink;
	
	int		fNumItems;
	
	
	DList*	Init(void);
	void	AddItem(void* item);
	Boolean	RemoveItem(void* item);
	Boolean	ItemInList(void* item);
	int		NumItems(void);
};// end of DList class declaration

//
// The iteration scheam could get lost in space if
// the list interation is shortend as the itteration is going
// resulting in problems.  To be safe avoid iterations where the list
// is shortend durring the time the itterator is opperating on the list.
//

class DIterator
{
	Boolean fInUse;
		// this is set to be TRUE at Init time and FALSE when GetCurrentThenIncrement
		// gets to the end of its list.  And if Init gets called while fInUse is true
		// we drop into the debugger and let the developer about the BooBoo.

public:
	DIterator(void);
	~DIterator(void);
		// constructor sets fInUse to FALSE the destructor is just for style
	
	DLink*	fCurLink;
	
	
	DIterator*	Init(DList* list);
		// Sets up the iteration...
		
	void*	GetCurrentThenIncrement(void);
		// dose what its name indicates....
};// end of class declaration TIterator



#endif __DOBJECT__