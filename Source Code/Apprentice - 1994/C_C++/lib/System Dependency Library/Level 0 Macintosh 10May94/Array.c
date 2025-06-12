/* Array.c */
/*****************************************************************************/
/*                                                                           */
/*    System Dependency Library for Building Portable Software               */
/*    Macintosh Version                                                      */
/*    Written by Thomas R. Lawrence, 1993 - 1994.                            */
/*                                                                           */
/*    This file is Public Domain; it may be used for any purpose whatsoever  */
/*    without restriction.                                                   */
/*                                                                           */
/*    This package is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Debug.h"
#include "Audit.h"
#include "Definitions.h"

#include "Array.h"
#include "Memory.h"


/* Abstract data structure for maintaining a linear array of elements.  It provides */
/* indexed lookup, insertion, and deletion capabilities on the array, and attempts */
/* to provide efficient storage management */


/* when array runs out of free space at end, it is extended by ARRAYDELTACOUNT slots. */
/* when there is more than ARRAYMAXFREECOUNT free space at the end, it is reduced */
/* by ARRAYDELTACOUNT.  Therefore, ARRAYMAXFREECOUNT should be greater than */
/* ARRAYDELTACOUNT to prevent resizing the array constantly. */
#define ARRAYDELTACOUNT (32)
#define ARRAYMAXFREECOUNT (64)

struct ArrayRec
	{
		long					RealElementCount; /* actual number of elements in array */
		long					ListElementCount; /* number of slots in List */
		char**				List;
	};


/* create a new array.  returns NIL if it couldn't allocate any memory. */
ArrayRec*			NewArray(void)
	{
		ArrayRec*		Array;
		char**			List;

		Array = (ArrayRec*)AllocPtrCanFail(sizeof(ArrayRec),"ArrayRec");
		if (Array == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		List = (char**)AllocPtrCanFail(sizeof(void*) * ARRAYDELTACOUNT,"ArrayList");
		if (List == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)Array);
				goto FailurePoint1;
			}
		Array->List = List;
		Array->RealElementCount = 0;
		Array->ListElementCount = ARRAYDELTACOUNT;
		return Array;
	}


/* dispose of an array.  any pointers contained in the array are NOT disposed. */
void					DisposeArray(ArrayRec* Array)
	{
		CheckPtrExistence(Array);
		ReleasePtr((char*)Array->List);
		ReleasePtr((char*)Array);
	}


/* get the number of elements currently stored in the array */
long					ArrayGetLength(ArrayRec* Array)
	{
		CheckPtrExistence(Array);
		return Array->RealElementCount;
	}


/* get an indexed element from the array */
void*					ArrayGetElement(ArrayRec* Array, long Where)
	{
		CheckPtrExistence(Array);
		ERROR((Where < 0) || (Where >= ArrayGetLength(Array)),
			PRERR(ForceAbort,"ArrayGetElement:  Index out of range"));
		return (Array->List)[Where];
	}


/* change the value of an indexed element in the array */
void					ArraySetElement(ArrayRec* Array, void* Element, long Where)
	{
		CheckPtrExistence(Array);
		ERROR((Where < 0) || (Where >= ArrayGetLength(Array)),
			PRERR(ForceAbort,"ArraySetElement:  Index out of range"));
		(Array->List)[Where] = Element;
	}


/* insert an element into the array.  Where is the index of the element to insert */
/* BEFORE.  if Where == number of elements in the array, the element is appended. */
/* return True if successful */
MyBoolean			ArrayInsertElement(ArrayRec* Array, void* Element, long Where)
	{
		CheckPtrExistence(Array);
		ERROR((Where < 0) || (Where >/*NB*/ ArrayGetLength(Array)),
			PRERR(ForceAbort,"ArrayInsertElement:  Index out of range"));
		ERROR(Array->RealElementCount > Array->ListElementCount,
			PRERR(ForceAbort,"ArrayInsertElement:  [pre] real count > list block count"));
		if (Array->RealElementCount == Array->ListElementCount)
			{
				long				NewSize;
				char**			NewList;

				/* no free space at the end of the array, so we have to extend it */
				NewSize = Array->RealElementCount + ARRAYDELTACOUNT;
				NewList = (char**)ResizePtr((char*)(Array->List),NewSize * sizeof(void*));
				if (NewList == NIL)
					{
						return False;
					}
				Array->List = NewList;
				Array->ListElementCount = NewSize;
			}
		PRNGCHK(Array->List,&(Array->List[Where]),
			(Array->RealElementCount - Where) * sizeof(void*));
		PRNGCHK(Array->List,&(Array->List[Where + 1]),
			(Array->RealElementCount - Where) * sizeof(void*));
		MoveData((char*)&(Array->List[Where]),
			(char*)&(Array->List[Where + 1]),
			(Array->RealElementCount - Where) * sizeof(void*));
		(Array->List)[Where] = Element;
		Array->RealElementCount += 1;
		ERROR(Array->RealElementCount > Array->ListElementCount,
			PRERR(ForceAbort,"ArrayInsertElement:  [post] real count > list block count"));
		return True;
	}


/* append an element to the end of the array.  returns True if successful */
MyBoolean			ArrayAppendElement(ArrayRec* Array, void* Element)
	{
		CheckPtrExistence(Array);
		return ArrayInsertElement(Array,Element,Array->RealElementCount);
	}


/* delete an element from the array and shift any elements after it down one. */
void					ArrayDeleteElement(ArrayRec* Array, long Where)
	{
		CheckPtrExistence(Array);
		ERROR((Where < 0) || (Where >= ArrayGetLength(Array)),
			PRERR(ForceAbort,"ArrayDeleteElement:  Index out of range"));
		PRNGCHK(Array->List,&(Array->List[Where + 1]),
			(Array->RealElementCount - Where - 1) * sizeof(void*));
		PRNGCHK(Array->List,&(Array->List[Where]),
			(Array->RealElementCount - Where - 1) * sizeof(void*));
		MoveData((char*)&(Array->List[Where + 1]),(char*)&(Array->List[Where]),
			(Array->RealElementCount - Where - 1) * sizeof(void*));
		Array->RealElementCount -= 1;
		if (Array->ListElementCount - Array->RealElementCount >= ARRAYMAXFREECOUNT)
			{
				long				NewSize;
				char**			NewBlock;

				/* if this fails, well... too bad */
				NewSize = Array->ListElementCount - ARRAYDELTACOUNT;
				ERROR(NewSize < 0,PRERR(ForceAbort,
					"ArrayDeleteElement:  reducing array to negative size"));
				NewBlock = (char**)ResizePtr((char*)(Array->List),NewSize * sizeof(void*));
				if (NewBlock != NIL)
					{
						Array->List = NewBlock;
						Array->ListElementCount = NewSize;
					}
			}
	}


/* search the array to find the position of the specified element.  if the */
/* element can't be found in the array, it returns -1. */
long					ArrayFindElement(ArrayRec* Array, void* Element)
	{
		long					Scan;

		CheckPtrExistence(Array);
		for (Scan = 0; Scan < Array->RealElementCount; Scan += 1)
			{
				if ((Array->List)[Scan] == Element)
					{
						return Scan;
					}
			}
		return -1;
	}


/* return a duplicate of the array */
ArrayRec*			DuplicateArray(ArrayRec* Original)
	{
		ArrayRec*		Copy;
		long				ListSize;

		CheckPtrExistence(Original);
		Copy = (ArrayRec*)AllocPtrCanFail(sizeof(ArrayRec),"ArrayRecDup");
		if (Copy == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		ListSize = PtrSize((char*)Original->List);
		Copy->List = (char**)AllocPtrCanFail(ListSize,"ArrayListDup");
		if (Copy->List == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)Copy);
				goto FailurePoint1;
			}
		CopyData((char*)Original->List,(char*)Copy->List,ListSize);
		Copy->RealElementCount = Original->RealElementCount;
		Copy->ListElementCount = Original->ListElementCount;
		return Copy;
	}
