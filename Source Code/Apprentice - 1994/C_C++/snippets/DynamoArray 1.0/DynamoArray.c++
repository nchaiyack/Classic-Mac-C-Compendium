/*
	DynamoArray.c++

	Version 1.0
	(c)1993, 1994 by Hiep Dam. All Rights Reserved.
	3G Software
	Blah, blah, blah.
	Usage: Public domain. Feel free to exploit it to your own needs; it would
	       be nice of you, however, to give me some tiny credits if you decide
	       to use this in your application or if the code helps you...

	---------------------------------------------------------------------------

	December 21, 1993.
	Last update: May 4, 1994.

*/

//-------------------------------------------------------------

#include "DynamoArray.h"

//-------------------------------------------------------------


void DynamoArray::IncreaseSize() {
	// Increase array length/size
	fMaxSize += STD_DYNAMO_LEN;

	// Now create newly-sized array
	DynamoArrayType *temp = new DynamoArrayType[fMaxSize];

	// Copy items from old array into newly-created array!
	for (short i = 0; i < fCurSize; i++)
		temp[i] = fArray[i];

	// Delete original copy.
	delete fArray;

	fArray = temp;
} // END IncreaseSize

//-------------------------------------------------------------

// Copy Constructor

DynamoArray::DynamoArray(DynamoArray& sourceArray) {
	if (&sourceArray == this) return;	// Don't copy with references to self

	fCurSize = sourceArray.fCurSize;
	fMaxSize = 0;

	while (fMaxSize < fCurSize)
		fMaxSize += STD_DYNAMO_LEN;	// Increase it in chunks of STD_DYNAMO_LEN

	fArray = new DynamoArrayType[fMaxSize];

	BlockMove((Ptr)&sourceArray.fArray[0], (Ptr)&fArray[0],
		sizeof(DynamoArrayType) * fCurSize);
} // END Copy constructor

//-------------------------------------------------------------

// Assignment operator (overloaded)

DynamoArray DynamoArray::operator=(const DynamoArray& sourceArray) {
	// Make a copy, so trash whatever we have currently. Unfortunately
	// we can't make a call to the destructor, so we have to do this manually...
	delete []fArray;

	// Now make a new array
	fArray = new DynamoArrayType[fCurSize = sourceArray.fCurSize];

	// Do the actual copy...
	BlockMove((Ptr)&sourceArray.fArray[0], (Ptr)&fArray[0],
		sizeof(DynamoArrayType) * fCurSize);

	return(*this);	// return a copy of this object
} // END Assignment operator

//-------------------------------------------------------------

DynamoArrayType *DynamoArray::operator[] (short index) {
	if ((index < 0) || (index >= fCurSize))
		return nil;
	else
		return &fArray[index];
} // END operator[]
		
//-------------------------------------------------------------

short DynamoArray::Get(DynamoArrayType& theItem, short index) {
	if ((index < 0) || (index >= fCurSize) || (fArray == nil))
		// Check for correct boundaries...
		return(OUT_OF_BOUNDS);
	else {
		theItem = fArray[index];
		return(index);
	}
} // END GetItem

//-------------------------------------------------------------

short DynamoArray::RandomGet(DynamoArrayType& theItem) {
	if (fCurSize > 0) {
		// Get a random value.
		short randomVal = GetRandom(0, fCurSize - 1);
		// Now retrieve the item.
		short returnVal = Get(theItem, randomVal);
		
		// Did something go wrong? If no...
		if (returnVal >= 0)
			return(randomVal);
		else
			// Ooops, something went wrong; return error value.
			return(returnVal);
		}
	else
		// Oops, array empty.
		return(OUT_OF_BOUNDS);
} // END RandomGet

//-------------------------------------------------------------

short DynamoArray::Append(const DynamoArrayType& theItem) {
	if (fArray == nil) {
		// If array empty, create new one.
		fMaxSize = STD_DYNAMO_LEN;
		fCurSize = 1;
		fArray = new DynamoArrayType[fMaxSize];
	}
	else {
		if (fCurSize == fMaxSize)
			// Ooh, we reached the maximum size. Time to enlarge
			// the array.
			IncreaseSize();
		// We're going to add an item, so increment fCurSize
		fCurSize++;
	}

	// Put item into the array.
	fArray[fCurSize - 1] = theItem;
	// Return index value of item we appended into array.
	return(fCurSize - 1);
} // END Append

//-------------------------------------------------------------

short DynamoArray::Insert(const DynamoArrayType& theItem, short index) {
	if ((index < 0) || (index > fCurSize) || (fArray == nil))
		return(OUT_OF_BOUNDS);
	
	if (fCurSize + 1 == fMaxSize)
		// Reached maximum size, so enlarge the array.
		IncreaseSize();

	// OK, make room for our new item! Shift everything down from
	// location on to the end of the array.
	for (short i = fCurSize - 1; i >= index; i--)
		fArray[i + 1] = fArray[i];
	
	// Add our new item.
	fArray[index] = theItem;

	// We're adding an item, so increment the number of items in array.
	fCurSize++;
	
	return(0);	// Everything OK, so return zero value.
} // END Insert

//-------------------------------------------------------------

short DynamoArray::Delete(short index) {
	if ((index < 0) || (index >= fCurSize) || (fArray == nil))
		return(OUT_OF_BOUNDS);
	
	// Shift array backwards (down to 0), so we don't lose anything.
	// If user specified last item is the item to be deleted, this loop
	// won't execute. We just decrement fCurSize instead.
	for (short i = index + 1; i < fCurSize; i++) 
		fArray[i - 1] = fArray[i];

	fCurSize--;
	return(index);
} // END Delete

//-------------------------------------------------------------

short DynamoArray::SearchDelete(DynamoArrayType& theItem, short startIndex) {
	short val = OUT_OF_BOUNDS;	// Assume did not find item

	// Search for the item by value, not by index.
	val = Search(theItem, startIndex);
	if (val >= 0)
		Delete(val);	// We found it, so delete!

	return val;
} // END SearchDelete

//-------------------------------------------------------------

short DynamoArray::Remove(DynamoArrayType& theItem, short index) {
	if ((index < 0) || (index >= fCurSize) || (fArray == nil))
		return(OUT_OF_BOUNDS);
	else {
		// In previous versions, Remove() just returned the address of the item
		// that was "deleted". Problem was, the address was no longer pointing
		// to the item that was deleted, but pointed to the item replacing
		// the deleted item!!
		if (Get(theItem, index) != OUT_OF_BOUNDS) {
			if (Delete(index) != OUT_OF_BOUNDS) {
				return(index);
			}
		}
		return(OUT_OF_BOUNDS);
	}
} // END Remove

//-------------------------------------------------------------

short DynamoArray::RandomRemove(DynamoArrayType& theItem) {
	return Remove(theItem, GetRandom(0, fCurSize - 1));
} // END RandomRemove

//-------------------------------------------------------------

short DynamoArray::Search(const DynamoArrayType& source, short startIndex) {
	// Assume we haven't found our baby...
	short foundItem = OUT_OF_BOUNDS;

	if (startIndex >= fCurSize)
		return(foundItem);

	for (short i = startIndex; i < fCurSize; i++) {
		if (fArray[i] == source) {
			// Yeah, found it. Set return value and exit loop.
			foundItem = i;
			break;
		}
	}
	return(foundItem);
} // END Search


// -----------------------------------------------------------------------------

// The following routine was "lifted" from Tony Myles' SpriteWorld Package,
// in GameUtils.c
// Great code in SpriteWorld, by the way.


//	By:	Tony Myles
//	Date:	9/19/90
//	Copyright: � 1991-93 Tony Myles, All rights reserved worldwide.

//	GetRandom
//	generate a random number between min and max inclusive
unsigned short DynamoArray::GetRandom(unsigned short min, unsigned short max)
{
	unsigned short random;
	long range, temp;

	random = Random();
	range = (max - min) + 1;
	temp = (random * range) / 65536;
	random = temp + min;

	return random;
}

// -----------------------------------------------------------------------------


// END DynamoArray.c++