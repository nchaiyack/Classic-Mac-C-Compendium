// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UGroupListDoc.h

class TGroupList;
class TGroupListView;
class TPeriodicCheckNewArticles;

class TGroupListDoc : public TFileBasedDocument
{
	public:
		TGroupList *GetGroupList();
		TGroupListView *GetGroupListView();
		void SpecifyList(TGroupList *groupList);

		pascal void DoRead(TFile *aFile, Boolean forPrinting);
		pascal void DoWrite(TFile *aFile, Boolean forPrinting);
		pascal void DoNeedDiskSpace(TFile *itsFile, long &dataForkBytes, long &rsrcForkBytes);
		pascal void ReadDocument(Boolean forPrinting);

		pascal void DoMakeViews(Boolean forPrinting);
		pascal void CloseWindow(TWindow* aWindow);
		AliasHandle GetAlias();
		void UpdateGroupStatus(const CStr255 &dotName, Boolean hasNewArticles);
		pascal void Changed(ChangeID theChange, TObject* changedBy);
		Boolean IsCheckingForNewArticlesNow();
		void CheckForNewArticlesNow();
		
		pascal void DoMenuCommand(CommandNumber aCommandNumber);
		pascal void DoSetupMenus();
		pascal void OpenAgain(CommandNumber itsCommandNumber, TDocument *openingDoc);
		pascal void UntitledName(CStr255& noName);
		pascal TFile* DoMakeFile(CommandNumber itsCommandNumber);
		
		TGroupListDoc();
		pascal void Initialize();
		void IGroupListDoc(TFile *itsFile);
		pascal void DoInitialState();
		pascal void RevertDocument();
		pascal void DoPostMakeViews(Boolean forPrinting);
		pascal void Close();
		pascal void Free();
	private:
		TGroupList *fGroupList;
		TGroupListView *fGroupListView;
		TPeriodicCheckNewArticles *fPeriodicCheck;
		TWindow *fGroupListWindow;
		VRect fWindowFrame;
		Boolean fWasOldFileVersion;

		void DoReadFile(TStream *aStream);
		void DoWriteFile(TStream *aStream);
};
