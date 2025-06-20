
#ifndef __LIST__
#define __LIST__

typedef short ArrayIndex;
typedef enum {kSelectionSort, kInsertionSort, kBubbleSort, kShellSort, kQuickSort,
				kHeapSort} SortType;

typedef struct {
	ArrayIndex		numRecords;
	ArrayIndex		maxRecords;
	Handle			records[];
} DynamicArray, **DynamicArrayPtr, **DynamicArrayHdl;

typedef Boolean	(*ElementProc)(Handle entry, void* yourData);

DynamicArrayHdl		CreateEmptyList(void);
void				SortList(DynamicArrayHdl array);
void				AppendRecord(DynamicArrayHdl array, Handle data);
void				InsertRecord(DynamicArrayHdl array, Handle data);
void				InsertRecordBefore(DynamicArrayHdl array, Handle data, ArrayIndex);
void				DeleteRecord(DynamicArrayHdl array, ArrayIndex recordToBiteBigOne);
void				ClearArray(DynamicArrayHdl list);
void				GrowArray(DynamicArrayHdl array, ArrayIndex recordsToAdd);
void				CompactArray(DynamicArrayHdl array);
void				SetSortType(SortType);

#endif
