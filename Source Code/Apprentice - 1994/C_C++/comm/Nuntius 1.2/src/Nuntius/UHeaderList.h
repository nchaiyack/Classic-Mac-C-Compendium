// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UHeaderList.h

#define __UHEADERLIST__

struct HandleOffsetLength;
class TOffsetLengthList;

class THeaderList : public TObject
{
	public:
		void AtGet(long id, HandleOffsetLength &hol);
		void AddNewHeaders(Handle h, long groupFirstID, 
																	long newFirstID, long newLastID);

		void DoRead(TStream *aStream);
		void DoWrite(TStream *aStream);
		void DoNeedDiskSpace(long &dataForkBytes);
		
		void DeleteAll();

		Boolean SanityCheck();
		void DebugDump(Boolean verbose);
		
		THeaderList();
		pascal void Initialize();
		void IHeaderList();
		pascal void Free();
	private:
		long fFirstID, fLastID;
		Handle fDataH;
		TOffsetLengthList *fIndexList;
};

THeaderList *NewHeaderList();
