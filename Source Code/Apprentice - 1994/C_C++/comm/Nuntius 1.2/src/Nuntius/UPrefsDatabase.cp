// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UPrefsDatabase.cp

#include "UPrefsDatabase.h"
#include "UDynDynArray.h"
#include "Tools.h"
#include "StreamTools.h"

#include <RsrcGlobals.h>
#include <ErrorGlobals.h>

#include <Errors.h>
#include <Folders.h>
#include <Fonts.h>

#pragma segment MyTools

#define qDebugPrefs qDebug & 0

const long kCurrentPrefsVersion = 10;
const long kMinPrefsVersion = 10;

PPrefsDatabase *gPrefs = nil;
//-----------------------------------------------------------------------------
PPrefsDatabase::PPrefsDatabase()
	: PPtrObject()
{
	fPrefsDB = nil;
	fPrefsTypes = nil;
	fIsDirty = false;
}

void PPrefsDatabase::IPrefsDatabase()
{
	FailInfo fi;
	if (fi.Try())
	{
		PDynDynArray *ddList = new PDynDynArray();
		ddList->IDynDynArray(1024);
		fPrefsDB = ddList;

		TLongintList *lList = new TLongintList();
		lList->ILongintList();
		fPrefsTypes = lList;
		
		gPrefs = this;
		fi.Success();
	}
	else // fail
	{
		FreeIfPtrObject(this);
		fi.ReSignal();
	}
}

PPrefsDatabase::~PPrefsDatabase()
{
	if (!gPrefs)
	{
#if qDebug
		ProgramBreak("gPrefs == nil");
#endif
		return;
	}
	gPrefs = nil;
	FreeIfPtrObject(fPrefsDB); fPrefsDB = nil;
	FreeIfObject(fPrefsTypes); fPrefsTypes = nil;
}

void PPrefsDatabase::DoRead(TStream *aStream)
{
	long version = aStream->ReadLong();
	if (!MyCheckVersion(version, kMinPrefsVersion, kCurrentPrefsVersion, "PPrefsDatabase.Real"))
	{
		gApplication->ShowError(errIncompatiblePrefsFileFormat, messageUsePrefs);
		return;
	}
	ReadDynamicArray(aStream, fPrefsTypes);
	fPrefsDB->DoRead(aStream);
#if qDebugPrefs
	fprintf(stderr, "After reading pref's from disk:\n");
	DumpTable();
#endif
	fIsDirty = false;
	while (fPrefsDB->GetSize() > fPrefsTypes->fSize)
		fPrefsDB->DeleteElementAt(fPrefsDB->GetSize());
}

void PPrefsDatabase::DoWrite(TStream *aStream)
{
	aStream->WriteLong(kCurrentPrefsVersion);
	WriteDynamicArray(aStream, fPrefsTypes);
	fPrefsDB->DoWrite(aStream);
	fIsDirty = false;
}

long PPrefsDatabase::NeededDiskSpace()
{
	return	sizeof(long) + // version
					MyStreamSizeOfDynamicArray(fPrefsTypes) +
					fPrefsDB->NeededDiskSpace();
}

void PPrefsDatabase::DeleteAll()
{
	fPrefsDB->DeleteAll();
	fPrefsTypes->DeleteAll();
}

//======== SUPPORT ==========================================================
void PPrefsDatabase::SetDirtyFlag()
{
	if (!fIsDirty)
		fIsDirty = true;
}

Boolean PPrefsDatabase::IsDirty()
{
	return fIsDirty;
}

inline long PPrefsDatabase::RoundSize(long size)
{
	return (size + 3) & ~3;
}

long PPrefsDatabase::FindIndex(OSType id, Boolean failIfMissing)
{
	long noTypes = fPrefsTypes->GetSize();
	if (noTypes)
	{
		OSType *typeP = (OSType*) fPrefsTypes->ComputeAddress(1);
		for (long index = 1; index <= noTypes; index++)
			if (*typeP++ == id)
				return index;
	}
	if (failIfMissing)
	{
#if qDebug
		fprintf(stderr, "Prefs entry not found for type %s\n", OSType2String(id));
#endif
		FailOSErr(errNoSuchPrefs);
	}
	return kEmptyIndex;
}

Ptr PPrefsDatabase::ComputePrefsAddress(OSType id)
{
	return fPrefsDB->ComputeAddress(FindIndex(id));
}

void PPrefsDatabase::DumpTable()
{
#if qDebug
	fprintf(stderr, "Dumping preferences table:\n");
	for (long index = 1; index <= fPrefsTypes->GetSize(); index++)
	{
		Ptr p = fPrefsDB->ComputeAddress(index);
		long data = *( (long*) p );
		long offset = p - fPrefsDB->ComputeAddress(1);
		fprintf(stderr, "%3ld, id = %s, data-size =%3ld, first 4 bytes = %8lx, offset = %ld\n", 
			index, OSType2String(fPrefsTypes->At(index)), 
			fPrefsDB->GetElementSize(index), data, offset);
	}
	fprintf(stderr, "End of dump\n");
#endif
}

#if qDebug
void PPrefsDatabase::ReportWrongSize(OSType id, long size)
{
	ArrayIndex index = FindIndex(id);
	fprintf(stderr, "Wrong element size (%ld) found, %ld expected\n",
		fPrefsDB->GetElementSize(index), size);
	fprintf(stderr, "index = %ld for id: %s\n", index, OSType2String(id));
	DumpTable();
	ProgramBreak(gEmptyString);
	fPrefsDB->SetElementSize(index, size);
}
#else
void PPrefsDatabase::ReportWrongSize(OSType /* id */, long /* size */)
{
}
#endif

#if qDebug
inline void PPrefsDatabase::DebugCheckSize(OSType id, long size)
{
	long realSize = fPrefsDB->GetElementSize(FindIndex(id));
	if (realSize != RoundSize(size))
		ReportWrongSize(id, size);			
}
#else
inline void PPrefsDatabase::DebugCheckSize(OSType /* id */, long /* size */)
{
}
#endif

void PPrefsDatabase::SetPrefs(OSType id, const void *dataP, long size)
{
#if qDebugPrefs
	fprintf(stderr, "\nSetting data id %s with size %ld\n", OSType2String(id), size);
	if (size != RoundSize(size))
		fprintf(stderr, "- size rounded to %ld\n", size, RoundSize(size));
#endif
	size = RoundSize(size);
	ArrayIndex index = FindIndex(id, false);
	if (index == kEmptyIndex)
	{
		index = fPrefsDB->CreateNewElement(size);
		fPrefsTypes->InsertLast(id);
		fPrefsTypes->GetSize();
		SetDirtyFlag();
	}
	if (fPrefsDB->GetElementSize(index) != size)
	{
		SetDirtyFlag(); // changed size
		fPrefsDB->SetElementSize(index, size);
	}
	if (dataP == nil || size == 0)
		return;
	Ptr toP = fPrefsDB->ComputeAddress(index);
	if (memcmp(dataP, toP, int(size)) == 0)
		return; // didn't change contents
	BytesMove(dataP, toP, size);
	SetDirtyFlag();
}

void PPrefsDatabase::GetPrefs(OSType id, void *data)
{
	ArrayIndex index = FindIndex(id);
	long size = fPrefsDB->GetElementSize(index);
	Ptr fromP = fPrefsDB->ComputeAddress(index);
	BytesMove(fromP, data, size);
}

long PPrefsDatabase::GetPrefsSize(OSType id)
{
	ArrayIndex index = FindIndex(id);
	return fPrefsDB->GetElementSize(index);
}

void PPrefsDatabase::DeletePrefs(OSType id)
{
	ArrayIndex index = FindIndex(id, false);
	if (index == kEmptyIndex)
		return;
	fPrefsDB->DeleteElementAt(index);
	fPrefsTypes->DeleteElementsAt(index, 1);
	SetDirtyFlag();
}

Boolean PPrefsDatabase::PrefExists(OSType id)
{
	return FindIndex(id, false) != kEmptyIndex;
}

Boolean PPrefsDatabase::GetBooleanPrefs(OSType id)
{
	DebugCheckSize(id, sizeof(long));
	long value = *( (long*) ComputePrefsAddress(id));
	return Boolean(value);
}

void PPrefsDatabase::SetBooleanPrefs(OSType id, Boolean b)
{
	long li = b;
	SetPrefs(id, &li, sizeof(long));
}

short PPrefsDatabase::GetShortPrefs(OSType id)
{
	DebugCheckSize(id, sizeof(long));
	return short( *( (long*) ComputePrefsAddress(id)) );
}

void PPrefsDatabase::SetShortPrefs(OSType id, short i)
{
	long li = i;
	SetPrefs(id, &li, sizeof(long));
}

long PPrefsDatabase::GetLongPrefs(OSType id)
{
	DebugCheckSize(id, sizeof(long));
	return *( (long*) ComputePrefsAddress(id));
}

void PPrefsDatabase::SetLongPrefs(OSType id, long l)
{
	SetPrefs(id, &l, sizeof(long));
}

void PPrefsDatabase::GetVPointPrefs(OSType id, VPoint &vp)
{
	DebugCheckSize(id, sizeof(VPoint));
	vp = *( (VPoint*) ComputePrefsAddress(id));
}

void PPrefsDatabase::SetVPointPrefs(OSType id, const VPoint &const vp)
{
	SetPrefs(id, &vp, sizeof(VPoint));
}

void PPrefsDatabase::GetVRectPrefs(OSType id, VRect &vr)
{
	DebugCheckSize(id, sizeof(VRect));
	vr = *( (VRect*) ComputePrefsAddress(id));
}

void PPrefsDatabase::SetVRectPrefs(OSType id, const VRect &const vr)
{
	SetPrefs(id, &vr, sizeof(VRect));
}

void PPrefsDatabase::GetSilentVRectPrefs(OSType id, VRect &vr)
{
	FailInfo fi;
	if (fi.Try())
	{
		GetVRectPrefs(id, vr);
		fi.Success();
		return;
	}
	else // fail
	{
		if (fi.error != errNoSuchPrefs)
			fi.ReSignal();
	}
	vr = VRect(0, 0, 0, 0);
	SetVRectPrefs(id, vr);
}


void PPrefsDatabase::GetTextStylePrefs(OSType id, TextStyle &theTextStyle)
{
	FailInfo fi;
	if (fi.Try())
	{
		MATextStyle *matsP;
		DebugCheckSize(id, sizeof(MATextStyle));
		matsP = (MATextStyle*) ComputePrefsAddress(id);
		theTextStyle.tsFace = matsP->tsFace;
		theTextStyle.tsSize = matsP->tsSize;
		theTextStyle.tsColor = matsP->tsColor;
		CStr255 name(matsP->tsFont);
		theTextStyle.tsFont = GetFontNum(name);		
		fi.Success();
	}
	else // fail
	{
		if (fi.error != errNoSuchPrefs)
			fi.ReSignal();
		theTextStyle = gApplicationStyle;
	}
}

void PPrefsDatabase::SetTextStylePrefs(OSType id, const TextStyle &theTextStyle)
{
	MATextStyle mats;
	mats.tsFace = theTextStyle.tsFace;
	mats.tsSize = theTextStyle.tsSize;
	mats.tsColor = theTextStyle.tsColor;
	CStr255 name;
	GetFontName(theTextStyle.tsFont, name);
	CopyCString2String(name, mats.tsFont);
	SetPrefs(id, &mats, sizeof(MATextStyle));
}

OSType PPrefsDatabase::GetSignaturePrefs(OSType id)
{
	DebugCheckSize(id, sizeof(OSType));
	return *( (OSType*) ComputePrefsAddress(id));
}

void PPrefsDatabase::SetSignaturePrefs(OSType id, const OSType signature)
{
	SetPrefs(id, &signature, sizeof(OSType));
}

void PPrefsDatabase::GetStringPrefs(OSType id, CStr255 &prefs)
{
	Ptr p = ComputePrefsAddress(id);
	long len = CStringPtr(p)->Length();
	long size = (len + 8) & ~7;
	DebugCheckSize(id, size);
	BytesMove(p, &prefs, len + 1);
}

void PPrefsDatabase::SetStringPrefs(OSType id, const CStr255 &prefs)
{
	long size = (prefs.Length() + 8) & ~7; // remember length byte
	SetPrefs(id, &prefs, size);
}

void PPrefsDatabase::SetStringPrefs(OSType id, const char *prefs)
{
	CStr255 s(prefs);
	SetStringPrefs(id, s);
}

Handle PPrefsDatabase::GetHandlePrefs(OSType id)
{
	long size = *( (long*) ComputePrefsAddress(id));	
	DebugCheckSize(id, sizeof(long) + size);
	Handle h = NewPermHandle(size); // can move mem
	Ptr p = ComputePrefsAddress(id);
	BytesMove(p + sizeof(long), *h, size);
	return h;
}

void PPrefsDatabase::SetHandlePrefs(OSType id, Handle h)
{
	long size = GetHandleSize(h);
	SetPrefs(id, nil, size + 4);
	Ptr prefP = ComputePrefsAddress(id);
	long *lP = (long*)prefP;
	Ptr toP = prefP + sizeof(long);
	Boolean changed = (*lP != size);
	*lP = size;
	if (!changed && memcmp(*h, toP, int(size)) != 0)
		changed = true;
	BytesMove(*h, toP, size);
	if (changed)
		SetDirtyFlag();
}

Ptr PPrefsDatabase::GetPtrPrefs(OSType id)
{
	long size = *( (long*) ComputePrefsAddress(id));	
	DebugCheckSize(id, sizeof(long) + size);
	Ptr p = NewPermPtr(size); // can move mem
	Ptr fromP = ComputePrefsAddress(id);
	BytesMove(fromP + sizeof(long), p, size);
	return p;
}

void PPrefsDatabase::SetPtrPrefs(OSType id, Ptr p)
{
	long size = GetPtrSize(p);
	SetPrefs(id, nil, size + 4);
	Ptr prefP = ComputePrefsAddress(id);
	long *lP = (long*)prefP;
	Ptr toP = prefP + sizeof(long);
	Boolean changed = (*lP != size);
	*lP = size;
	if (!changed && memcmp(p, toP, int(size)) != 0)
		changed = true;
	BytesMove(p, toP, size);
	if (changed)
		SetDirtyFlag();
}

AliasHandle PPrefsDatabase::GetAliasHandlePrefs(OSType id)
{
	return AliasHandle(GetHandlePrefs(id));
}


void PPrefsDatabase::SetAliasHandlePrefs(OSType id, AliasHandle ah)
{
	SetHandlePrefs(id, Handle(ah));
}

void PPrefsDatabase::GetAliasPrefs(OSType id, FSSpec &spec)
{
	AliasHandle ah = AliasHandle(GetHandlePrefs(id));
	VOLATILE(ah);
	FailInfo fi;
	if (fi.Try())
	{
		Boolean wasChanged;
		short err = ResolveAlias(nil, ah, spec, wasChanged);
		if (wasChanged)
			SetHandlePrefs(id, Handle(ah));
		FailOSErr(err);
		DisposeIfHandle(Handle(ah)); ah = nil;
		fi.Success();
	}
	else // fail
	{
		DisposeIfHandle(Handle(ah)); ah = nil;
		fi.ReSignal();
	}
}

Boolean PPrefsDatabase::TryGetAliasPrefs(OSType id, FSSpec &spec)
{
	FailInfo fi;
	if (fi.Try())
	{
		GetAliasPrefs(id, spec);
		fi.Success();
		return true;
	}
	else // fail
	{
		if (fi.error == errNoSuchPrefs)
			return false;
		else
			fi.ReSignal();
	}
}

void PPrefsDatabase::SetAliasPrefs(OSType id, const FSSpec &const spec)
{
	AliasHandle ah = nil;
	VOLATILE(ah);
	FailInfo fi;
	if (fi.Try())
	{
		FailOSErr(NewAlias(nil, spec, ah));
		SetHandlePrefs(id, Handle(ah));
		DisposeIfHandle(Handle(ah)); ah = nil;
		fi.Success();
	}
	else // fail
	{
		DisposeIfHandle(Handle(ah)); ah = nil;
		fi.ReSignal();
	}
}

void PPrefsDatabase::GetDirAliasPrefs(OSType id, FSSpec &spec)
{
	FSSpec tmpSpec;
	GetAliasPrefs(id, tmpSpec);
	CInfoPBRec info;
	info.dirInfo.ioCompletion = nil;
	info.dirInfo.ioNamePtr = tmpSpec.name;
	info.dirInfo.ioVRefNum = tmpSpec.vRefNum;
	info.dirInfo.ioFDirIndex = 0;
	info.dirInfo.ioDrDirID = tmpSpec.parID;
	FailOSErr(PBGetCatInfoSync(&info));
	spec.parID = info.dirInfo.ioDrDirID;
	spec.vRefNum = tmpSpec.vRefNum;
}

void PPrefsDatabase::SetDirAliasPrefs(OSType id, const FSSpec &const spec)
{
	FSSpec tmpSpec;
	tmpSpec.vRefNum = 0; // otherwise unused error
	CInfoPBRec info;
	info.dirInfo.ioCompletion = nil;
	info.dirInfo.ioNamePtr = tmpSpec.name;
	info.dirInfo.ioVRefNum = spec.vRefNum;
	info.dirInfo.ioFDirIndex = -1;
	info.dirInfo.ioDrDirID = spec.parID;
	FailOSErr(PBGetCatInfoSync(&info));
	tmpSpec.parID = info.dirInfo.ioDrParID;
	tmpSpec.vRefNum = info.dirInfo.ioVRefNum;
	SetAliasPrefs(id, tmpSpec);
}

void PPrefsDatabase::GetSilentDirAliasPrefs(OSType id, FSSpec &spec, const OSType folderType)
{
	FailInfo fi;
	if (fi.Try())
	{
		GetDirAliasPrefs(id, spec);
		fi.Success();
		return;
	}
	else // fail
	{
		switch (fi.error)
		{
			case nsvErr:
			case fnfErr:
			case paramErr:
			case dirNFErr:
			case errNoSuchPrefs:
				break; // == not found

			case userCanceledErr:
				Failure(0, 0); // maybe should just juse default folder
				
			default:
				fi.ReSignal();
		}
	}
	short errNo;
	errNo = FindFolder(kOnSystemDisk, folderType, kCreateFolder, spec.vRefNum, spec.parID);
	if (errNo)
		errNo = FindFolder(kOnSystemDisk, kDesktopFolderType, kCreateFolder, spec.vRefNum, spec.parID);
	if (errNo)
		errNo = FindFolder(kOnSystemDisk, kSystemFolderType, kCreateFolder, spec.vRefNum, spec.parID);
		// just _want_ a folder
	FailOSErr(errNo);
	SetDirAliasPrefs(id, spec);
}


void PPrefsDatabase::SetWindowPosPrefs(OSType id, TWindow *window)
{
	SetVRectPrefs(id, VRect(window->fLocation, window->fLocation + window->fSize));
}

void PPrefsDatabase::GetSilentWindowPosPrefs(OSType id, TWindow *window)
{
	FailInfo fi;
	if (fi.Try())
	{
		VRect newFrame;
		GetVRectPrefs(id, newFrame);
		if (!window->fIsResizable)
			newFrame[botRight] = newFrame[topLeft] + window->fSize;
		window->SetFrame(newFrame, kRedraw);		
		window->ForceOnScreen();
		fi.Success();
		return;
	}
	else // fail
	{
		if (fi.error != errNoSuchPrefs)
			fi.ReSignal();
	}
}

void PPrefsDatabase::SetApplNameAndID(OSType id, OSType applID, const CStr255 &name)
{
	long size = ((name.Length() + 8) & ~7) + sizeof(OSType); // remember length byte
	char buffer[sizeof(CStr255) + sizeof(OSType)];
	*OSTypePtr(buffer) = applID;
	BytesMove(&name, buffer + sizeof(OSType), name.Length() + 1);
	SetPrefs(id, buffer, size);
}

void PPrefsDatabase::GetApplNameAndID(OSType id, OSType &applID, CStr255 &name)
{
	Ptr p = ComputePrefsAddress(id);
	applID = *OSTypePtr(p);
	p += sizeof(OSType);
	long len = CStringPtr(p)->Length();
	DebugCheckSize(id, ((len + 8) & ~7) + sizeof(OSType));
	BytesMove(p, &name, len + 1);
}

void PPrefsDatabase::DumpPrefs()
{
#if qDebug
	const long maxDump = 50;
	unsigned char buffer[maxDump + 10];
	char info[100];
	fprintf(stderr, "Dump of preferences database:\n");
	ArrayIndex noPrefs = fPrefsTypes->GetSize();
	for (ArrayIndex index = 1; index <= noPrefs; index++)
	{
		OSType id = fPrefsTypes->At(index);
		fprintf(stderr, "%6ld: ", index);
		fprintf(stderr, "%c%c%c%c", char(id >> 24 & 255), char(id >> 16 & 255), char(id >> 8 & 255), char(id & 255));
		long elemSize = fPrefsDB->GetElementSize(index);
		info[0] = 0;
		if (elemSize == 4)
		{
			long lw = *( (long*) fPrefsDB->ComputeAddress(index));
			if (lw <= 65535 && lw > -1000)
				sprintf(info, "%4ld", lw);
		}
		long dumpSize = Min(maxDump, elemSize);
		BytesMove(fPrefsDB->ComputeAddress(index), buffer, dumpSize);
		buffer[dumpSize] = 0;
		unsigned char *p = buffer;
		for (long i = 1; i <= dumpSize; i++)
		{
			if (*p < 32)
				*p = '.';
			++p;
		}
		fprintf(stderr, "%6ld %4s '%s'\n", elemSize, info, buffer);
	}
	fprintf(stderr, "End of dump\n");
#endif
}
