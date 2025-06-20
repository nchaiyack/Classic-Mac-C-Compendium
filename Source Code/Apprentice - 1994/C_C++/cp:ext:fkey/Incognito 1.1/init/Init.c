#include <Memory.h>
#include <Resources.h>
#include <GestaltValue.h>
#include <Exceptions.h>
#include <PLstringfuncs.h>
#include <Folders.h>
#include <TextUtils.h>

/*

asm patch.a
asm install.a
c nbpaction.c -b -opt full -w -d SystemSevenOrLater -sym full -mbg off
c init.c -opt full -w -d SystemSevenOrLater -sym full -mbg off
link install.a.o init.c.o patch.a.o nbpaction.c.o showinit.a.o -ra Main=resSysHeap,resLocked -m INITInstall Compass:Development:MPW:Libraries:Libraries:GestaltValue.o Compass:Development:MPW:Libraries:Libraries:Interface.o Compass:Development:MPW:Libraries:Libraries:Runtime.o -o 'Incog' -t INIT -c HEID -rt INIT -mf -Sym 'On '
Rez ingoc.r -append -o incog
rename Incog.sym 'Incog/INIT_0.sym'
move Incog 'sextant:system folder:extensions:' -y
move 'Incog/INIT_0.sym' 'sextant:system folder:extensions:' -y
*/

#define kSuccessIcon	-4064
#define kFailureIcon	-4033
#define kPrefsName		0

typedef struct NameLink
{
	Str32				originalName;
	Str32				newString;
	struct	NameLink	*next, *previous;
} NameLink, *NameLinkPtr;

typedef struct OriginalLink
{
	Str32	originalName;
	struct OriginalLink *next;
} OriginalLink, *OriginalLinkPtr;

typedef struct
{
	NameLinkPtr		registeredNames;
	OriginalLinkPtr	trappedNames;
} MyGestaltRecord, *MyGestaltPtr;

pascal void ShowInit(short iconID, short moveX);

static void InsertItems(OriginalLinkPtr pb)
{
	Handle			temp;
	Str32			rName;
	short			rID;
	OSType			rType;
	OriginalLinkPtr newLink;
	short			numTypes = Count1Resources('KILL');
	
	while (numTypes)
	{
		temp = Get1IndResource('KILL', numTypes);
		if (temp)
		{
			GetResInfo(temp, &rID, &rType, rName);
			ReleaseResource(temp);
			newLink = (OriginalLinkPtr) NewPtrSysClear(sizeof(OriginalLink));
			if (!newLink) return;
			PLstrcpy(newLink->originalName, rName);
			pb->next = newLink;
			pb = pb->next;
		}
		numTypes--;
	}
}

static void InsertNames(OriginalLinkPtr pb)
{
	Str255	theString;
	short	refNum;
	FSSpec	theFile;
	
	FindFolder(-1, kPreferencesFolderType, true, &theFile.vRefNum, &theFile.parID);
	GetIndString(theString, kPrefsName, 1);
	if (!theString[0])
	{
		PLstrcpy(theFile.name, "\pIncognito Prefs");
	}
	PLstrcpy(theFile.name, theString);
	refNum = FSpOpenResFile(&theFile, fsRdPerm);
	if (refNum == -1)
	{
		FSpCreateResFile(&theFile, 'HEID', 'pref', 0);
		refNum = FSpOpenResFile(&theFile, fsRdPerm);
	}
	if (refNum != -1)
	{
		InsertItems(pb);
		CloseResFile(refNum);
	};
}

Boolean Initialize(void)
{
	OSErr			error;
	MyGestaltPtr	pb;
	NameLinkPtr		pc;
	OriginalLinkPtr	pd;
	
	pb = (MyGestaltPtr)NewPtrSysClear(sizeof(MyGestaltRecord));
	require(pb, mygestalt);
	
	pc = (NameLinkPtr)NewPtrSysClear(sizeof(NameLink));
	require(pc, namelink);
//	pc->originalName[0] = 32;
	
	pd = (OriginalLinkPtr)NewPtrSysClear(sizeof(OriginalLink));
	require(pd, origlink);
//	pd->originalName[0] = 32;
	
	pb->registeredNames = pc;
	pb->trappedNames = pd;
	
	InsertNames(pd);
	
	error = NewGestaltValue('HEID', (long) pb);
	nrequire(error, newgestalt);
	
	ShowInit(kSuccessIcon, 40);
	return noErr;
	
newgestalt:
	DisposePtr((Ptr) pd);
origlink:
	DisposePtr((Ptr) pc);
namelink:
	DisposePtr((Ptr) pb);
mygestalt:
	ShowInit(kFailureIcon, 40);
	return error;
}