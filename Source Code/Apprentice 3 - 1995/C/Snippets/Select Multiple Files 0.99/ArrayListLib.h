#pragma once

typedef struct {
	short	elemsz;
	short	nelem;
	Handle	array;
} ArrayListType, *ArrayListPtr, **ArrayListHandle;

Boolean ArrayListValid(ArrayListHandle list);
Boolean ArrayListValidIndex(ArrayListHandle list, short index);
ArrayListHandle ArrayListBegin(short elemsz);
ArrayListHandle ArrayListEnd(ArrayListHandle list);
short ArrayListCount(ArrayListHandle list);
short ArrayListItemSize(ArrayListHandle list);
void ArrayListInsert(ArrayListHandle list, short index);
void ArrayListDelete(ArrayListHandle list, short index);
void ArrayListSet(ArrayListHandle list, short index, void *data);
void ArrayListGet(ArrayListHandle list, short index, void *data);
void *ArrayListGetHandle(ArrayListHandle list);
void ArrayListDetachHandle(ArrayListHandle list);
void ArrayListAttachHandle(ArrayListHandle list, void *array);
