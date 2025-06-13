#include "LinkedLists.h"

/***************************************************

	SlimListClass: Simple Pop and Push Features
	
***************************************************/

template <class itemRecord> Boolean SlimListClass<itemRecord>::Push(itemRecord &theItem)
{
	ListItem<itemRecord> *newItem;
	
	newItem=new ListItem<itemRecord>;
	newItem->item=theItem;
	
	if (newItem) {
		newItem->next=stackTop;
		stackTop=newItem;			//	stackTop variable always points to the top of the stack
		return 1;
	}
	
	return 0;
}

template <class itemRecord> Boolean SlimListClass<itemRecord>::Pop(itemRecord &theItem)
{
	if (stackTop) {
		theItem=stackTop->item;
		ListItem<itemRecord> *temp=stackTop->next;
		
		delete stackTop;
		stackTop=temp;
		
		return 1;
	}
	
	return 0;
}

template <class itemRecord> void SlimListClass<itemRecord>::DeleteAll()
{
	ListItem<itemRecord>	*temp=nil;
	
	if (stackTop) {
		temp=stackTop->next;
		delete stackTop;
		stackTop=temp;
	}
	
	while (temp) {
		temp=stackTop->next;
		delete stackTop;
		stackTop=temp;
	}
		
}

/****************************
*							*
*	Stepping List			*
*							*
****************************/

template <class itemRecord> Boolean SteppingList<itemRecord>::Add(itemRecord &theItem)
{
	ListItem<itemRecord> *newItem, *tempItem;
	
	placeHolder=newItem=new ListItem<itemRecord>;
	
	if (newItem) {
		numberOfItems++;
		newItem->item=theItem;
		
		if (stackTop) {							//	Go through whole list until end is found, add
			tempItem=stackTop;					//	newItem to the end
			while (tempItem->next)
				tempItem=tempItem->next;
			
			tempItem->next=newItem;
		}
		
		else
			stackTop=newItem;
			
		newItem->next=nil;						//	Set newItem->next to nil, connotating end of list
		return 1;
	}
	
	return 0;
}	

template <class itemRecord> Boolean SteppingList<itemRecord>::Push(itemRecord &theItem)
{
	ListItem<itemRecord> *newItem;
	
	placeHolder=newItem=new ListItem<itemRecord>;
	
	if (newItem) {						//	Essentially, this is the same as SlimSteppingListes,
		numberOfItems++;				//	except that it updates the placeHolder var and the
		newItem->item=theItem;			//	numberOfItems var
		newItem->next=stackTop;
		stackTop=newItem;
		return 1;
	}
	
	return 0;
}

template <class itemRecord> Boolean SteppingList<itemRecord>::Pop(itemRecord &theItem)
{
	ListItem<itemRecord> *temp=stackTop;
	
	if (stackTop) {
		theItem=stackTop->item;
		placeHolder=stackTop=stackTop->next;
		delete temp;
		numberOfItems--;
		return 1;
	}
	return 0;
}

template <class itemRecord> void SteppingList<itemRecord>::Delete(itemRecord theItem)
{
	ListItem<itemRecord>	*tempItem=stackTop;
	
	if (tempItem->item==theItem) {
		placeHolder=stackTop=stackTop->next;
		delete tempItem;
		numberOfItems--;
	}
	
	else {
	
		while (tempItem->next->item!=theItem && tempItem)
			tempItem=tempItem->next;
			
		if (tempItem) {
			ListItem<itemRecord>	*oldItem=tempItem->next;
			
			placeHolder=tempItem->next=tempItem->next->next;
			delete oldItem;
			numberOfItems--;
		}
	}
	
}
		
template <class itemRecord> void SteppingList<itemRecord>::DeleteAll()
{
	ListItem<itemRecord>	*temp=stackTop;
	
	while (temp) {					//	Essentially the same as the SlimSteppingList DeleteAll
		temp=stackTop->next;
		delete stackTop;
		stackTop=temp;
	}
	numberOfItems=0;
	placeHolder=stackTop;
}

template <class itemRecord> ListItem<itemRecord> * SteppingList<itemRecord>::GetTopItem(itemRecord * &theItem)
{
	if (SetToTop())
		theItem=&(placeHolder->item);
	return placeHolder;
}

template <class itemRecord> ListItem<itemRecord> * SteppingList<itemRecord>::GetPlaceItem(itemRecord * &theItem)
{
	if (placeHolder)
		theItem=&(placeHolder->item);
	return placeHolder;
}

template <class itemRecord> ListItem<itemRecord> * SteppingList<itemRecord>::GetNextItem(itemRecord * &theItem)
{
	if (placeHolder)					//	Check to make sure placeHolder is valid address
		placeHolder=placeHolder->next; 
	if (placeHolder) 					//	Check to make sure it still is, after pointing to the next
		theItem=&(placeHolder->item); 	//	If everything checks out, set second parameter to address
	return placeHolder;					//	of placeHolder's item
}

template <class itemRecord> ListItem<itemRecord> * SteppingList<itemRecord>::GetBottomItem(itemRecord * &theItem)
{
	if (placeHolder)							//	Start where we are, if we can, instead
		while (placeHolder->next)				//	of simply going to the stack top
			placeHolder=placeHolder->next;
	else if ((placeHolder=stackTop)!=nil)		//	Otherwise, go to the stack top and go down list
		while (placeHolder->next)
			placeHolder=placeHolder->next;
	if (placeHolder)
		theItem=&(placeHolder->item);
	
	return placeHolder;
}

template <class itemRecord> ListItem<itemRecord> * SteppingList<itemRecord>::GetItemX(short itemNumber, itemRecord * &theItem)
{
	short n=1;

	if (itemNumber>numberOfItems)
		return nil;
	
	SetToTop();
	while (n++!=itemNumber)
		placeHolder=placeHolder->next;
	
	theItem=&(placeHolder->item);
	
	return placeHolder;
}

template <class itemRecord> ListItem<itemRecord> * SteppingList<itemRecord>::ItemInList(itemRecord *theItem)
{
	itemRecord	*tempItem;
	if (placeHolder)
		if (placeHolder->item==*theItem)
			return placeHolder;
	if (GetTopItem(tempItem)) {
		if (*tempItem==*theItem)
			return placeHolder;
		else
			while (GetNextItem(tempItem))
				if (*tempItem==*theItem)
					return placeHolder;
	}
	return nil;
}

template <class itemRecord> ListItem<itemRecord> * SteppingList<itemRecord>::Cycle(itemRecord *currentItem, itemRecord * &nextItem)
{
	ListItem<itemRecord> * temp=ItemInList(currentItem);	//	this will find the item in the list
															//	and return a pointer to it.  More
															//	importantly, it will set the 
															//	placeHolder to the position, so we
															//	know to look for the next item
	
	if (temp) {
		if (placeHolder->next)
			placeHolder=placeHolder->next;
		else
			placeHolder=stackTop;
		nextItem=&(placeHolder->item);
	}
	
	return temp;
}

template <class itemRecord> int SteppingList<itemRecord>::NumItems()
{
	return numberOfItems;
}

/****************************************************
*													*
*	ComparisonList: 								*
*													*
****************************************************/

template <class itemRecord, class comparisonObject> Boolean ComparisonList<itemRecord, comparisonObject>::XInList(comparisonObject theObject, itemRecord * &theItem)
{
	ListItem<itemRecord>	*top;
	
	placeHolder=stackTop;
	
	if (stackTop) {
		if (stackTop->item==theObject) {
			theItem=&(stackTop->item);				//	Set theItem to the address of the item's address
			return 1;
		}
		else 
			while ((placeHolder=placeHolder->next)!=nil) {
				if (placeHolder->item==theObject) {
					theItem=&(placeHolder->item);	//	Set theItem to the address of the item's address
					return 1;
				}
			}
	}
	
	return 0;
}


/****************************************************
*													*
*	SortingList:	 								*
*													*
****************************************************/

template <class itemRecord> Boolean SortingList<itemRecord>::AddSort(itemRecord &theItem)
{
	ListItem<itemRecord> *newItem, *tempItem;
	
	placeHolder=newItem=new ListItem<itemRecord>;
	
	if (newItem) {
		numberOfItems++;
		newItem->item=theItem;
	
		ListItem<itemRecord>	*tempItem;
				
		if (stackTop) {							//	If stackTop is not nil, a stack is present
		
			if (theItem<stackTop->item) {		//	Set newItem->next to stackTop if it's less and
				newItem->next=stackTop;			//	set stackTop to newItem 
				stackTop=newItem;
			}
			
			else {
				tempItem=stackTop;				// 	Otherwise, put insert it in first spot it's less
												//	than an other item
				while (theItem>tempItem->next->item && tempItem->next)
					tempItem=tempItem->next;
				
				newItem->next=tempItem->next;
				tempItem->next=newItem;
			}
		}
		
		else {									//	Stack is created, set stackTop to newItem
			stackTop=newItem;
			newItem->next=nil;
		}
	return (1);
	}
	
	return (0);
}

