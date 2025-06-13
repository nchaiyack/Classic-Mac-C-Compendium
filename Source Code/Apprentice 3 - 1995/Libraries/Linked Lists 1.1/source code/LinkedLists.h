#pragma once


/****************************************************************************************************
*																									*
*	Class ListItem is used internally by the ListClasses; you don't have to worry about it.			*
*	It is simply a template class which will hold the class you're storing in the list and will		*
*	point to the next object in the list															*
*																									*
****************************************************************************************************/




template <class itemClass> class ListItem {

public:
	itemClass	item;
	ListItem	*next;
};





/****************************************************************************************************
*																									*
*	General List Info																				*
*		¥ "itemRecord" refers to the specific variable/class/struct type 							*
*		  when an instance of this template class is created										*
*																									*
****************************************************************************************************/


/****************************************************************************************************
*																									*
*	SlimListClass																					*
*																									*
*	SlimListClass is the most basic of all the Lists, providing simple Push and Pop operations.		*
*	(It will create a stack, with only the top item accessible, no sorting involved.)				*
*																									*
*	This class will also "clean up" after itself, deallocating all the memory it allocates when		*
*	it is terminated.																				*
*																									*
*																									*
*	Function Usage																					*
*																									*
*	Boolean		Push(itemRecord &theItem);															*
*		¥ will "push" the item on to the top of the stack											*
*		¥ returns 1 if successful, 0 if not (memory couldn't be allocated)							*
*																									*
*	Boolean		Pop(itemRecord &theItem);															*	
*		¥ "pops" the top item - takes it off the stack and returns true if there is an item			*
*		 - Pop will destroy the item in memory, calling the item's destructor - Use with care!		*
*																									*
*	void		DeleteAll(); 																		*
*		¥ deletes all entries in stack																*
*		¥ also called when variable is destroyed (e.g., local variable in function dieing when		*
*		  function is done)																			*
*																									*
*																									*
****************************************************************************************************/
	




template <class itemRecord> class SlimListClass {
protected:
	ListItem<itemRecord>	*stackTop;
public:
	SlimListClass()		{ stackTop=nil; }
	~SlimListClass()	{ DeleteAll(); }
	Boolean		Push(itemRecord &theItem);
	Boolean		Pop(itemRecord &theItem);
	void		DeleteAll(); 
};





/****************************************************************************************************
*																									*
*	SteppingList																					*
*																									*
*	SteppingList is a list which can be "stepped" through, meaning you can retrieve specific		*
*	items in the list, or cycle through it.  It maintains a placeHolder in the list, meaning		*
*	you can call the GetTopItem(É) function, which sets the placeHolder to the top of the list,		*
*	then call the GetPlaceItem(É) function repeatedly to get the next items.  Items can be Pushed	*
*	on to the list (the top of the list) or Added to the bottom of the list.						*
*																									*
*	Function Usage																					*
*																									*
*	¥ Add, Push, and Pop should all be self-evident; Add adds the item to the end of the list, Push	*
*	adds the item to the top, and Pop removes the top item from the list.  Using Pop will call the 	*
*	class's destructor function, so use with care!													*
*																									*
*	Boolean	Add(itemRecord &theItem);																*
*	Boolean	Push(itemRecord &theItem);																*
*	Boolean	Pop(itemRecord &theItem);																*
*																									*
*	¥ Delete(É) will delete the specified item from the list if it can find it.  DeleteAll deletes	*
*	all the items in the list from memory															*
*																									*
*	void	Delete(itemRecord theItem);																*
*	void	DeleteAll();																			*
*																									*
*																									*
*	¥ These next five functions will retrieve pointers to specific items in this list.  MAKE SURE	*
*	THE RETURN VALUE IS NOT NIL!  If the return value is nil, the pointer is undefined.				*
*																									*
*	¥ GetTopItem(É) sets the placeHolder to the top of the stack and returns a pointer to the top	*
*	item, if there is one																			*
*																									*
*	ListItem<itemRecord>	*GetTopItem(itemRecord * &theItem);										*
*																									*
*																									*
*	¥ GetNextItem(É) gets the next item in place.  GetTopItem(É) and GetNextItem(É) are nice to use	*
*	to step through a list.  Just call GetTopItem(É) followed by GetNextItem(É) until the latter	*
*	returns nil to get all the items in the list													*
*																									*
*	ListItem<itemRecord>	*GetNextItem(itemRecord * &theItem);									*
*																									*
*																									*
*	¥ GetBottomItem(É) sets the placeHolder to the bottom and returns a pointer to the bottom item,	*
*	if there is one																					*
*																									*
*	ListItem<itemRecord>	*GetBottomItem(itemRecord * &theItem);									*
*																									*
*																									*
*	¥ GetItemX(É) will retrieve itemNumber from the list, if there is an nth item					*
*																									*
*	ListItem<itemRecord>	*GetItemX(short itemNumber, itemRecord * &theItem);						*
*																									*
*	¥ Cycle(É) will cycle from the item you pass it to the next item and retrieve a pointer to		*
*	that item; if the next item is at the beginning, it will automatically cycle around				*
*																									*
*	ListItem<itemRecord>	*Cycle(itemRecord *currentItem, itemRecord * &nextItem);				*
*																									*
*																									*
*	¥ ItemInList(É) will query whether that item is in the list										*
*																									*
*	ListItem<itemRecord>	*ItemInList(itemRecord *theItem);										*
*																									*
*																									*											*
*	¥ NumItems() returns the number of items in the list											*
*																									*
*	int		NumItems();																				*
*																									*
****************************************************************************************************/
	
	


template <class itemRecord> class SteppingList {

protected:
	ListItem<itemRecord>	*placeHolder;
	ListItem<itemRecord>	*stackTop;
	ListItem<itemRecord>	*SetToTop()		{ placeHolder=stackTop; return stackTop; }
	int	numberOfItems;
	ListItem<itemRecord>	*GetPlaceItem(itemRecord * &theItem);

public:
	SteppingList()	{ stackTop=placeHolder=nil; }
	
	Boolean	Add(itemRecord &theItem);
	Boolean	Push(itemRecord &theItem);
	Boolean	Pop(itemRecord &theItem);
	
	void	Delete(itemRecord theItem);
	void	DeleteAll();
	
	ListItem<itemRecord>	*GetTopItem(itemRecord * &theItem);
	ListItem<itemRecord>	*GetNextItem(itemRecord * &theItem);
	ListItem<itemRecord>	*GetBottomItem(itemRecord * &theItem);
	ListItem<itemRecord>	*ItemInList(itemRecord *theItem);
	
	ListItem<itemRecord>	*GetItemX(short itemNumber, itemRecord * &theItem);
	ListItem<itemRecord>	*Cycle(itemRecord *currentItem, itemRecord * &nextItem);

	int		NumItems();
};





/****************************************************************************************************
*	IMPORTANT NOTE ABOUT INHERITING TEMPLATE CLASSES:												*
*																									*
*	When you inherit template classes into another template class, you must create specific			*
*	instances of all the template classes inherited.  For example, creating instances of a			*
*	ComparisonList below would require the following:												*
*																									*
*	#pragma template SteppingList<Class1>															*
*	#pragma template ComparisonList<Class1, Class2>													*
*																									*
****************************************************************************************************/



/****************************************************************************************************
*	ComparisonList																					*
*																									*
*	ComparisonList is NOT a self-sorting list, but a list in which you can compare your				*
*	stored objects with another type of object.  For example, you may create a window class			*
*	and store a list of your windows in a linked list.  But when you get update/activate events,	*
*	you need to need to call the update function of your window class and need to find the object	*
*	based on a WindowPtr provided by the EventRecord.  To find it, you can create a ComparisonList	*
*	with you window class as the list object and a WindowPtr as the item to compare the object to.	*
*	ComparisonList is a descendent of the SteppingList and as such contains all its functions.		*
*	It also adds the function XInList(É):															*
*																									*
*																									*
*	Boolean	XInList(comparisonObject theObject, itemRecord * &theItem);								*
*		¥ passed an object to compare theItem with, XInList will return if item is in list			*
*																									*
*		¥ if the comparison object is in the list, theItem parameter will be set to a pointer to	*
*		  that object																				*
*																									*
*		¥Important note: You must have a logic comparison in the itemRecord class for the 			*
*		comparison class.  For example, suppose you create ComparisonList<myWindowClass, WindowPtr>.*
*		Within myWindowClass, you must overload the "==" operator for myWindowClass==WindowPtr		*
*		logic tests.																				*
*																									*
*		¥ a pointer is returned so that changes will be made to _that_ object; if you simply used	*
*		  a reference variable of the object, a copy of that object would be made so changes to		*
*		  the object returned would not be made to the object in the list							*
*																									*
****************************************************************************************************/





template <class itemRecord, class comparisonObject> class ComparisonList : public SteppingList<itemRecord> {

public:
	Boolean	XInList(comparisonObject theObject, itemRecord * &theItem);
};






/****************************************************************************************************
*																									*
*	SortingList																						*
*																									*
*	SortingList is a self-sorting list.  Provided you have overloaded the logic operators 			*
*	( <, >, ==, != ) in the class being stored, the list will sort itself as items are added		*
*	via AddSort(É).  It is a descendent of SteppingList.											*
*																									*
*	Boolean	AddSort(itemRecord &theItem);															*
*		¥ Using AddSort to enter all items will create an ascending list (lesser objects at			*
*		  the stack top, down to the greater items)													*
*																									*
*		¥ Important note: AddSort will sort only the piece that's entered, not the whole list.		*
*		  That is, if you Push some items on, Add others, then AddSort an item, the list will		*
*		  not be sorted, and the item will be placed after the first object it's greater than		*
*																									*
*																									*
****************************************************************************************************/

	


template <class itemRecord> class SortingList : public SteppingList<itemRecord> {

public:
	Boolean	AddSort(itemRecord &theItem);
};

