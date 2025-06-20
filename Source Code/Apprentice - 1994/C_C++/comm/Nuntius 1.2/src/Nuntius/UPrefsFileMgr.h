// Copyright � 1994 Peter Speck (speck@dat.ruc.dk).  All rights reserved.
// UPrefsFileMgr.h

#ifndef __UPREFSDATABASE__
#include "UPrefsDatabase.h"
#endif

class PPrefsFileMgr : public PPrefsDatabase
{
	public:
	
		void Load();
		void Save();
		
		void SetPrefsValidationState(Boolean valid);
		TFile *GetFile();
		
		PPrefsFileMgr();
		void IPrefsFileMgr(FSSpec spec);
		virtual ~PPrefsFileMgr();
	private:
		TFile *fPrefsFile;
		TFile *fTmpFile;
		Boolean fPrefsIsValid;
		
		void SpecifyTmpFile();
};
extern PPrefsFileMgr *gPrefsFileMgr;

void GetPrefsDocLocation(FSSpec &spec);
void GoPrefsLocation(TFile *file, short stringID);
void GetPrefsLocationFileUntitledName(CStr255 &name, short stringID);
