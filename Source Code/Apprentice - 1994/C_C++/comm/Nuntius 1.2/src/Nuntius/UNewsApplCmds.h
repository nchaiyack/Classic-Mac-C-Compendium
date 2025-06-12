// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UNewsApplCmds.h

#define __UNEWSAPPLCMDS__

class TProgress;

class TOpenPrefsCommand : public TCommand
{
	public:
		pascal void DoIt();
		
		TOpenPrefsCommand();
		pascal void Initialize();
		void IOpenPrefsCommand(TList *docList = nil);
		pascal void Free();
	private:
		TFile *fGroupTreeFile;
		TList *fDocFileList;
		Boolean fUpdateServerInfo;
		long fOldPrefsVersion, fMinNeededVersion;

		TFile *GetSpecialFile(OSType fileType);
		void OpenPrefsFile();
		void OpenGroupTreeFile();
		void UpgradePrefs();
		void ReadExistingPrefs();
		void UseNewServer(CStr255 &name);
		void AskNewsServer();
		void CheckNewsServerIP();
		void BuildListOfAllGroups();
		void CheckPreFinalVersion();

		void UpdateFolders();
		void GetNuntiusTrashFolder(FSSpec &spec);
		void MoveToTrash(const FSSpec &spec);
		void MovePrefFileToTrash(const CStr255 &name);
		void MoveFileToPrivate(const CStr255 &name);
		void MoveGroupLists();
};

void CloseDownTheManagers();
