// Copyright � 1994 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UPrefsDatabase.h

#define __UPREFSDATABASE__

#ifndef __UPTROBJECT__
#include "UPtrObject.h"
#endif

class PDynDynArray;

class PPrefsDatabase : public PPtrObject
{
	public:
		//----------------------------------------------
		void GetPrefs(OSType id, void *data);
		void SetPrefs(OSType id, const void *data, long size);
		// warning: size is rounded to long-even-size
		// nil means set only size

		long		GetPrefsSize				(OSType id);
		void		DeletePrefs					(OSType id); // no fail if not found
		Boolean PrefExists					(OSType id);
		
		Boolean	GetBooleanPrefs			(OSType id);
		void		SetBooleanPrefs			(OSType id, Boolean b);

		short		GetShortPrefs				(OSType id);
		void		SetShortPrefs				(OSType id, short i);

		long		GetLongPrefs				(OSType id);
		void		SetLongPrefs				(OSType id, long l);

		void		GetVPointPrefs			(OSType id, VPoint &vp);
		void		SetVPointPrefs			(OSType id, const VPoint &const vp);

		void		GetVRectPrefs				(OSType id, VRect &vr);
		void		SetVRectPrefs				(OSType id, const VRect &const vr);
		void		GetSilentVRectPrefs	(OSType id, VRect &vr); // 0 when new
		
		void		GetTextStylePrefs		(OSType id, TextStyle &theTextStyle);
		void		SetTextStylePrefs		(OSType id, const TextStyle &theTextStyle);
		
		OSType	GetSignaturePrefs		(OSType id);
		void		SetSignaturePrefs		(OSType id, const OSType signature);

		void		GetStringPrefs			(OSType id, CStr255 &prefs);
		void		SetStringPrefs			(OSType id, const CStr255 &prefs);
		void		SetStringPrefs			(OSType id, const char *prefs);

		Handle	GetHandlePrefs			(OSType id); // returns created handle, caller owns it
		void		SetHandlePrefs			(OSType id, Handle h); // makes copy of handle contents

		Ptr			GetPtrPrefs					(OSType id); // returns created ptr, caller owns it
		void		SetPtrPrefs					(OSType id, Ptr p); // makes copy of ptr contents

		// general alias
		AliasHandle
						GetAliasHandlePrefs	(OSType id);
		void		SetAliasHandlePrefs	(OSType id, AliasHandle ah);
		//- specific for file aliases
		void		GetAliasPrefs				(OSType id, FSSpec &spec);
		Boolean	TryGetAliasPrefs		(OSType id, FSSpec &spec); // fail -> return false
		void		SetAliasPrefs				(OSType id, const FSSpec &const spec);
		//- specific for directory alises
		void		GetDirAliasPrefs		(OSType id, FSSpec &spec); // name unused
		void		SetDirAliasPrefs		(OSType id, const FSSpec &const spec);
		void		GetSilentDirAliasPrefs(OSType id, FSSpec &spec, const OSType folderType = 'desk'); // desktop folder
		// if could not find folder, changes prefs to 'folderType' (FolderMgr)
	
		void		SetWindowPosPrefs		(OSType id, TWindow *window);
		void		GetSilentWindowPosPrefs(OSType id, TWindow *window);
		
		void		SetApplNameAndID		(OSType id, OSType applID, const CStr255 &name);
		void		GetApplNameAndID		(OSType id, OSType &applID, CStr255 &name);

		//----------------------------------------------
		virtual void DeleteAll();
		virtual Boolean IsDirty();
		
		virtual void DoRead(TStream *aStream);
		virtual void DoWrite(TStream *aStream);
		virtual long NeededDiskSpace();

		void DumpPrefs(); // only available in qDebug
		
		PPrefsDatabase();
		void IPrefsDatabase();
		virtual ~PPrefsDatabase();
	protected:
		PDynDynArray *fPrefsDB;
		TLongintList *fPrefsTypes;
		Boolean fPrefsIsValid;
		Boolean fIsDirty;

		long FindIndex(OSType id, Boolean failIfMissing = true); // kEmptyIndex when not found
		Ptr ComputePrefsAddress(OSType id);
		inline long RoundSize(long size);

		void SetDirtyFlag();
		
		inline void DebugCheckSize(OSType id, long size);
		void ReportWrongSize(OSType id, long size);
		void DumpTable();
};

extern PPrefsDatabase *gPrefs;
