// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UGroupTree.cp

#include "UGroupTree.h"
#include "Tools.h"
#include "UThread.h"
#include "UProgress.h"
#include "UDynDynArray.h"
#include "UGrouplist.h"
#include "StreamTools.h"
#include "NetAsciiTools.h"

#include <RsrcGlobals.h>
#include <ErrorGlobals.h>

#include <ToolUtils.h>

#pragma segment MyGroupList

const long kCurrentGroupTreeVersion = 2;
const long kMinGroupTreeVersion = 2;

#define qDebugCreateBrief			qDebug & 0
#define qDebugCreate					qDebugCreateBrief & 0
#define qDebugCreateVerbose		qDebugCreate & 0
#define qDebugHelp						qDebug & 0
#define qDebugHelpVerbose     qDebugHelp & 0
#define qDebugIterator qDebug & 0
#define qDebugInfo qDebug & 0

char gDebugNodeName[300];

#define WNE { EventRecord event; WaitNextEvent(everyEvent, event, 1, nil); }

struct Node
{
	ArrayIndex fNextNodeIndex;
	ArrayIndex fFirstSubNodeIndex;
	ArrayIndex fParentIndex;
	Boolean fIsFolder;
	Boolean fFiller1;
	short fSubNoRealGroups;
	CRect fWindowFrame;
	long fLastReadArticleID;
	long fLastUpdatedArticleID;
};


struct NodeInfo
{
	CRect fFrame;
	long fLastReadArticleID;
	long fLastUpdatedArticleID;
};

typedef NodeInfo *NodeInfoPtr;

#if 0
  #if qDebugCreate | qDebugIterator

  #define stderr mystderr
  #define mystderr mystderr
FILE *mystderr;
void OpenOut()
{
	char filename[] = "HD2:Desktop Folder:GroupTreeLog";
	mystderr = fopen(filename, "w");
	fsetfileinfo(filename, 'MPS ', 'TEXT'); 
}
  #endif
#endif

//************************************************************************************************
inline long RoundToLong(long x) { return (x + 3) & ~3; }
//************************************************************************************************
CFindSubGroupIterator::CFindSubGroupIterator(TGroupTree *groupTree, TLongintList *expandData, ArrayIndex windowFolderIndex, ArrayIndex line)
{
	fGroupTree = groupTree;
	fExpandData = expandData;
	fLine = line;
	fOffset = 0;
	fIndex = kEmptyIndex;
	fWindowFolderIndex = windowFolderIndex;
#if qDebugIterator
	fprintf(stderr, "\n");
	fprintf(stderr, "Construct of CFSGI: line = %ld, wfi = %ld\n", line, windowFolderIndex);
	fPrevGroupLock = fGroupTree->Lock(true);
#endif
}

CFindSubGroupIterator::~CFindSubGroupIterator()
{
#if qDebugIterator
	fprintf(stderr, "Destruct of CFSGI\n");
	fGroupTree->Lock(fPrevGroupLock);
#endif
}

ArrayIndex CFindSubGroupIterator::FirstGroup()
{
	fOffset = fLine;
	fIndex = fGroupTree->ComputeNodeAddress(fWindowFolderIndex)->fFirstSubNodeIndex;
#if qDebugIterator
	fprintf(stderr, "CFSGI::First (fOffset = %ld), returned index = %ld\n", fOffset, fIndex);
#endif
	return fIndex;
}

Boolean CFindSubGroupIterator::More()
{
	if (fIndex == kEmptyIndex)
	{
#if qDebugIterator
		fprintf(stderr, "CFSGI::More returns false as fIndex == kEmptyIndex\n");
#endif
		return false;
	}
	if (!fOffset)
	{
#if qDebugIterator
		fprintf(stderr, "CFSGI::More returns false as fOffset == 0\n");
#endif
		return false;
	}
#if qDebugIterator
	fprintf(stderr, "CFSGI::More is true  (fOffset = %ld)\n", fOffset);
#endif
	return true;
}

ArrayIndex CFindSubGroupIterator::NextGroup()
{
	Advance();
#if qDebugIterator
	fprintf(stderr, "CFSGI::NextGroup returns %ld\n", fIndex);
#endif
	return fIndex;
}

void CFindSubGroupIterator::Advance()
{
#if qDebugIterator
	fprintf(stderr, "CFSGI::Advance at entry: fIndex = %ld, fOffset = %ld\n", fIndex, fOffset);
#endif
	NodePtr nodeP;
	NodePtr nodeIndex1P = fGroupTree->ComputeNodeAddress(1);
	while (true) // name loop
	{
		nodeP = nodeIndex1P + (fIndex - 1);
		fOffset--; // count line
		if (!fOffset)
		{
#if qDebugIterator
			fGroupTree->FetchDebugNodeName(fIndex);
			fprintf(stderr, "CFSGI::Advance, Found _the_ group %s as fOffset == 0 (isFolder = %hd)\n", gDebugNodeName, nodeP->fIsFolder);
#endif
			break; // found it!!!
		}
		if (!fOffset)
			return;
		ArrayIndex subLines;
		subLines = fExpandData->At(fIndex);
		if (fOffset <= subLines)
		{
			fIndex = nodeP->fFirstSubNodeIndex;
#if qDebugIterator
			fGroupTree->FetchDebugNodeName(fIndex);
			fprintf(stderr, "CFSGI::Advance, Found super-group (%s) for the group, returned sub index = %ld\n", gDebugNodeName, fIndex);
#endif
			break;
		}
		fOffset -= subLines;
#if qDebugIterator
		if (nodeP->fNextNodeIndex == kEmptyIndex)
		{
			ProgramBreak("Ups, nodeP->fNextNodeIndex == kEmptyIndex");
			return;
		}
		fGroupTree->FetchDebugNodeName(fIndex);
		fprintf(stderr, "CFSGI::Advance (%ld), skipped past %s with index %ld\n", fOffset, gDebugNodeName, fIndex);
#endif
		fIndex = nodeP->fNextNodeIndex;
	}
#if qDebugIterator
	fGroupTree->FetchDebugNodeName(fIndex);
	fprintf(stderr, "CFSGI::Advance at return: fOffset = %ld, index = %ld, isFolder = %hd, name = %s\n", fOffset, fIndex, nodeP->fIsFolder, gDebugNodeName);
#endif
}
//************************************************************************************************
CRealGroupIterator::CRealGroupIterator(TGroupTree *groupTree, ArrayIndex startIndex)
{
	fGroupTree = groupTree;
	fStartIndex = startIndex;
	fIndex = kEmptyIndex;
#if qDebugIterator
	fprintf(stderr, "Construct of CRealGroupIterator: startIndex = %ld\n", startIndex);
#endif
}

CRealGroupIterator::~CRealGroupIterator()
{
#if qDebugIterator
	fprintf(stderr, "Destruct of CRealGroupIterator\n");
#endif
}

ArrayIndex CRealGroupIterator::FirstGroup()
{
#if qDebug
	fPrevGroupDebugLock = fGroupTree->Lock(true);
#endif
	fIndex = fStartIndex;
	if (fGroupTree->ComputeNodeAddress(fIndex)->fIsFolder)
		Advance();
#if qDebugIterator
	fprintf(stderr, "CRealGroupIterator::First, returned index = %ld\n", fIndex);
#endif
	return fIndex;
}

Boolean CRealGroupIterator::More()
{
	if (fIndex == kEmptyIndex)
	{
#if qDebug
		fGroupTree->Lock(fPrevGroupDebugLock);
#endif
		return false;
	}
#if qDebugIterator
	fprintf(stderr, "CRealGroupIterator::More (fIndex = %ld) is true\n", fIndex);
#endif
	return true;
}

ArrayIndex CRealGroupIterator::NextGroup()
{
	Advance();
	return fIndex;
}

void CRealGroupIterator::Advance()
{
	NodePtr nodeP;
	NodePtr nodeIndex1P = fGroupTree->ComputeNodeAddress(1);
	nodeP = nodeIndex1P + (fIndex - 1);
#if qDebugIterator
	fGroupTree->FetchDebugNodeName(fIndex);
	fprintf(stderr, "\nCRealGroupIterator::Advance, Current item is: index = %ld, name = %s\n", fIndex, gDebugNodeName);
#endif
	if (nodeP->fFirstSubNodeIndex != kEmptyIndex)
	{
		fIndex = nodeP->fFirstSubNodeIndex;
		nodeP = nodeIndex1P + (fIndex - 1);
#if qDebugIterator
		fGroupTree->FetchDebugNodeName(fIndex);
		fprintf(stderr, "CRealGroupIterator::Advance, found sub-item: index = %ld, isFolder = %hd, name = %s\n", fIndex, nodeP->fIsFolder, gDebugNodeName);
		if (fIndex == kEmptyIndex)
			ProgramBreak("UPS");
#endif
		if (nodeP->fIsFolder)
			Advance();
		return;
	}
	while (true)
	{
		if (fIndex == fStartIndex && !nodeP->fIsFolder)
		{
#if qDebugIterator
			fprintf(stderr, "CRealGroupIterator::Advance, current was real entry-group, returning kEmptyIndex\n");
#endif
			fIndex = kEmptyIndex;
			return;
		}
		if (nodeP->fNextNodeIndex != kEmptyIndex)
		{
			// next group in list
			fIndex = nodeP->fNextNodeIndex;
			nodeP = nodeIndex1P + (fIndex - 1);
#if qDebugIterator
			fGroupTree->FetchDebugNodeName(fIndex);
			fprintf(stderr, "CRealGroupIterator::Advance, checking next item: index = %ld, isFolder = %hd, name = %s\n", fIndex, nodeP->fIsFolder, gDebugNodeName);
			if (fIndex == kEmptyIndex)
				ProgramBreak("UPS");
#endif
			if (nodeP->fIsFolder)
				Advance();
			return;
		}
		fIndex = nodeP->fParentIndex;
		nodeP = nodeIndex1P + (fIndex - 1);
		if (fIndex == fStartIndex)
		{
#if qDebugIterator
			fprintf(stderr, "CRealGroupIterator::Advance, parent was entry-group, returning kEmptyIndex\n");
#endif
			fIndex = kEmptyIndex;
			return;
		}
#if qDebugIterator
		fGroupTree->FetchDebugNodeName(fIndex);
		fprintf(stderr, "CRealGroupIterator::Advance, escaping to parent: index = %ld, isFolder = %hd, name = %s\n", fIndex, nodeP->fIsFolder, gDebugNodeName);
		if (fIndex == kEmptyIndex)
			ProgramBreak("UPS");
#endif
	}	
}
//************************************************************************************************
TGroupTree::TGroupTree()
{
}

pascal void TGroupTree::Initialize()
{
#ifdef mystderr
	OpenOut();
#endif
	inherited::Initialize();
	fGroupNameList = nil;
	fHelpTextList = nil;
}

void TGroupTree::IGroupTree()
{
#if qDebug
	if (sizeof(Node) != 32)
	{
		fprintf(stderr, "sizeof(Node) is %ld, and not 32\n", long(sizeof(Node)));
		ProgramBreak(gEmptyString);
	}
#endif
	inherited::IDynamicArray(0, sizeof(Node));
	FailInfo fi;
	if (fi.Try())
	{
		fAllocationIncrement = 512;
		fGroupNameList = NewDynDynArray(32 * 1024);
		fHelpTextList = NewDynDynArray(32 * 1024);
		fi.Success();
	}
	else // fail
	{
		Free();
		fi.ReSignal();
	}
}

pascal void TGroupTree::Free()
{
	delete fGroupNameList; fGroupNameList = nil;
	delete fHelpTextList; fHelpTextList = nil;
	inherited::Free();
}

Boolean TGroupTree::TreeIsEmpty()
{
	return fSize <= 1; // head-fake-node
}

void TGroupTree::DoRead(TStream *aStream)
{
	long version = aStream->ReadLong();
	if (!MyCheckVersion(version, kMinGroupTreeVersion, kCurrentGroupTreeVersion, "TGroupTree"))
		return;
	ReadDynamicArray(aStream, this);
	fGroupNameList->DoRead(aStream);
	fHelpTextList->DoRead(aStream);
}

void TGroupTree::DoWrite(TStream *aStream)
{
	aStream->WriteLong(kCurrentGroupTreeVersion);
	WriteDynamicArray(aStream, this);
	fGroupNameList->DoWrite(aStream);
	fHelpTextList->DoWrite(aStream);
}

void TGroupTree::DoNeedDiskSpace(long &dataForkBytes)
{
	dataForkBytes += sizeof(long); // version
	dataForkBytes += MyStreamSizeOfDynamicArray(this);
	dataForkBytes += fGroupNameList->NeededDiskSpace();
	dataForkBytes += fHelpTextList->NeededDiskSpace();
}

void TGroupTree::WriteTreeInfo(TStream *aStream)
{
	Handle h = nil;
	VOLATILE(h);
	FailInfo fi;
	if (fi.Try())
	{
		h = NewPermHandle(fSize * sizeof(NodeInfo));
		HLock(h);
		NodePtr nodeP = ComputeNodeAddress(1);
		NodeInfoPtr nodeInfoP = NodeInfoPtr(*h);
		for (ArrayIndex i = 1; i <= fSize; i++)
		{
			nodeInfoP->fFrame = nodeP->fWindowFrame;
			nodeInfoP->fLastReadArticleID = nodeP->fLastReadArticleID;
			nodeInfoP->fLastUpdatedArticleID = nodeP->fLastUpdatedArticleID;
			nodeP++;
			nodeInfoP++;
		}
		aStream->WriteHandle(h);
		h = DisposeIfHandle(h);
		fi.Success();
	}
	else // fail
	{
		h = DisposeIfHandle(h);
		// ignore errors, just don't saved info
	}
}

void TGroupTree::ReadTreeInfo(TStream *aStream)
{
	Handle h = nil;
	VOLATILE(h);
	FailInfo fi;
	if (fi.Try())
	{
		h = aStream->ReadHandle();
		if (GetHandleSize(h) != fSize * sizeof(NodeInfo))
		{
#if qDebug
			fprintf(stderr, "Handle had wrong size!!! Does not read Tree Info\n");
#endif
			Failure(0, 0); // it have been modified, just ignore saved info
		}
		HLock(h);
		NodePtr nodeP = ComputeNodeAddress(1);
		NodeInfoPtr nodeInfoP = NodeInfoPtr(*h);
		for (ArrayIndex i = 1; i <= fSize; i++)
		{
			nodeP->fWindowFrame = nodeInfoP->fFrame;
			nodeP->fLastReadArticleID = nodeInfoP->fLastReadArticleID;
			nodeP->fLastUpdatedArticleID = nodeInfoP->fLastUpdatedArticleID;
			nodeP++;
			nodeInfoP++;
		}
		h = DisposeIfHandle(h);
		fi.Success();
	}
	else // fail
	{
		h = DisposeIfHandle(h);
		// ignore errors, just forget saved info
	}
}

//========= REAL THING HERE =====================================================

#if qDebug
Node *TGroupTree::ComputeNodeAddress(ArrayIndex index)
{
	if (index < 1 || index > fSize)
	{
		fprintf(stderr, "TGroupTree, index = %ld out of range, fSize = %ld\n", index, fSize);
		ProgramBreak(gEmptyString);
		Failure(minErr, 0);
	}
	return NodePtr(inherited::ComputeAddress(index));
}
#endif

const CStr255 &TGroupTree::GetNodeName(ArrayIndex index)
{
#if qDebug & 0
	if (!VerboseIsObject(fGroupNameList))
		ProgramBreak("fGroupNameList is not object");
#endif
	return *(const CStr255 *) fGroupNameList->ComputeAddress(index);
}

void TGroupTree::FetchDebugNodeName(ArrayIndex index)
{
	CStr255 name = GetNodeName(index);
	BytesMove(&name[1], gDebugNodeName, name.Length());
	gDebugNodeName[name.Length()] = 0;
}

Boolean TGroupTree::GetNextSubName(const CStr255 &dotName, short &index, CStr255 &subName)
{
	if (index > dotName.Length())
		return false;
	subName = "";
	index++;
	while (index <= dotName.Length() && dotName[index] != '.')
		subName += dotName[index++];
	return true;
}

void TGroupTree::AddGroupEntry(CStr255 &subName, ArrayIndex parentIndex, Boolean isFolder, 
															ArrayIndex &firstIndex, 
															ArrayIndex &index, Boolean &newSubNode, ArrayIndex &deltaGroups)
{
	newSubNode = false;
#if qDebugCreate
	Lock(true);
#endif
	index = firstIndex;
	NodePtr checkNodeP = nil;
	NodePtr prevNodeP = nil;
	Boolean insertAsFirst = true;
	Boolean insertBefore = false;
	if (index != kEmptyIndex)
	{
		while (true)
		{
			checkNodeP = ComputeNodeAddress(index);
#if qDebugCreateVerbose
			CStr255 debugName(GetNodeName(index));
			fprintf(stderr, "Comparing subgroup with: %s\n", (char*)debugName);
#endif
			short cmp = StrCmp(subName, GetNodeName(index));
			if (cmp == kItem1LessThanItem2)
			{
#if qDebugCreateVerbose
				FetchDebugNodeName(index);
				fprintf(stderr, "Inserting before the subgroup: %s\n", gDebugNodeName);
#endif
				insertBefore = true;
				break; // insert it before checkNodeP
			}
			if (cmp == kItem1EqualItem2)
			{
				if (isFolder != checkNodeP->fIsFolder)
				{
#if qDebugCreateVerbose
					fprintf(stderr, "Found the group, but 'folder' state did not match\n");
#endif
					if (!isFolder)
						break; // insert the group x before the folder x
				}
				else
				{
#if qDebugCreateVerbose
					fprintf(stderr, "Found the subgroup: %s\n", (char*) subName);
#endif
					return; // found it
				}
			}
			insertAsFirst = false;
			if (checkNodeP->fNextNodeIndex == kEmptyIndex)
			{
#if qDebugCreateVerbose
				fprintf(stderr, "Did not find the subgroup: %s\n", (char*) subName);
#endif
				insertBefore = false;
				break; // at end of list
			}
			index = checkNodeP->fNextNodeIndex;
			prevNodeP = checkNodeP;
		}
	}
	ArrayIndex newIndex = fSize + 1; // new element
	newSubNode = true;
	Node newNode;
	BlockSet(Ptr(&newNode), sizeof(newNode), 0x88);

	newNode.fFirstSubNodeIndex = kEmptyIndex;
	newNode.fSubNoRealGroups = 0;
	newNode.fParentIndex = parentIndex;
	newNode.fIsFolder = isFolder;
	newNode.fWindowFrame = CRect(0, 0, 0, 0);
	newNode.fLastReadArticleID = 1;
	newNode.fLastUpdatedArticleID = 1;
	newNode.fFiller1 = 0;


	if (!isFolder)
		deltaGroups++;

	newNode.fNextNodeIndex = kEmptyIndex;
	if (insertAsFirst)
	{
		firstIndex = newIndex;
		newNode.fNextNodeIndex = index;
#if qDebugCreate
		fprintf(stderr, "%s is inserted as first in this list\n", (char*) subName);
#endif
	} 
	else if (insertBefore)
	{
		prevNodeP->fNextNodeIndex = newIndex;
		newNode.fNextNodeIndex = index;
#if qDebugCreate
		FetchDebugNodeName(index);
		fprintf(stderr, "New subgroup inserted before: %s\n", gDebugNodeName);
#endif
	}
	else // insert after existing group
	{
#if qDebugCreate
		FetchDebugNodeName(index);
		fprintf(stderr, "New subgroup chained after: %s\n", gDebugNodeName);
#endif
		newNode.fNextNodeIndex = checkNodeP->fNextNodeIndex;
		checkNodeP->fNextNodeIndex = newIndex;
	}
#if qDebugCreate
	Lock(false);
#endif
#if qDebug
	if (newIndex != fSize + 1)
		ProgramBreak("Trouble with newIndex for new element at InsertElementBefore");
#endif
	fGroupNameList->InsertLast(&subName, RoundToLong(subName.Length() + 1));
	fHelpTextList->CreateNewElement(0);
	InsertElementsBefore(newIndex, &newNode, 1);
	index = newIndex;
#if qDebugCreate
	DumpTree();
#endif
}

void TGroupTree::AddGroup(const CStr255 &dotName, Boolean &isNewGroup)
{
#if qDebugCreate
	fprintf(stderr, "\n");
#endif
#if qDebugCreateBrief
	CStr255 s(dotName);
	fprintf(stderr, "Adding new group:     ===> %s <===\n", (char*) s);
	WNE;
#endif
	short dotNameIndex = 0;
	ArrayIndex deltaGroups = 0;
	AddSubGroup(dotName, dotNameIndex, kFirstIndex, deltaGroups);
	if (deltaGroups)
	{
#if qDebugCreateVerbose
		fprintf(stderr, "Got %ld subDeltaGroups for group %s\n", deltaGroups, (char*)dotName);
#endif
		ComputeNodeAddress(kFirstIndex)->fSubNoRealGroups += short(deltaGroups);
	}
	isNewGroup = (deltaGroups != 0);
}

void TGroupTree::AddSubGroup(const CStr255 &dotName, short dotNameIndex, ArrayIndex parentIndex, ArrayIndex &deltaGroups)
{
	CStr255 subName;
	if (GetNextSubName(dotName, dotNameIndex, subName))
	{
		ArrayIndex firstSubIndex = ComputeNodeAddress(parentIndex)->fFirstSubNodeIndex;
		ArrayIndex index;
		Boolean newSubNode;
		Boolean isFolder = dotNameIndex <= dotName.Length();
		AddGroupEntry(subName, parentIndex, isFolder, firstSubIndex, index, newSubNode, deltaGroups);
		if (newSubNode)
		{
#if qDebugCreateVerbose
			fprintf(stderr, "A new sub-group was created: %s with ",  (char*) subName);
			fprintf(stderr, "index = %ld and parent = %ld ", index, parentIndex);
			fprintf(stderr, "(firstSub = %ld)\n", firstSubIndex);
#endif
			NodePtr parentP = ComputeNodeAddress(parentIndex);
			parentP->fFirstSubNodeIndex = firstSubIndex;
		}
		ArrayIndex subDeltaGroups = 0;
		AddSubGroup(dotName, dotNameIndex, index, subDeltaGroups);
		if (subDeltaGroups)
		{
#if qDebugCreateVerbose
			fprintf(stderr, "Got %ld subDeltaGroups for group %s\n", subDeltaGroups, (char*)subName);
#endif
			ComputeNodeAddress(index)->fSubNoRealGroups += short(subDeltaGroups);
			deltaGroups += subDeltaGroups;
		}
	}
}

void TGroupTree::DoInitialState()
{
	DeleteAll();
	Node newNode;
	BlockSet(Ptr(&newNode), sizeof(newNode), 0x88);
	CStr255 rootName;
	MyGetIndString(rootName, kGroupTreeRootName);
	fGroupNameList->InsertLast(&rootName, RoundToLong(rootName.Length() + 1));
	fHelpTextList->CreateNewElement(0);
	newNode.fNextNodeIndex = kEmptyIndex;
	newNode.fFirstSubNodeIndex = kEmptyIndex;
	newNode.fParentIndex = kEmptyIndex;
	newNode.fSubNoRealGroups = 0;
	newNode.fIsFolder = true;
	newNode.fWindowFrame = CRect(0, 0, 0, 0);
	newNode.fLastReadArticleID = 1;
	newNode.fFiller1 = 0;
	newNode.fLastUpdatedArticleID = 0;
	InsertElementsBefore(fSize + 1, &newNode, 1);
}

void TGroupTree::GetLineFromText(Handle h, long size, long &offset, CStr255 &line)
// The parsing in this proc assumes that the text in h is terminated with a CR
{
	line = "";
	Ptr p = *h + offset;
	while (*p != 13)
		line += *p++;
	p++; // past CR
	offset = p - *h;
	if (offset < size && *p == 10)
		p++, offset++;
}

void TGroupTree::UpdateGroupTree(Handle groupListH, TGroupList *newGroups)
// The parsing in this proc assumes that the text in groupListH (if any) is terminated with a CR
{
#if qDebug
	if (!fSize)
		ProgramBreak("UpdateGroupTree called while tree is missing head-fake-node");
#endif
	long size = GetHandleSize(groupListH);
	gCurProgress->SetWorkToDo(size);
	long offset = 0;
	ArrayIndex lineNo = 0;
	while (offset <= size)
	{
		CStr255 line, dotName;
		dotName = "";
		GetLineFromText(groupListH, size, offset, line);
		while (line.Length() && line[line.Length()] < 32)
			line.Length()--;
		char ch = line[line.Length()];
		if (ch == 'y' || ch == 'n' || ch == 'm')
		{
			short i = 1;
			while (i <= line.Length() && line[i] > 32)
				dotName += line[i++];
			if (dotName.Length())
			{
				Boolean isNew;
				AddGroup(dotName, isNew);
				if (newGroups && isNew)
					newGroups->AppendGroup(dotName);
#if qDebugCreateVerbose
				DumpTree();
#endif
			}
			if ((lineNo++ & 7) == 0)
			{
				gCurProgress->SetWorkDone(offset);
				gCurThread->CheckYield();
			}
		}
	}
	SetArraySize(fSize);
	fGroupNameList->SizeAllocToFit();
	fHelpTextList->SizeAllocToFit();
	gCurProgress->SetWorkDone(size);
#if qDebugCreate
	DumpTree();
#endif
}

void TGroupTree::UpdateDescriptions(Handle h)
// The parsing in this proc assumes that the text in groupListH (if any) is terminated with a CR
{
#if qDebug
	if (!fSize)
		ProgramBreak("UpdateDescriptions called while tree is missing head-fake-node");
#endif
	long size = GetHandleSize(h);
	gCurProgress->SetWorkToDo(size);
	long offset = 0;
	ArrayIndex lineNo = 0;
	while (offset <= size)
	{
		CStr255 line, dotName, help;
		dotName = help = "";
		GetLineFromText(h, size, offset, line);
		short i = 1;
		while (i <= line.Length() && line[i] > 32)
			dotName += line[i++];
		while (i <= line.Length() && line[i] <= 32)
			i++;
		while (i <= line.Length())
			help += line[i++];
		if (dotName.Length() && help.Length() && help[1] != '?')
		{
			ArrayIndex index = GetNodeIndexFromDotName(dotName);
			if (index)
			{
				long helpSize = RoundToLong(help.Length() + 1);
				fHelpTextList->SetElementSize(index, helpSize);
				fHelpTextList->ReplaceElementsAt(index, &help, 1);
#if qDebugHelpVerbose
				DumpTree();
#endif
			}
		}
		if ((lineNo++ & 7) == 0)
		{
			gCurProgress->SetWorkDone(offset);
			gCurThread->CheckYield();
		}
	}
	gCurProgress->SetWorkDone(size);
#if qDebugHelpVerbose
	DumpTree();
#endif
}

void TGroupTree::DumpAllNodes()
{
#if qDebug
	Boolean prevLock = Lock(true);
	fprintf(stderr, "index   next   fsub   parn   real   type   name\n");
	for (ArrayIndex index = 1; index <= fSize; index++)
	{
		fprintf(stderr, "%4ldi", index);
		NodePtr nodeP = ComputeNodeAddress(index);
		fprintf(stderr, "%6ldn", nodeP->fNextNodeIndex);
		fprintf(stderr, "%6lds", nodeP->fFirstSubNodeIndex);
		fprintf(stderr, "%6ldp", nodeP->fParentIndex);
		fprintf(stderr, "%6ldr", long(nodeP->fSubNoRealGroups));
		fprintf(stderr, nodeP->fIsFolder ? "   fldr" : "   grup");
		FetchDebugNodeName(index);
		fprintf(stderr, "   '%s'", gDebugNodeName);
		CStr255 help;
		GetHelpText(index, help);
		if (help.Length())
			fprintf(stderr, ", '%s'", (char*)help);
		fprintf(stderr, "\n");
	}
	Lock(prevLock);
#endif
}

void TGroupTree::DumpTreePart(ArrayIndex index, ArrayIndex level)
{
#if qDebug
	while (index != kEmptyIndex)
	{
		Node node;
		node = *ComputeNodeAddress(index);
		fprintf(stderr, "%3ldg %hdf  ", long(node.fSubNoRealGroups), node.fIsFolder);
		CStr255 s("");
		for (short i = 1; i <= level; i++)
			s += "¥    ";
		FetchDebugNodeName(index);
		CStr255 as = gDebugNodeName;
		s += as;
		fprintf(stderr, "%s\n", (char*)s);
		if (node.fFirstSubNodeIndex == index || node.fFirstSubNodeIndex < 0 || node.fFirstSubNodeIndex > fSize)
			fprintf(stderr, "*** INVALID SUB LINK ***\n");
		else
			DumpTreePart(node.fFirstSubNodeIndex, level + 1);
		if (node.fNextNodeIndex == index || node.fNextNodeIndex < 0 || node.fNextNodeIndex > fSize)
		{
			fprintf(stderr, "*** INVALID NEXT LINK ***\n");
			break;
		}
		index = node.fNextNodeIndex;
	}
#else
	index = level; // pragma unused
#endif
}

void TGroupTree::DumpTree()
{
#if qDebug
	fprintf(stderr, "Dumping all nodes:\n");
	DumpAllNodes();
	fprintf(stderr, "Dumping group table as tree:\n");
	if (kFirstIndex < 0 || kFirstIndex > fSize)
		fprintf(stderr, "*** kFirstIndex IS INVALID (%ld) ***\n", kFirstIndex);
	else
		DumpTreePart(kFirstIndex, 0);
	fprintf(stderr, "End of dump\n");
#endif
}

void TGroupTree::GetDrawInfo(ArrayIndex index, long &level, CStr255 &text, Boolean &isFolder)
{
	level = 0;
	NodePtr nodeP = ComputeNodeAddress(index);
	text = GetNodeName(index);
	isFolder = nodeP->fIsFolder;
	ArrayIndex levelIndex = index;
	while (levelIndex != kEmptyIndex)
	{
		NodePtr nodeP = ComputeNodeAddress(levelIndex);
		level++;
		levelIndex = nodeP->fParentIndex;
	}	
#if qDebugIterator
	fprintf(stderr, "GetDrawInfo: Got index: %ld at level %ld\n", index, level);
	fprintf(stderr, "   name = %s, isFolder = %hd\n", (char*)text, isFolder);
#endif
}

void TGroupTree::GetHelpText(ArrayIndex index, CStr255 &help)
{
	help = "";
	fHelpTextList->GetElementsAt(index, &help, 1);
}

//............................................................

void TGroupTree::GetDotNameFromNodeIndex(ArrayIndex index, CStr255 &name)
{
	if (index == kFirstIndex)
	{
		MyGetIndString(name, kGroupTreeRootName);
		return;
	}
	name = "";
	while (index != kFirstIndex)
	{
		NodePtr nodeP = ComputeNodeAddress(index);
		if (name.Length())
		{
			name.Insert(".", 1);
			name.Insert(GetNodeName(index), 1);
		}
		else
			name = GetNodeName(index);
		index = nodeP->fParentIndex;
	}
}

ArrayIndex TGroupTree::GetNodeIndexFromDotName(const CStr255 &dotName)
{
	short dotNameIndex = 0;
	CStr255 subName;
	ArrayIndex index = ComputeNodeAddress(kFirstIndex)->fFirstSubNodeIndex;
	while (true)
	{
		if (!GetNextSubName(dotName, dotNameIndex, subName))
			return kEmptyIndex;
		Boolean isLastPart = (dotNameIndex > dotName.Length());
		NodePtr nodeP = ComputeNodeAddress(index);
		while (!StrEqual(subName, GetNodeName(index)) || nodeP->fIsFolder == isLastPart)
		{
			index = nodeP->fNextNodeIndex;
			if (index == kEmptyIndex)
			{
#if qDebug & 0
				fprintf(stderr, "Did not find the group '%s' in the tree\n", (char*)dotName);
#endif
				return kEmptyIndex;
			}
			nodeP = ComputeNodeAddress(index);
		}
		if (isLastPart)
			return index;
		index = nodeP->fFirstSubNodeIndex;
	}
}
//............................................................
ArrayIndex TGroupTree::SubGetNoItems(TLongintList *expandData, ArrayIndex parentIndex)
{
	long noItems = 0;
	ArrayIndex index = ComputeNodeAddress(parentIndex)->fFirstSubNodeIndex;
	while (index != parentIndex)
	{
		NodePtr nodeP = ComputeNodeAddress(index);
		noItems++;
		if (nodeP->fIsFolder && expandData->At(index))
			noItems += SubGetNoItems(expandData, index);
		if (nodeP->fNextNodeIndex == kEmptyIndex)
			return noItems;
		index = nodeP->fNextNodeIndex;
	}
}

ArrayIndex TGroupTree::GetNoItems(TLongintList *expandData, ArrayIndex startIndex)
{
	ArrayIndex noItems = SubGetNoItems(expandData, startIndex);
#if qDebugInfo
	fprintf(stderr, "TGroupTree::GetNoItems, startIndex = %ld -> noItems = %ld\n", startIndex, noItems);
#endif
	return noItems;
}

ArrayIndex TGroupTree::FindSubGroupIndexFromLine(TLongintList *expandData, ArrayIndex windowFolderIndex, ArrayIndex line)
{
#if qDebug
	if (windowFolderIndex < 1 || windowFolderIndex > fSize)
		ProgramBreak("Invalid windowFolderIndex");
	if (line < 1 || line > 32000)
		ProgramBreak("Invalid line");
#endif
	CFindSubGroupIterator iter(this, expandData, windowFolderIndex, line);
	for (ArrayIndex index = iter.FirstGroup(); iter.More(); index = iter.NextGroup())
		;
#if qDebugInfo
	fprintf(stderr, "TGroupTree::FindSubGroupIndexFromLine line = %hd -> index = %ld\n", line, index);
#endif
	return index;	
}

ArrayIndex TGroupTree::GetParentIndex(ArrayIndex index)
{
	if (index == kFirstIndex)
	{
#if qDebug
		ProgramBreak("TGroupTree::GetParentIndex called with index = kFirstIndex");
#endif
		return kFirstIndex;
	}
	return ComputeNodeAddress(index)->fParentIndex;
}

CRect TGroupTree::GetWindowFrame(ArrayIndex index)
{
	return ComputeNodeAddress(index)->fWindowFrame;
}

void TGroupTree::SetWindowFrame(ArrayIndex index, const CRect &frame)
{
	ComputeNodeAddress(index)->fWindowFrame = frame;
}

Boolean TGroupTree::HasGroup(const CStr255 &dotName)
{
	return GetNodeIndexFromDotName(dotName) != kEmptyIndex;
}

long TGroupTree::GetLastReadArticleID(const CStr255 &dotName)
{
	ArrayIndex index = GetNodeIndexFromDotName(dotName);
	if (index == kEmptyIndex)
		return 0;
	return ComputeNodeAddress(index)->fLastReadArticleID;
}

void TGroupTree::SetLastReadArticleID(const CStr255 &dotName, long lastID)
{
	ArrayIndex index = GetNodeIndexFromDotName(dotName);
	if (index != kEmptyIndex)
		ComputeNodeAddress(index)->fLastReadArticleID = lastID;
}

long TGroupTree::GetLastUpdatedArticleID(const CStr255 &dotName)
{
	ArrayIndex index = GetNodeIndexFromDotName(dotName);
	if (index == kEmptyIndex)
		return 0;
	return ComputeNodeAddress(index)->fLastUpdatedArticleID;
}

void TGroupTree::SetLastUpdatedArticleID(const CStr255 &dotName, long lastID)
{
	ArrayIndex index = GetNodeIndexFromDotName(dotName);
	if (index != kEmptyIndex)
		ComputeNodeAddress(index)->fLastUpdatedArticleID = lastID;
}

//******************************** EXPAND ******************************************
//------------------------------------------------------------------
void TGroupTree::Expand(TLongintList *expandData, Boolean doExpand, Boolean wayDown, 
																						ArrayIndex index, ArrayIndex &deltaLines)
{
#if qDebugExpand
	Lock(true);
	fprintf(stderr, "\n");
	fprintf(stderr, "Expand: <> <> <> <>\n");
#endif
	deltaLines = 0;
// find the line
	if (doExpand && expandData->At(index))
	{
#if qDebugExpand
		fprintf(stderr, "TGroupTree::Expand: Is expanded with %ld sublines\n", expandData->At(index));
#endif
		return; // is expanded
	}
	if (!doExpand && !expandData->At(index))
	{
#if qDebugExpand
		fprintf(stderr, "TGroupTree::Expand: Was unexpanded \n");
#endif
		return; // is unexpanded
	}
	
	Boolean groupsIsVisible = (expandData->At(index) > 0);
	NodePtr nodeP = ComputeNodeAddress(index);
	NodePtr nodeIndex1P = ComputeNodeAddress(1);
	DoExpand(expandData, nodeIndex1P, nodeP->fFirstSubNodeIndex, doExpand, wayDown, deltaLines, groupsIsVisible);

// adjust parents
	while (index != kEmptyIndex)
	{
		nodeP = ComputeNodeAddress(index);
#if qDebugExpand
		FetchDebugNodeName(index);
		fprintf(stderr, "Adjusted %s with %ld (%ld to %ld) lines\n", gDebugNodeName, deltaLines, nodeP->fSubNoShownLines, deltaLines + expandData->At(index));
#endif		
		ArrayIndex *edP = (ArrayIndex*) expandData->ComputeAddress(index);
		*edP += deltaLines;
		index = nodeP->fParentIndex;
	}
#if qDebugExpand
	Lock(false);
	DumpTree();
#endif
}

// TGroupTree must be locked in debug mode!
void TGroupTree::DoExpand(TLongintList *expandData, NodePtr nodeIndex1P, ArrayIndex index, Boolean doExpand, 
									Boolean wayDown, ArrayIndex &deltaLines, Boolean groupsIsVisible)
{
#if qDebugExpand
	NodePtr XnodeP = nodeIndex1P + (index - 1);
	FetchDebugNodeName(index);
	fprintf(stderr, "DoExpand   (x, %ld, %hd, %hd, %hd) for %s\n",
		index, doExpand, wayDown, deltaLines, gDebugNodeName);
#endif
// mark all sub as unexpanded and count deltaLines
	while (index != kEmptyIndex)
	{
		NodePtr nodeP = nodeIndex1P + (index - 1); // much faster when doing this alot
		if (doExpand && !groupsIsVisible)
			deltaLines++;
		if (!doExpand && groupsIsVisible)
			deltaLines--;
#if qDebugExpand
	FetchDebugNodeName(index);
	NodePtr XnodeP = nodeIndex1P + (index - 1);
	fprintf(stderr, "--->DoExpand(x, %ld, %hd, %ld, %hd) for %s\n",
		index, doExpand, wayDown, deltaLines, gDebugNodeName);
#endif
		if (wayDown)
		{
			Boolean subGroupsVisible = groupsIsVisible && expandData->At(index);
			ArrayIndex subDeltaLines = 0;
			ArrayIndex subIndex = nodeP->fFirstSubNodeIndex;
			if (subIndex != kEmptyIndex) // optimize
				DoExpand(expandData, nodeIndex1P, subIndex, doExpand, wayDown, subDeltaLines, subGroupsVisible);
			if (doExpand)
				expandData->AtPut(index, subDeltaLines);
			else
				expandData->AtPut(index, 0);
			deltaLines += subDeltaLines;
		}
		else
		{
			if (doExpand)
				deltaLines += expandData->At(index);
			else
				deltaLines -= expandData->At(index);
		}
		index = nodeP->fNextNodeIndex;
	}
}
