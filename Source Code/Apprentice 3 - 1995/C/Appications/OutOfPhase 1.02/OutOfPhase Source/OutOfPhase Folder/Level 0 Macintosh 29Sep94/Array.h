/* Array.h */

#ifndef Included_Array_h
#define Included_Array_h

/* Array depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */

/* Abstract data structure for maintaining a linear array of elements.  It provides */
/* indexed lookup, insertion, and deletion capabilities on the array, and attempts */
/* to provide reasonably efficient storage management */

struct ArrayRec;
typedef struct ArrayRec ArrayRec;

/* create a new array.  returns NIL if it couldn't allocate any memory. */
ArrayRec*			NewArray(void);

/* create a new array with preallocated space for a bunch of elements. */
ArrayRec*			NewArrayReserveSpace(long NumElements);

/* dispose of an array.  any pointers contained in the array are NOT disposed. */
void					DisposeArray(ArrayRec* Array);

/* get the number of elements currently stored in the array */
long					ArrayGetLength(ArrayRec* Array);

/* get an indexed element from the array */
void*					ArrayGetElement(ArrayRec* Array, long Where);

/* change the value of an indexed element in the array */
void					ArraySetElement(ArrayRec* Array, void* Element, long Where);

/* insert an element into the array.  Where is the index of the element to insert */
/* BEFORE.  if Where == number of elements in the array, the element is appended. */
/* return True if successful */
MyBoolean			ArrayInsertElement(ArrayRec* Array, void* Element, long Where);

/* append an element to the end of the array.  returns True if successful */
MyBoolean			ArrayAppendElement(ArrayRec* Array, void* Element);

/* delete an element from the array and shift any elements after it down one. */
void					ArrayDeleteElement(ArrayRec* Array, long Where);

/* search the array to find the position of the specified element.  if the */
/* element can't be found in the array, it returns -1. */
long					ArrayFindElement(ArrayRec* Array, void* Element);

/* return a duplicate of the array */
ArrayRec*			DuplicateArray(ArrayRec* Original);

#endif
