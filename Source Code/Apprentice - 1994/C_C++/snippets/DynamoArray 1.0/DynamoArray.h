/*
	DynamoArray.h

	Version 1.0
	(c)1993, 1994 by Hiep Dam. All Rights Reserved.
	3G Software
	Blah, blah, blah.

	---------------------------------------------------------------------------

	December 21, 1993.
	Last update: May 4, 1994.
	Just for your information, I spent 5 hours one night developing this class
	(from midnight to 5 in the morning. I must be crazy).

	---------------------------------------------------------------------------

	Dynamic Arrays implemented as a C++ class. Pretty self-explanatory.
	Reason for this class? Well, I needed to maintain a dynamic list of items
	that were to be randomly chosen, one item at a time. However, once that
	item was chosen, it was not to be chosen again. The best way to do
	this was via a dynamic array, getting the item and then deleting it
	and then shrinking the array by 1.
	Note however that in the implementation I don't actually "shrink" the array;
	I just decrement the array size counter.
	The array is allocated via _new.
	
	This is also a good sample to work from if you're new to C++ and OOP.

	---------------------------------------------------------------------------

	Updated: 12/24/93
	         Added [] operator and fixed minor bug in destructor.
	         4/1/94
	         Added copy constructor, assignment constructor
*/


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef DYNAMOARRAY_H_
#define DYNAMOARRAY_H_

typedef long DynamoArrayType;		// Change the type here for your own use

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

const short STD_DYNAMO_LEN = 10;	// The standard chunk length the array grows by
const short OUT_OF_BOUNDS = -1;		// Error code returned by the methods

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



// IMPORTANT:
// -----------
// When using arrays, it is always tricky dealing with indices.
// Remember that arrays start with 0, as in anArray[0]. This is different
// from Pascal, where the first item starts at 1, as in anArray[1]. So in C and
// C++ the correct boundaries are anArray[0] to anArray[x - 1], where x is the
// number of items in the array. In Pascal this would be anArray[1] to
// anArray[x]. To clarify things, I use two terms: whole values and index values.
// Whole values range from 1 to the number of items, and index values
// range from 0 to the number of items - 1. So there. No reason for anyone
// to get confused anymore, is there?



typedef class DynamoArray {
	protected:
		short fMaxSize;				// Current length of array	(it's maximum size).
		short fCurSize;				// Number of items in array currently.
		DynamoArrayType *fArray;	// The array itself (actually pointer to the array)
									// You can change what the array is by changing
									// the definition of DynamoArrayType. By default
									// it's just a short (integer).

		unsigned short GetRandom(unsigned short min, unsigned short max);
		void IncreaseSize();

	public:
		DynamoArray() {
			// Default constructor; no arguments.
			// This constructor just creates an array with the default size
			// of STD_DYNAMO_LEN (i.e. 10). You can change the standard
			// default size by changing STD_DYNAMO_LEN, if you wish.
			fMaxSize = STD_DYNAMO_LEN;
			fCurSize = 0;
			fArray = new DynamoArrayType[fMaxSize];
		} // END constructor

		DynamoArray(short size) {
			if (size < 1) {
				fMaxSize = STD_DYNAMO_LEN;
				fCurSize = 0;
				fArray = new DynamoArrayType[fMaxSize];
			}
			else {
				fMaxSize = size;
				fCurSize = 0;
				fArray = new DynamoArrayType[fMaxSize];
			}
		} // END constructor

		~DynamoArray() {
			if (fArray != nil)
				delete []fArray;
		} // END destructor
		
		// Copy constructor
		DynamoArray(DynamoArray& sourceArray);
		
		// Assignment operator
		DynamoArray operator=(const DynamoArray& sourceArray);

		// [].
		// Bracket operator; pass a valid index value (0 to fCurSize - 1);
		DynamoArrayType *operator[] (short index);

		// SizeOf.
		// Get how many items are stored in array.
		// Returns a whole value, from 1 to x.
		virtual short SizeOf() { return fCurSize; }

		// Get.
		// Specify an index number from 0 to fCurSize - 1. Returns OUT_OF_BOUNDS
		// (-1) if the array is empty; else the index number of the item chosen.
		virtual short Get(DynamoArrayType& theItem, short index);
		
		// RandomGet.
		// Get a random item from the array. The item is NOT deleted from
		// the array. Returns the index value of the item randomly chosen,
		// OUT_OF_BOUNDS (-1) if array is empty.
		virtual short RandomGet(DynamoArrayType& theItem);
		
		// Append.
		// Inserts the specified item at the end of the array. Returns the index
		// value of the item; If the array is initially empty, Append() will
		// create the array itself.
		virtual short Append(const DynamoArrayType& theItem);

		// Insert.
		// Inserts the specified item into the array at the specified location.
		// Valid locations are from 0 to fCurSize. Using fCurSize as
		// the location will result the same as Append().
		// Returns OUT_OF_BOUNDS if error occured and unable to insert item; if
		// able to insert item, returns 0.
		virtual short Insert(const DynamoArrayType& theItem, short index);

		// Delete.
		// Delete the specified item at array index.
		// Specify an item number from 0 to fCurSize - 1.
		// If the array is empty, Delete() returns OUT_OF_BOUNDS; else
		// the index value of the item deleted.
		virtual short Delete(short index);

		// SearchDelete.
		// Remove the specified item in the array, if that item
		// exists. Basically a call to Search(item, x) and then calling Delete(x).
		// Returns OUT_OF_BOUNDS (-1) if item not in array, else the index of the
		// item deleted (from 0 to fCurSize - 1).
		virtual short SearchDelete(DynamoArrayType& theItem, short startIndex = 0);
		
		// Remove.
		// Get the specified item from the array, and then delete that item.
		// Almost the same as calling Get(x) and then calling Delete(x).
		// However, you must pass to Remove the variable that is to hold the
		// deleted item. Item specified should be from 0 to fCurSize - 1.
		// Returns OUT_OF_BOUNDS (-1) if the array is empty, else
		// the array index of the item removed.
		virtual short Remove(DynamoArrayType& theItem, short index);
		
		// RandomRemove.
		// Same as Remove(), except item removed is randomly chosen.
		virtual short RandomRemove(DynamoArrayType& theItem);

		// Search.
		// Looks for specified item in the array.
		// Returns OUT_OF_BOUNDS (-1) if not found, else 0 to fCurSize - 1.
		// If your DynamoArrayType is a complex data structure (such as a class)
		// it is recommended you overload the == (equality) operator.
		virtual short Search(const DynamoArrayType& source, short startIndex = 0);
}; // END DynamoArray

// END DynamoArray.h

#endif // DYNAMOARRAY_H_