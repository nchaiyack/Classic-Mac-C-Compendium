// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UNewsAppl.h

class TGroupTreeDoc;
class TGroupTree;
class TOpenPrefsCommand;
class TThreadList;

class TNewsAppl : public TApplication 
{
	public:
		pascal TToolboxEvent *GetEvent(short eventMask, long sleep, RgnHandle sleepRegion);
		pascal void UpdateAllWindows();
		pascal void SpaceIsLowAlert();
		pascal void DoToolboxEvent(TToolboxEvent* event);
		pascal void DoKeyEvent(TToolboxEvent *event);
		pascal void DoMenuCommand(CommandNumber aCommandNumber);
		pascal void DoAppleCommand(CommandNumber aCommandNumber, const AppleEvent& message, const AppleEvent& reply);
		pascal void OpenOld(CommandNumber itsOpenCommand, TList* aFileList);
		void OpenOneOld(TFile *file); // owns file
		pascal void DoSetupMenus();
		pascal void RegainControl(Boolean checkClipboard);

		void EnableFontMenu(TextStyle ts);
		Boolean HandleFontMenu(CommandNumber cmd, TextStyle &ts);

		void SetGroupTreeDoc(TGroupTreeDoc *doc);
		TGroupTreeDoc *GetGroupTreeDoc();
		TGroupTree *GetGroupTree();
		void QuickUpdateGroupHasNoNewArticles(const CStr255 &dotName);
		void SetNeedCheckFolderMenus();
		
		pascal TFile* DoMakeFile(CommandNumber itsCommandNumber);
		pascal CommandNumber KindOfDocument(CommandNumber itsCommandNumber, TFile* itsFile);
		pascal TDocument *DoMakeDocument(CommandNumber itsCommandNumber, TFile *itsFile);
		pascal void GetFileTypeList(CommandNumber itsCommandNumber, TypeListHandle &typeList);
		pascal Boolean CanOpenDocument(CommandNumber itsCommandNumber, TFile* aFile);

		pascal void DoAboutBox();
		pascal void GetHelpParameters(ResNumber helpResource, short helpIndex,
														short helpState, HMMessageRecord &helpMessage,
														CPoint &localQDTip, CRect &localQDRect, short &balloonVariant);
		
		TNewsAppl();
		// init stuff
		pascal void Close();
		pascal void Initialize();
		void INewsAppl();
		pascal void Free();
	private:
		friend class TOpenPrefsCommand;
		
		MenuHandle fWindowMenuH, fFontNameMenuH, fFontSizeMenuH, fGroupListMenuH;
		unsigned long fLastFolderModDate;
		Boolean fNeedCheckFolderMenus;
		Boolean fHadGroupList;

		TGroupTreeDoc *fGroupTreeDoc;
		Boolean fApplIsRunning;
		Boolean fFlushVolumes;
		
		void CreateMenus();
		void DoWindowsMenu(short itemToSelect);
		void UpdateGroupListMenu();
		void HandleGroupListMenu(short item);
		
		void RestoreWindowState();
		void SaveWindowState();
		void DoSaveWindowStates(TStream *aStream);
};

extern TNewsAppl *gNewsAppl;

void ShowSplashScreen();
void RemoveSplashScreen();
