/*----------------------------------------------------------------------------

	ldefstub.c

	This module contains a stub for NewsWatcher's LDEF.
	
	This module is compiled by the ldef.� project to build the stand-alone
	LDEF 128 resource.
	
	This is only a very small stub used to call the main LDEF function in
	the ldef.c module, which is part of the main newswatcher.� project.
	
	A pointer to the main LDEF function must be stored in the list record's
	refcon field.
	
	Using the stub has two advantages:
	
	1. The main LDEF function can use global variables, call other functions,
	and in general enjoy all the privileges of a "real" function as opposed
	to a function in a stand-alone code resource.
	
	2. We don't have to remember to rebuild the stand alone LDEF resource
	whenever we make changes to the data structures in glob.h.
	
----------------------------------------------------------------------------*/



typedef void (*listDefFuncType) (short, Boolean, Rect *, Cell,
	short, short, ListHandle);


pascal void	main (short lMessage, Boolean lSelect, Rect *lRect, Cell lCell,
	short lDataOffset, short lDataLen, ListHandle lHandle)
{
	listDefFuncType listDefFunc;
	
	if (lMessage == lInitMsg) return;
	listDefFunc = (listDefFuncType)(**lHandle).refCon;
	(*listDefFunc)(lMessage, lSelect, lRect, lCell, lDataOffset, lDataLen, lHandle);
}

