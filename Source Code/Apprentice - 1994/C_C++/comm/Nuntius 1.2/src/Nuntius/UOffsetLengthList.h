// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UOffsetLengthList

#define __UOFFSETLENGTHLIST__

#ifndef __XTYPES__
#include "XTypes.h"
#endif

class TOffsetLengthList : public TDynamicArray 
{
	public:		
		void InsertLast(const OffsetLength &ol);

		OffsetLength At(long index);
		long OffsetAt(long index);
		long LengthAt(long index);
		void AtPut(long index, const OffsetLength &ol);
		void AtGet(long index, OffsetLength &ol);
		
		void DoRead(TStream *aStream);
		void DoWrite(TStream *aStream);
		void DoNeedDiskSpace(long &dataForkBytes);

		TOffsetLengthList();
		pascal void Initialize();
		void IOffsetLengthList();
		pascal void Free();
};
	
