#include "Sorting.h"
#include "Remember.h"				// FileObjectHdl
#include "PLStringFuncs.h"			// PLstrcompare

void				SiftUp(ArrayIndex guyToMove);
void				SiftDown(ArrayIndex arraySize);

void				QuickSort1(ArrayIndex left, ArrayIndex right);

short				Compare(ArrayIndex index1, ArrayIndex index2);
void				Swap(ArrayIndex index1, ArrayIndex index2);
short				CompareFileObjects(FileObjectHdl obj1, FileObjectHdl obj2);

DynamicArrayHdl		pArray;


#ifdef __NEVER__
//================================================================================
//	HeapSort
//================================================================================

void	HeapSort(DynamicArrayHdl dynArray)
{
	ArrayIndex	i;
	ArrayIndex	arraySize;

	arraySize = (**dynArray).numRecords;
	pArray = dynArray;

	for (i = 1; i < arraySize; i++)
		SiftUp(i);
	for (i = arraySize-1; i >= 1; i--) {
		Swap(1, i);
		SiftDown(i-1);
	}
}

//--------------------------------------------------------------------------------

void	SiftUp(ArrayIndex guyToMove)
{
	ArrayIndex		index;
	ArrayIndex		parentIndex;
	ArrayIndex		result;
	Handle			tempHandle;
	
	index = guyToMove;
	while (index != 1) {
		parentIndex = index >> 1;		// divide by 2 to get parent index
		result = Compare(parentIndex, index);
		if (result <= 0)
			break;
		Swap(parentIndex, index);
		index = parentIndex;
	}
}

//--------------------------------------------------------------------------------

void	SiftDown(ArrayIndex arraySize)
{
	ArrayIndex		index;
	ArrayIndex		childIndex;
	ArrayIndex		result;
	
	index = 1;
	childIndex = 2;
	while (childIndex <= arraySize) {
		if (childIndex < arraySize) {
			result = Compare(childIndex + 1, childIndex);
			if (result < 0)
				childIndex++;
		}
		result = Compare(index, childIndex);
		if (result <= 0)
			break;
		Swap(childIndex, index);
		index = childIndex;
		childIndex <<= 1;				// times by two to get child
	}
}
#endif


//================================================================================
//	QuickSort
//================================================================================

void	QuickSort(DynamicArrayHdl dynArray)
{
	pArray = dynArray;
	QuickSort1(0, (**dynArray).numRecords);
}

void	QuickSort1(ArrayIndex left, ArrayIndex right)
{
	ArrayIndex	leftToRight;
	ArrayIndex	rightToLeft;
 
	while (right - left > 1) {
		leftToRight = left;
		rightToLeft = right;
		for (;;) {
			while (++leftToRight < right && Compare(leftToRight, left) < 0) ;
			while (--rightToLeft > left && Compare(rightToLeft, left) > 0) ;
			if (leftToRight >= rightToLeft)
 				break;
			Swap(leftToRight, rightToLeft);
		}
		if (rightToLeft == left) {
			++left;
			continue;
		}
		Swap(left, rightToLeft);
		if (rightToLeft - left < right - (rightToLeft + 1)) {
			QuickSort1(left, rightToLeft);
			left = rightToLeft + 1;		// QuickSort1(rightToLeft + 1, right);
		}
		else {
			QuickSort1(rightToLeft + 1, right);
			right = rightToLeft;		// QuickSort1(left, rightToLeft);
		}
	}
}


//================================================================================
//	Utilities
//================================================================================

short	Compare(ArrayIndex index1, ArrayIndex index2)
{
	return CompareFileObjects( (FileObjectHdl) (**pArray).records[index1],
								(FileObjectHdl) (**pArray).records[index2]);
}


//--------------------------------------------------------------------------------

void	Swap(ArrayIndex index1, ArrayIndex index2)
{
	Handle		tempHandle;
	Handle*		hPtr1;
	Handle*		hPtr2;

	hPtr1 = &(**pArray).records[index1];
	hPtr2 = &(**pArray).records[index2];

	tempHandle = *hPtr1;
	*hPtr1 = *hPtr2;
	*hPtr2 = tempHandle;
}

//--------------------------------------------------------------------------------
/*
*/

short	CompareFileObjects(FileObjectHdl obj1, FileObjectHdl obj2)
{
	short			result;
	unsigned long	date1;
	unsigned long	date2;
	StringHandle	name1;
	StringHandle	name2;
	Boolean			perm1;
	Boolean			perm2;
	
	if ((gSortType & kSeparatePermObjects) != 0) {
		perm1 = (**obj1).permanent;
		perm2 = (**obj2).permanent;
		
		if (perm1 && !perm2)
			return -1;
		if (!perm1 && perm2)
			return 1;
	}
	
	switch (gSortType & kSortByMask) {

		case kSortByName:
			name1 = (**obj1).fName;
			name2 = (**obj2).fName;

			HLock((Handle) name1);
			HLock((Handle) name2);
		
			result = PLstrcompare(*name1, *name2);
		
			HUnlock((Handle) name1);
			HUnlock((Handle) name2);

			return result;

		case kSortByAge:
			date1 = (**obj1).timeAdded;
			date2 = (**obj2).timeAdded;

			if (date1 > date2)
				return -1;
			if (date1 < date2)
				return 1;
			return 0;
	}
}
