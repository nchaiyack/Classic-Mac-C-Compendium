// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UGroupDocCmds.h

#define __UGROUPDOCCMDS__

class TGroupDoc;
class PNntp;

void GoPublicFile(const CStr255 &groupDotName, FSSpec &spec);

class TOpenGroupCommand : public TCommand
{
	public:
		pascal void DoIt();

		TOpenGroupCommand();
		pascal void Initialize();
		void IOpenGroupCommand(const CStr255 &name, 
			Boolean updateDatabase, Boolean updateOnly, Boolean saveIfNonexisting);
		pascal void Free();

	private:
		CStr255 fGroupDotName;
		Boolean fUpdateDatabase;
		Boolean fUpdateOnly;
		Boolean fSaveIfNonexisting;
		Boolean fGotDBFile, fGotStatFile;
		TGroupDoc *fDoc;
		PNntp *fNntp;
		long fFirstID, fLastID, fFirstNewID;

		void OpenDocument();
		void ShowDiscListWindow();

		void UpdateDiscussions(); // fetches new headers from host
		Boolean StartUpdate(); // true if anything new
		void UpdateHeaderLists();

		void CreateNewDiscussion(long id);
		void DistributeArticle(long id);
		void DistributeNewArticles();
};
