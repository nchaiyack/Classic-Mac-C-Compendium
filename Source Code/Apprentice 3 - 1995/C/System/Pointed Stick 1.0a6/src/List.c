#include "Global.h"
#include "List.h"
#include "Sorting.h"			// QuickSort
#include "Utilities.h"			// ReportError

#define kDefaultGrowSize	5


//--------------------------------------------------------------------------------

DynamicArrayHdl CreateEmptyList()
{
	DynamicArrayHdl		newList;

	newList = (DynamicArrayHdl) NewHandleClear(sizeof(DynamicArray));	
	
	return newList;
}


//--------------------------------------------------------------------------------

void	SortList(DynamicArrayHdl array)
{
	unsigned char oldState;

	oldState = HGetState((Handle) array);
	HLock((Handle) array);
	QuickSort(array);
	HSetState((Handle) array, oldState);
}


//--------------------------------------------------------------------------------

void	AppendRecord(DynamicArrayHdl array, Handle data)
{
	ArrayIndex		numRecords;
	
	numRecords = (**array).numRecords;
	
	if (numRecords > (**array).maxRecords) {
		ReportError(kCorruptList, 0);
	} else if (numRecords == (**array).maxRecords) {
		GrowArray(array, kDefaultGrowSize);
	}

	if (numRecords < (**array).maxRecords) {
		(**array).records[(**array).numRecords++] = data;
	}
}


//--------------------------------------------------------------------------------

void	DeleteRecord(DynamicArrayHdl array, ArrayIndex recordToBiteBigOne)
{
	Handle*			sourcePtr;
	Handle*			destPtr;
	ArrayIndex		recordsToMove;
	ArrayIndex		numRecords;
	
	numRecords = (**array).numRecords;

	if (numRecords > (**array).maxRecords) {
		ReportError(kCorruptList, 0);
	} else if (recordToBiteBigOne > numRecords) {
		ReportError(kArrayIndexOOB, 0);
	} else {
		DisposeHandle((**array).records[recordToBiteBigOne]);
		recordsToMove = numRecords - (recordToBiteBigOne + 1);
		if (recordsToMove > 0) {
			destPtr = &(**array).records[recordToBiteBigOne];
			sourcePtr = destPtr + 1;
			BlockMove(sourcePtr, destPtr, recordsToMove * sizeof(Handle));
		}
		(**array).numRecords--;
	}
}




//--------------------------------------------------------------------------------

#ifdef __NEVER__
void	ClearArray(DynamicArrayHdl array)
{
	ArrayIndex	index;

	for (index = (**array).numRecords-1; index >= 0; index--)
		DeleteRecord(array, index);
}
#endif


//--------------------------------------------------------------------------------

void	GrowArray(DynamicArrayHdl array, ArrayIndex recordsToAdd)
{
	ArrayIndex	newSize;

	(**array).maxRecords += recordsToAdd;
	newSize = sizeof(DynamicArray) + (**array).maxRecords * sizeof(Handle);
	SetHandleSize((Handle) array, newSize);
}


//--------------------------------------------------------------------------------

#ifdef __NEVER__
void	CompactArray(DynamicArrayHdl array)
{
	ArrayIndex	newSize;

	(**array).maxRecords = (**array).numRecords;
	newSize = sizeof(DynamicArray) + (**array).maxRecords * sizeof(Handle);
	SetHandleSize(array, newSize);
}
#endif

