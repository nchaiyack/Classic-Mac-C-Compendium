// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UFolderSelectView.h

#define __UFOLDERSELECTVIEW__

class TButton;
class TStaticText;

class TFileSelectView : public TView 
{
	public:
		
		virtual void Specify(const FSSpec &spec);
		virtual void GetFile(FSSpec &spec);
		virtual OSType GetFileSignature();
		virtual Boolean GotFile();

		virtual void InitializeFromPreferences(OSType pref);
		virtual void StoreInPreferences(OSType pref);
	
		virtual void DimState(Boolean state, Boolean redraw);
		// calls button/staticText's DimState

		virtual pascal void DoEvent(EventNumber eventNumber, TEventHandler *source,
								TEvent *event);

		virtual void SpecifyFileTypes(OSType fileType, OSType creator);		
		TFileSelectView();
		virtual pascal void Initialize();
		virtual pascal void ReadFields(TStream *aStream);
		virtual pascal void DoPostCreate(TDocument *itsDocument);
		virtual pascal void Free();
	protected:
		friend pascal Boolean FileFFilter(ParmBlkPtr pb, void *yourDataPtr); // call-back
		FSSpec fSpec;
		TButton *fButton;
		TStaticText *fPath;
		Boolean fGotFile;
		OSType fNeededFileType, fNeededCreator;
		OSType fSignature;
		
		virtual Boolean DoGetFile();
		virtual void UpdatePathName();
		virtual Boolean DontShowFileFilter(ParmBlkPtr pb);
};

class TFolderSelectView : public TView
{
	public:
		
		virtual void Specify(short vRefNum, long dirID);
		virtual void GetDir(short &vRefNum, long &dirID);
		
		virtual void InitializeFromPreferences(OSType pref);
		virtual void StoreInPreferences(OSType pref);
	
		virtual void DimState(Boolean state, Boolean redraw);
		// calls button/staticText's DimState

		virtual pascal void DoEvent(EventNumber eventNumber, TEventHandler *source,
								TEvent *event);
		
		TFolderSelectView();
		virtual pascal void Initialize();
		virtual pascal void ReadFields(TStream *aStream);
		virtual pascal void DoPostCreate(TDocument *itsDocument);
		virtual pascal void Free();
	protected:
		short fVRefNum;
		long fDirID;
		TButton *fButton;
		TStaticText *fPath;
		Boolean fGotFolder;
		CStr255 fPromptString;
		
		virtual Boolean DoGetDirectory();
		virtual void UpdatePathName();
};
