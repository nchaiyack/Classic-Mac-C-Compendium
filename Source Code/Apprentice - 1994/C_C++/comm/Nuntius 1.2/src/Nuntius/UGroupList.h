// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UGroupList.h

#define __UGROUPLIST__

enum NewArticleStat {kUnknown, kNoNew, kHasNew};

struct GroupEntry;

class TGroupList : public TDynamicArray
{
	public:
		void GetGroupAt(ArrayIndex index, CStr255 &name);
		Boolean HasGroup(const CStr255 &name);
		ArrayIndex FindIndexFromName(const CStr255 &name);

		NewArticleStat GetNewArticleStat(ArrayIndex index);
		void SetNewArticleStat(ArrayIndex index, NewArticleStat newStat);
		
		Boolean InsertGroupBefore(ArrayIndex index, const CStr255 &name); // false if existed
		Boolean AppendGroup(const CStr255 &name);
		
		ArrayIndex AddGroupsFromListBefore(ArrayIndex beforeIndex, TGroupList *groupList);
		// AddGroupsFromListBefore returns number of groups inserted
		Boolean HasOneOfTheGroupsInList(TGroupList *groupList);

		void DeleteGroupAt(ArrayIndex index);
		void DeleteGroup(const CStr255 &name);

		GroupEntry *ComputeGroupEntryAddress(ArrayIndex index);

		void DoRead(TStream *aStream);
		void DoIronAgeFormatRead(TStream *aStream);
		void DoWrite(TStream *aStream);
		void DoNeedDiskSpace(long &dataForkBytes);

		TGroupList();
		pascal void Initialize();
		void IGroupList(TDocument *doc); // may be nil
		pascal void Free();
	protected:
		TDocument *fDoc;
		
		void DoPostRead();
};
