// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UGroupTree.h

#define __UGROUPTREE__

struct Node; // private
typedef Node *NodePtr;
class PDynDynArray;

const ArrayIndex kFirstIndex = 1;
class TGroupList;

class TGroupTree : public TDynamicArray
{
	public:
		void GetDotNameFromNodeIndex(ArrayIndex index, CStr255 &name);
		ArrayIndex GetNodeIndexFromDotName(const CStr255 &dotName);
		ArrayIndex FindSubGroupIndexFromLine(TLongintList *expandData, ArrayIndex windowFolderIndex, ArrayIndex line);
		ArrayIndex GetParentIndex(ArrayIndex index);

		Boolean HasGroup(const CStr255 &dotName);
		void GetDrawInfo(ArrayIndex index, long &level, CStr255 &text, Boolean &isFolder);
		void GetHelpText(ArrayIndex index, CStr255 &help);
		ArrayIndex GetNoItems(TLongintList *expandData, ArrayIndex startIndex);
		CRect GetWindowFrame(ArrayIndex index);
		void SetWindowFrame(ArrayIndex index, const CRect &frame);

		long GetLastReadArticleID(const CStr255 &dotName);
		void SetLastReadArticleID(const CStr255 &dotName, long lastID);
		long GetLastUpdatedArticleID(const CStr255 &dotName);
		void SetLastUpdatedArticleID(const CStr255 &dotName, long lastID);


		void Expand(TLongintList *expandData, Boolean doExpand, Boolean wayDown, ArrayIndex index, ArrayIndex &deltaLines);

		void UpdateGroupTree(Handle groupListH, TGroupList *newGroups); // newGroups may be nil
		void UpdateDescriptions(Handle h);
		Boolean TreeIsEmpty();

		void WriteTreeInfo(TStream *aStream);
		void ReadTreeInfo(TStream *aStream);

		void DoRead(TStream *aStream);
		void DoWrite(TStream *aStream);
		void DoNeedDiskSpace(long &dataForkBytes);
		void DoInitialState();
		TGroupTree();
		pascal void Initialize();
		void IGroupTree();
		pascal void Free();
	protected:
		friend class CFindSubGroupIterator;
		friend class CRealGroupIterator;
		
		PDynDynArray *fGroupNameList;
		PDynDynArray *fHelpTextList;
		
		NodePtr ComputeNodeAddress(ArrayIndex index);
		const CStr255 &GetNodeName(ArrayIndex index);
		void FetchDebugNodeName(ArrayIndex index);
		
		Boolean GetNextSubName(const CStr255 &dotName, short &index, CStr255 &subName);
		void AddGroupEntry(CStr255 &subName, ArrayIndex parentIndex, Boolean isReal, 
															ArrayIndex &firstIndex, 
															ArrayIndex &index, Boolean &newSubNode, ArrayIndex &deltaGroups);
		void AddSubGroup(const CStr255 &dotName, short dotNameIndex, ArrayIndex parentIndex, ArrayIndex &deltaGroups);
		void AddGroup(const CStr255 &dotName, Boolean &isNewGroup);
		void GetLineFromText(Handle h, long size, long &offset, CStr255 &line);
		
		void DumpTree();
		void DumpTreePart(ArrayIndex index, ArrayIndex level);
		void DumpAllNodes();

		ArrayIndex SubGetNoItems(TLongintList *expandData, ArrayIndex parentIndex);

		void DoExpand(TLongintList *expandData, NodePtr nodeIndex1P, ArrayIndex index, Boolean doExpand, 
									Boolean wayDown, ArrayIndex &deltaLines, Boolean groupsIsVisible);
};


class CRealGroupIterator : public CIterator
{
	public:
		CRealGroupIterator(TGroupTree *groupTree, ArrayIndex startIndex);
		~CRealGroupIterator();
		ArrayIndex FirstGroup();
		Boolean More();
		ArrayIndex NextGroup();

	protected:
		void Advance();

	private:
		TGroupTree *fGroupTree;
		ArrayIndex fStartIndex;
		ArrayIndex fIndex;
		Boolean fPrevGroupDebugLock;
};

class CFindSubGroupIterator : public CIterator
{
	public:
		CFindSubGroupIterator(TGroupTree *groupTree, TLongintList *expandData, ArrayIndex windowFolderIndex, ArrayIndex line);
		~CFindSubGroupIterator();
		ArrayIndex FirstGroup();
		Boolean More();
		ArrayIndex NextGroup();
	protected:
		void Advance();
		
	private:
		TGroupTree *fGroupTree;
		TLongintList *fExpandData;
		ArrayIndex fIndex;
		ArrayIndex fOffset;
		ArrayIndex fLine;
		ArrayIndex fWindowFolderIndex;
		Boolean fPrevGroupLock;
};

#if !qDebug
inline Node *TGroupTree::ComputeNodeAddress(ArrayIndex index)
{
	return NodePtr(inherited::ComputeAddress(index));
}
#endif