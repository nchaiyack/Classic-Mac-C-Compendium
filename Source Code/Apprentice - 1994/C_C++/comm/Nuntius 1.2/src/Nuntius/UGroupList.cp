// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UGroupList.cp

#include "UGroupList.h"
#include "UPrefsDatabase.h"
#include "Tools.h"
#include "StreamTools.h"

#include <RsrcGlobals.h>
#include <ErrorGlobals.h>


#pragma segment MyGroupList

const long kCurrentGroupListVersion = 2;
const long kMinGroupListVersion = 1;

const short kMaxNameLen = 223;
typedef struct GroupEntry
{
	NewArticleStat fStat;
	char fFiller[31];
	char fName[kMaxNameLen + 1]; // C string
};
typedef GroupEntry *GroupEntryPtr;

TGroupList::TGroupList()
{
}

pascal void TGroupList::Initialize()
{
#if qDebug
	if (sizeof(GroupEntry) != 256)
		ProgramBreak("sizeof(GroupEntry) != 256");
#endif
	inherited::Initialize();
	fDoc = nil;
}

void TGroupList::IGroupList(TDocument *doc)
{
	inherited::IDynamicArray(0, sizeof(GroupEntry));
	fDoc = doc;
}

pascal void TGroupList::Free()
{
	inherited::Free();
}

void TGroupList::DoRead(TStream *aStream)
{
	long version = aStream->ReadLong();
	MyStreamCheckVersion(version, kMinGroupListVersion, kCurrentGroupListVersion, "TGroupList");
	if (version == 1)
	{
		DoIronAgeFormatRead(aStream);
		return;
	}
	ReadDynamicArray(aStream, this);
	DoPostRead();
}

void TGroupList::DoIronAgeFormatRead(TStream *aStream)
{
	aStream->ReadLong(); // fillers
	aStream->ReadLong();
	aStream->ReadLong();
	long size = aStream->ReadLong();
	SetArraySize(size);
	fSize = size;
	if (fSize)
	{
		Boolean prevLock = Lock(true);
		aStream->ReadBytes(ComputeAddress(1), fSize * sizeof(GroupEntry));
		Lock(prevLock);
	}
	DoPostRead();
}

void TGroupList::DoWrite(TStream *aStream)
{
	aStream->WriteLong(kCurrentGroupListVersion);
	WriteDynamicArray(aStream, this);
}

void TGroupList::DoNeedDiskSpace(long &dataForkBytes)
{
	dataForkBytes += sizeof(long); // version number
	dataForkBytes += MyStreamSizeOfDynamicArray(this);
}

void TGroupList::DoPostRead()
{
	for (ArrayIndex index = 1; index <= fSize; index++)
		ComputeGroupEntryAddress(index)->fStat = kUnknown;
}
//======================================================================
GroupEntry *TGroupList::ComputeGroupEntryAddress(ArrayIndex index)
{
	return GroupEntryPtr(inherited::ComputeAddress(index));
}

void TGroupList::GetGroupAt(ArrayIndex index, CStr255 &name)
{
	name = ComputeGroupEntryAddress(index)->fName;
}

ArrayIndex TGroupList::FindIndexFromName(const CStr255 &name)
{
	for (ArrayIndex index = 1; index <= fSize; index++)
		if (name == ComputeGroupEntryAddress(index)->fName)
			return index;
	return kEmptyIndex;
}

NewArticleStat TGroupList::GetNewArticleStat(ArrayIndex index)
{
	return ComputeGroupEntryAddress(index)->fStat;
}

void TGroupList::SetNewArticleStat(ArrayIndex index, NewArticleStat newStat)
{
	ComputeGroupEntryAddress(index)->fStat = newStat;
}

Boolean TGroupList::HasGroup(const CStr255 &name)
{
	return FindIndexFromName(name) != kEmptyIndex;
}

Boolean TGroupList::InsertGroupBefore(ArrayIndex index, const CStr255 &name)
{
	if (FindIndexFromName(name) != kEmptyIndex)
		return false; // in list
	GroupEntry entry;
	BlockSet(Ptr(&entry), sizeof(entry), 0);
	short len = short(Min(kMaxNameLen, name.Length()));
	const unsigned char *p = (const unsigned char*) &name;
	entry.fName[len] = 0;
	BytesMove(p + 1, entry.fName, len);
	InsertElementsBefore(index, &entry, 1);
	if (fDoc)
		fDoc->Changed(cGroupListChange, nil);
	return true;
}

void TGroupList::DeleteGroupAt(ArrayIndex index)
{
	DeleteElementsAt(index, 1);
	if (fDoc)
		fDoc->Changed(cGroupListChange, nil);
}

void TGroupList::DeleteGroup(const CStr255 &name)
{
	ArrayIndex index = FindIndexFromName(name);
	if (index != kEmptyIndex)
		DeleteGroupAt(index);
}

ArrayIndex TGroupList::AddGroupsFromListBefore(ArrayIndex beforeIndex, TGroupList *groupList)
{
	short noGroupsInserted = 0;
	CArrayIterator iter(groupList);
	for (ArrayIndex index = iter.FirstIndex(); iter.More(); index = iter.NextIndex())
	{
		CStr255 name;
		name = groupList->ComputeGroupEntryAddress(index)->fName;
		if (InsertGroupBefore(beforeIndex, name))
		{
			beforeIndex++;
			noGroupsInserted++;
		}
	}
	return noGroupsInserted;
}

Boolean TGroupList::AppendGroup(const CStr255 &name)
{
	return InsertGroupBefore(fSize + 1, name);
}

Boolean TGroupList::HasOneOfTheGroupsInList(TGroupList *groupList)
{
	CArrayIterator iter(groupList);
	for (ArrayIndex index = iter.FirstIndex(); iter.More(); index = iter.NextIndex())
	{
		CStr255 name;
		groupList->GetGroupAt(index, name);
		if (HasGroup(name))
			return true;
	}
	return false;	
}
