///////
////// Array List Library ¥ Original by Ari Halberstadt from "Winter Shell"
///// Modified to by independent of the other parts of Winter Shell by Eddy J. Gurney
//// These functions provide a useful set of routines for adding, removing and 
/// retrieving an arbitrary list of data
//

#include "Select Multiple Files.h"
#include "ArrayListLib.h"

/* true if a valid list */
Boolean ArrayListValid(ArrayListHandle list)
{
	if ((list != nil) && (((long)list & 1) == 0) &&
			(*list != nil) && (((long)*list & 1) == 0) &&
			(GetHandleSize((Handle)list) >= sizeof(ArrayListType)) &&
			(GetHandleSize((Handle)(**list).array) == (**list).nelem * (**list).elemsz))
		return true;
	else
		return false;
}

/* true if a valid index into the array */
Boolean ArrayListValidIndex(ArrayListHandle list, short index)
{
	if (ArrayListValid(list))
		return index >= 0 && index < (**list).nelem;
	else
		return false;
}

/* create a new list */
ArrayListHandle ArrayListBegin(short elemsz)
{
	ArrayListHandle	list = nil;
	Handle				array = nil;
	
	list = (ArrayListHandle)NewHandleClear(sizeof(ArrayListType));
	if (list == nil)
		ErrorHandler("\pNo memory!");
	else {	
		array = NewHandle(0);
		if (array == nil)
			ErrorHandler("\pNo memory!");
		else {
			(**list).array = array;
			(**list).elemsz = elemsz;
		}
	}
	
	if (!ArrayListValid(list))
		ErrorHandler("\pArray List Error!");

	return list;
}

/* dispose of the list */
ArrayListHandle ArrayListEnd(ArrayListHandle list)
{
	if (list == nil || !ArrayListValid(list))
		ErrorHandler("\pArray List Error!");

	if ((**list).array != nil)
		DisposeHandle((**list).array);
	DisposeHandle((Handle)list);
	list = nil;
}

/* return number of items in array */
short ArrayListCount(ArrayListHandle list)
{
	if (!ArrayListValid(list))
		ErrorHandler("\pArray List Error!");

	return (**list).nelem;
}

/* return size of items in array */
short ArrayListItemSize(ArrayListHandle list)
{
	if (!ArrayListValid(list))
		ErrorHandler("\pArray List Error!");

	return (**list).elemsz;
}

/* insert space for an item before the indexed item */
void ArrayListInsert(ArrayListHandle list, short index)
{
	SignedByte	state;
	
	if (!ArrayListValid(list))
		ErrorHandler("\pArray List Error!");

	if (index >= 0 && index <= (**list).nelem) {
		state = HGetState((Handle)list);
		HNoPurge((Handle)list);
		if (MemError() != noErr)
			ErrorHandler("\pNo memory!");

		SetHandleSize((**list).array, ((**list).nelem + 1) * (**list).elemsz);
		if (MemError() != noErr)
			ErrorHandler("\pNo memory!");
			
		HSetState((Handle)list, state);
		if (MemError() != noErr)
			ErrorHandler("\pNo memory!");
	}
	if (index < (**list).nelem) {
		BlockMove(*(**list).array + index * (**list).elemsz,
				 	 *(**list).array + (index + 1) * (**list).elemsz,
					 ((**list).nelem - index) * (**list).elemsz);
	}
	(**list).nelem++;

	if (!ArrayListValidIndex(list, index))
		ErrorHandler("\pArray List Error!");
}

/* remove the indexed item */
void ArrayListDelete(ArrayListHandle list, short index)
{
	SignedByte	state;

	if (!ArrayListValid(list) || !ArrayListValidIndex(list, index))
		ErrorHandler("\pArray List Error!");

	if (index < (**list).nelem - 1) {
		BlockMove(*(**list).array + (index + 1) * (**list).elemsz,
					 *(**list).array + index * (**list).elemsz,
					 ((**list).nelem - index - 1) * (**list).elemsz);
	}

	state = HGetState((Handle)list);
	HNoPurge((Handle)list);
	if (MemError() != noErr)
		ErrorHandler("\pNo memory!");

	SetHandleSize((**list).array, --(**list).nelem * (**list).elemsz);
	if (MemError() != noErr)
		ErrorHandler("\pNo memory!");
		
	HSetState((Handle)list, state);
	if (MemError() != noErr)
		ErrorHandler("\pNo memory!");
	
	if (!ArrayListValid(list))
		ErrorHandler("\pArray List Error!");
}

/* set the value of the indexed item */
void ArrayListSet(ArrayListHandle list, short index, void *data)
{
	if (!ArrayListValid(list) || !ArrayListValidIndex(list, index))
		ErrorHandler("\pArray List Error!");

	BlockMove(data, *(**list).array + index * (**list).elemsz, (**list).elemsz);

	if (!ArrayListValid(list))
		ErrorHandler("\pArray List Error!");
}

/* get the value of the indexed item */
void ArrayListGet(ArrayListHandle list, short index, void *data)
{
	if (!ArrayListValid(list) || !ArrayListValidIndex(list, index))
		ErrorHandler("\pArray List Error!");

	BlockMove(*(**list).array + index * (**list).elemsz, data, (**list).elemsz);

	if (!ArrayListValid(list))
		ErrorHandler("\pArray List Error!");
}

/* return the handle to the array */
void *ArrayListGetHandle(ArrayListHandle list)
{
	if (!ArrayListValid(list))
		ErrorHandler("\pArray List Error!");

	return (**list).array;
}

/* detach the array from the list (call ArrayListGetHandle first) */
void ArrayListDetachHandle(ArrayListHandle list)
{
	if (!ArrayListValid(list))
		ErrorHandler("\pArray List Error!");

	(**list).array = NewHandle(0);
	if ((**list).array == nil)
		ErrorHandler("\pNo memory!");
	(**list).nelem = 0;

	if (ArrayListCount(list) != 0)
		ErrorHandler("\pArray List Error!");
}

/* attach the handle to the array */
void ArrayListAttachHandle(ArrayListHandle list, void *array)
{
	if (!ArrayListValid(list))
		ErrorHandler("\pArray List Error!");

	if (array != nil && (long)array & 1 == 0) {
		if ((**list).array != nil)
			DisposeHandle((**list).array);
		(**list).array = array;
		(**list).nelem = GetHandleSize(array) / (**list).elemsz;
	} else
		ErrorHandler("\pArray List Error!");

	if (!ArrayListValid(list))
		ErrorHandler("\pArray List Error!");
}
