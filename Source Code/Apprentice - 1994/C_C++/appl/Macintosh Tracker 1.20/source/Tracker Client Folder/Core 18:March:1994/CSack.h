/* CSack.h */

#pragma once

#include "CObject.h"

typedef struct SackBlock
	{
		struct SackBlock**	Next;
		short								NumElements;
		short								NumBytes;
		char								Data[];
	} SackBlock;

struct	CSack	:	CObject
	{
	 private:
		SackBlock**		FirstBlock;
		SackBlock**		NextHandleToAccess;
		short					NextIndexToAccess;
		short					BytesPerElement;
		short					MaxBytesPerBlock;
		EXECUTE(MyBoolean Initialized;)
		EXECUTE(MyBoolean Scannable;)

	 public:
		/* */				CSack();
		/* */				~CSack();
		void				ISack(long TheSizeOfElement, long TheMaxBytesPerBlock);
		void				PushElement(void* Element);
		MyBoolean		KillElement(void* Element);
		void				ResetScan(void);
		MyBoolean		GetNext(void* PlaceToPut);
		MyBoolean		GetCurrent(void* PlaceToPut);
		MyBoolean		RewriteCurrent(void* PlaceToGet);
		MyBoolean		AdvanceToNext(void);
		void				InsertSorted(void* Element, short (*Comparator)(void*,void*));
		void				KillCurrent(void);
		ulong				NumElements(void);
		MyBoolean		DoesItContain(void* Element);
	};
