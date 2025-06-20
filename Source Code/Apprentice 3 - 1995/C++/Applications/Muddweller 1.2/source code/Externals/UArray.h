/* UArray.p */
/* Copyright � 1989-90 by Apple Computer, Inc. All rights reserved.*/

#ifndef  __UArray__
#define __UArray__

/*	This unit implements a dynamically-sized object, called TIntegerArray, which is
	an array of integers.  Each TIntegerArray can hold up to 32767 integers.

	Methods are provided for creating and initializing, as well as adding and
	accessing elements of IntegerArrays.  No methods have been written for
	deleting elements.
*/

#ifndef  __UObject__
#include "UObject.h"
#endif

#ifndef  __UFailure__
#include "UFailure.h"
#endif

#ifndef  __Packages__
#include "Packages.h"
#endif


const short kElementSize 	= 2;		/* size of one element (integer) */
const short kIncElements	= 2;		/* number of elements to grow by */

		/* Constants for NewInitializedArray�s zero parameter */
const Boolean kZeroValue	= TRUE;		/* initialize all elements to 0 */
const Boolean kIntegerValue	= FALSE;	/* initialize elements to 1, 2, 3, ... */

		/* Constant for TIntegerArray.SortBy�s CompareItems function */
const short kFirstGreaterThanSecond	=  1;


class TIntegerArray	: public TObject {
	public:
	short fSize;		/* the number of valid elements */
	/* fIntegers:	dynamic ARRAY[1..n] of INTEGER */

	virtual pascal void IArray(short elements);
			/* Initialize the object to hold the specified number of elements,
			  and set fSize to 0. */

	virtual pascal void AddElement(short value);
			/* Add an element to the array with the given value. */

	virtual pascal void AddUnique(short value);
			/* Add the specified value only if it is not yet in the array */

	virtual pascal short At(short index);
			/* Return the element at the given index. */

	virtual pascal void AtPut(short index, short value);
			/* Change the element at the given index to value. */

	virtual pascal Boolean Contains(short value);
			/* Return TRUE if the array contains the given value. */

	virtual pascal short GetSize();
			/* Return the number of valid elements. */

	virtual pascal void SortBy(pascal short (*CompareItems)(short index1, short index2,
		void *CompareItems_StaticLink), void *CompareItems_StaticLink);
			/* Sort the elements using the given compare routine. */

	virtual pascal void Fields(pascal void (*DoToField)(StringPtr fieldName, Ptr fieldAddr, short 
	   fieldType, void *DoToField_StaticLink), void *DoToField_StaticLink);
};


extern pascal TIntegerArray *NewIntegerArray(short initialSize);
	/* Create a TIntegerArray big enough to hold initialSize elements */

extern pascal TIntegerArray *NewInitializedArray(short elements, Boolean zero);
	/* Create a TIntegerArray with the specified number of elements, whose values are
	  initialized to either 0 (zero=T) or consecutive integers 1,2,3,... (zero=F). */

#endif
