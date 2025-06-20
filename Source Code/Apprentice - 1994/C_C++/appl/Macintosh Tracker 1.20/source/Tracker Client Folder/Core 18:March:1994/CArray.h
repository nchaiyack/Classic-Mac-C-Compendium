/* CArray.h */

#pragma once

#include "CObject.h"

typedef struct arrayd
	{
		struct arrayd**		Next;
		long							NumElements;
		char							Data[];
	} ArrayType;

struct	CArray	:	CObject
	{
		long					BytesPerElement;
		long					ElementsPerArray;
		ArrayType**		FirstBlock;

		/* */				CArray();
		/* */				~CArray();
		void				IArray(long TheBytesPerElement, long TheElementsPerArray);
		long				GetNumElements(void);
		long				AppendElement(void);
		long				InsertElement(long IndexToInsertBefore);
		MyBoolean		DeleteElement(long Index);
		MyBoolean		GetElement(long Index, void* PlaceToPut);
		MyBoolean		PutElement(long Index, void* PlaceToGet);
		void*				GetElementAddress(long Index);
		MyBoolean		KillElement(void* Element);
	};
