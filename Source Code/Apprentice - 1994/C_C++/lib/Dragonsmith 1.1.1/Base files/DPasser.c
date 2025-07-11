/*
	DPasser.c
	
	Pass 'odoc' events to other processes (e.g., other dragons running under the THINK C Debugger)
					
	NOTE:	I'm not sure what to do about stationery handling�  Then again, who's gonna drop
			stationery on a dragon?  Drop and RUN!!!
	
	Created	25 Apr 1992	Basic code, doesn't work
	Modified	29 Aug 1992	Began a rewrite to use the new Dragonsmith
						First build (v0.1) � I'll use TMON Pro to debug it; then, when it's stable, I
							can debug it with (well, another copy of) itself!
			04 Sep 1992	Added Options menu with a single item Target�
			15 Jan 1994	Fixed memory leak in PassDroppings
			
	
	Copyright � 1992�1994 by Paul M. Hoffman
	Send comments or suggestions to paul.hoffman@umich.edu -or- dragonsmith@umich.edu
	
	This code may be freely used, altered, and distributed in any way you want as long as:
		1.	It is GIVEN away rather than sold;
		2.	This statement and the above copyright notice are left intact.
	
*/
									
#include	"Dragon.h"
#include	<EPPC.h>
#include	<Packages.h>		// For IUEqualString

#define	kNoDefaultApp		FALSE

enum {
	mOptions = mEdit + 1
};

enum {
	iChooseTarget = 1
};

class DPasser: public Dragon {
	
	protected:
		AppleEvent		odocEvent;
		PortInfoRec		targetPortInfo;
		TargetID			targetID;		
		AEAddressDesc	targetAddress;
		Boolean			haveTarget;
		MenuHandle		optionsMenu;

	public:
						DPasser (void);
		virtual void		Start (void);
		virtual OSErr		ProcessDroppings (AEDescList *docList);	// Override
		virtual OSErr		PassDroppings (AEDescList *docList);
		virtual Boolean	ProcessLoneDoc (AEDescList *docList);
		virtual void		AskForTarget (void);
	
	protected:
		virtual void		SetUpMenus (void);
		virtual void		DoMenu (long menuItemCode);
		virtual void		DoOptionsMenu (short itemNum);
};

// Filter function for PPCBrowser �
	pascal Boolean FilterOutMyself (LocationNameRec *locName, PortInfoRec *portInfo);

Dragon *CreateGDragon (void)
{
	return (Dragon *) new DPasser;
}

DPasser::DPasser (void)
{
	targetAddress.descriptorType = typeNull;
	targetAddress.dataHandle = NULL;
	
	HLockHi ((Handle) this);		// This simplifies a lot of things later�
	
	haveTarget = FALSE;

	optionsMenu = NULL;
}

void DPasser::Start (void)
{
	OSErr	err;
	
	inherited::Start ();
	
	err = PPCInit ();
	if (err != noErr)
		Abort (err);
	
}

OSErr DPasser::ProcessDroppings (AEDescList *docList)
{
	long			numDroppings;
	OSErr		err;
	AppleEvent	theEvent;
	
	AEDesc		srcDesc;
	DescType	retType;
	short		source;
	long			retSize;
	
	err = AEGetTheCurrentEvent (&theEvent);
	err = AEGetAttributePtr (&theEvent, 'esrc', 'shor', &retType, (Ptr) &source, sizeof (source), &retSize);
	
	err = AECountItems (docList, &numDroppings);
	if (err == noErr) {
		BeginProcessing ();
		if (numDroppings != 1 || !ProcessLoneDoc (docList)) {
			if (!haveTarget)
				AskForTarget ();
			if (haveTarget)
				err = PassDroppings (docList);
		}
		EndProcessing ();
	}
	return err;
}

OSErr DPasser::PassDroppings (AEDescList *docList)
{
	OSErr		err;
	AppleEvent	reply;
	
	err = AECreateAppleEvent (kCoreEventClass, kAEOpenDocuments, &targetAddress,
							kAutoGenerateReturnID, kAnyTransactionID, &odocEvent);
	if (err != noErr) return err;
	
	err = AEPutParamDesc (&odocEvent, keyDirectObject, docList);
	if (err != noErr) return err;
	
	err = AESend (&odocEvent, &reply, kAENoReply + kAECanInteract,
											kAENormalPriority, 60, NULL, NULL);
// BEGIN bug-fix 1.1.1
	(void) AEDisposeDesc (&odocEvent);	// Don't forget to dispose of our copy of the sent event!
// END bug-fix 1.1.1
	
	return err;
}

Boolean DPasser::ProcessLoneDoc (AEDescList *docList)
{
	// Return TRUE if we were able to process the doc all by itself, or FALSE if there's nothing special about it

	return FALSE;		// I'll implement drag-and-drop of "special files" later
}

void DPasser::AskForTarget (void)
{
	EventRecord		event;
	OSErr			err;
	unsigned char		prompt[] = "\pChoose a program to pass the event to:";		// Hard-coded string (tsk, tsk!)
	unsigned char		appListLabel[] = "\pPrograms:";						// Hard-coded string (tsk, tsk!)
	AEAddressDesc	targetAddress;
	
	err = PPCBrowser ((ConstStr255Param) &prompt, (ConstStr255Param) &appListLabel,
				kNoDefaultApp, &targetID.location, &targetPortInfo, FilterOutMyself, NULL);
	if (err == noErr) {
		targetID.name = targetPortInfo.name;		// Don't worry, this is a struct copy, not an array copy
		AEDisposeDesc (&this->targetAddress);	// Get rid of the old target (if there was one)
		err = AECreateDesc (typeTargetID, (Ptr) &targetID, sizeof (TargetID), &targetAddress);
		if (err == noErr) {
			this->targetAddress = targetAddress;
			haveTarget = TRUE;
		}
	}
}

void DPasser::SetUpMenus (void)
{
	inherited::SetUpMenus ();			// Add the Apple, File, and Edit menus

	optionsMenu = GetMenu (mOptions);
	InsertMenu (optionsMenu, 0);
	
	DrawMenuBar ();
}

void DPasser::DoMenu (long menuItemCode)
{
	short	menuID, itemNum;

	menuID = menuItemCode >> 16;
	itemNum = menuItemCode & 0xFFFF;

	if (menuID == mOptions)
		DoOptionsMenu (itemNum);
	else
		inherited::DoMenu (menuItemCode);
}

void DPasser::DoOptionsMenu (short itemNum)
{
	switch (itemNum) {
		case iChooseTarget:
			AskForTarget ();
			break;
		default:
			break;
	}
}

pascal Boolean FilterOutMyself (LocationNameRec *locName, PortInfoRec *portInfo)
{
	return IUEqualString (portInfo->name.name, CurApName);		// IUEqualString doesn't move memory, so this is safe
}