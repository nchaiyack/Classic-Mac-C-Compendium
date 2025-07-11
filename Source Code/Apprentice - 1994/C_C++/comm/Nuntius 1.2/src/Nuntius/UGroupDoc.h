// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UGroupDoc.h

#define __UGROUPDOC__

class PDiscList;
class TDiscListView;
class TArticleStatus;
class THeaderList;
struct HandleOffsetLength;
class TOpenGroupCommand;

class TGroupDoc : public TFileBasedDocument
{
	public:
		TArticleStatus *GetArticleStatus();
		TArticleStatus *GetOldArticleStatus();

		PDiscList *GetDiscList();
		TDiscListView *GetDiscListView();

		void GetGroupDotName(CStr255 &name);		
		long GetFirstArticleID();
		long GetLastArticleID();
		Boolean GetFrom(long id, HandleOffsetLength &hol);
		Boolean GetSubject(long id, HandleOffsetLength &hol);
				
		void IncLockAgainstClose();
		void DecLockAgainstClose();
		
		pascal void DoRead(TFile *aFile, Boolean forPrinting);
		pascal void DoWrite(TFile *aFile, Boolean forPrinting);
		pascal void DoNeedDiskSpace(TFile *itsFile, long &dataForkBytes, long &rsrcForkBytes);
		pascal TFile *DoMakeFile(CommandNumber itsCommandNumber);
		void CheckDBFile(ResNumber rsrcId, short rsrcIndex, Boolean reverting);
		pascal void SaveDocument(CommandNumber itsCommandNumber);
		pascal void ReadDocument(Boolean forPrinting);
		pascal void FileHasBeenSaved(const CStr255& newName);
		pascal void Changed(ChangeID theChange, TObject* changedBy);
		void DoPostUpdate(Boolean updateOnly);
		
		pascal void DoMakeViews(Boolean forPrinting);

		pascal void DoMenuCommand(CommandNumber aCommandNumber);
		pascal void DoSetupMenus();
		
		TGroupDoc();
		pascal void Initialize();
		void IGroupDoc(TFile *statFile, TFile *dbFile, const CStr255 &dotName);
		pascal void DoInitialState();
		pascal void Close();
		pascal void Free();
	private:
		friend class TOpenGroupCommand;

		TFileHandler *fDBFileHandler;
		PDiscList *fDiscList;
		TWindow *fDiscListWindow;
		TDiscListView *fDiscListView;

		TArticleStatus *fOldArticleStatus, *fArticleStatus;
		THeaderList *fSubjectHdr, *fFromHdr, *fMsgIDHdr, *fRefsHdr;
		long fFirstArticleID, fLastArticleID;
		CStr255 fGroupDotName;
		long fLockLevelAgainstClose;
		Boolean fArticleStatusIsDirty, fDatabaseIsDirty;
		Boolean fSaveArticleStatus, fSaveDatabase;

		void DoReadDatabase(TStream *aStream);
		void DoWriteDatabase(TStream *aStream);
		void DatabaseDoNeedDiskSpace(TFile *itsFile, long &dataForkBytes,  long &rsrcForkBytes);
		void DoReadArticleStatus(TStream *aStream);
		void DoWriteArticleStatus(TStream *aStream);
		void ArticleStatusDoNeedDiskSpace(TFile *itsFile, long &dataForkBytes,  long &rsrcForkBytes);

};

