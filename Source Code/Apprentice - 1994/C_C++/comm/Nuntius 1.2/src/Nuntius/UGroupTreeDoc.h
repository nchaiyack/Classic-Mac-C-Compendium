// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UGroupTreeDoc.h

#define __UGROUPTREEDOC__

class TGroupTree;
class TGroupTreeView;

class TGroupTreeDoc : public TFileBasedDocument
{
	public:
		TGroupTree *GetGroupTree();
		void CreateGroupTreeWindow(ArrayIndex windowFolderIndex = 0, TGroupTreeView *openFromView = nil);
		void ForgetGroupTreeView(TGroupTreeView *view);
		
		void RebuildGroupTree();
		void UpdateGroupTree();
		
		pascal void DoRead(TFile *aFile, Boolean forPrinting);
		pascal void DoWrite(TFile *aFile, Boolean forPrinting);
		pascal void DoNeedDiskSpace(TFile *itsFile, long &dataForkBytes, long &rsrcForkBytes);
		void SaveTheTree();

		pascal void DoMakeViews(Boolean forPrinting);
		pascal void CloseWindow(TWindow* aWindow);
		void CloseAllWindows();
		void WriteTreeInfo(TStream *aStream);
		void ReadTreeInfo(TStream *aStream);
		void WriteWindowInfo(TStream *aStream, TWindow *window);
		void ReadWindowInfo(TStream *aStream);

		pascal void DoMenuCommand(CommandNumber aCommandNumber);
		pascal void DoSetupMenus();
		pascal void OpenAgain(CommandNumber itsCommandNumber, TDocument *openingDoc);
		pascal void UntitledName(CStr255& noName);
		pascal TFile* DoMakeFile(CommandNumber itsCommandNumber);
		
		TGroupTreeDoc();
		pascal void Initialize();
		void IGroupTreeDoc(TFile *itsFile);
		pascal void DoInitialState();
		pascal void DoPostMakeViews(Boolean forPrinting);
		pascal void Close();
		pascal void Free();
	private:
		TGroupTree *fGroupTree;
		TList *fGroupTreeViewList;
		unsigned long fLastUpdateDate;
		
		void DoReadFile(TStream *aStream);
		void DoWriteFile(TStream *aStream);
};
