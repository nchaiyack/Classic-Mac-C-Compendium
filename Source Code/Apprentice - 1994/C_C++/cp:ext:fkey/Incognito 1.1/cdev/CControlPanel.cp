#ifndef __FILES__
#include <Files.h>
#endif

#ifndef __FOLDERS__
#include <Folders.h>
#endif

#ifndef __DEVICES__
#include <Devices.h>
#endif

#ifndef __RESOURCES__
#include <Resources.h>
#endif

#ifndef __PLSTRINGFUNCS__
#include <PLStringFuncs.h>
#endif

#ifndef __DIALOGS__
#include <Dialogs.h>
#endif

#ifndef __DIALOGUTIL__
#include <DialogUtil.h>
#endif

#ifndef __TOOLUTILS__
#include <ToolUtils.h>
#endif

#ifndef __CCONTROLPANEL__
#include "CControlPanel.h"
#endif

#ifndef __CChooserDialog__
#include "ChooserDialog.h"
#endif


long CControlPanel::DispatchMessage(short theMessage)
{
	long result = (long) this;
	switch (theMessage)
	{
		case initDev:			// init dialog
			result = DoInit();
			break;
		case hitDev:			// item hit
			result = DoHit();
			break;
		case closeDev:			// cdev closed
			result = DoClose();
			break;
		case nulDev:			// null event
			break;
		case updateDev:			// update stuff !updated by dialog
			result = DoUpdate();
			break;
		case activDev:			// activating
			result = DoActivate();
			break;
		case deactivDev:		// deactivating
			result = DoDeactivate();
			break;
		case keyEvtDev:			// keydown
			result = DoKey();
			break;
		case undoDev:			// undo chosen
		case cutDev:			// cut chosen
		case copyDev:			// copy chosen
		case pasteDev:			// paste chosen
		case clearDev:			// clear chosen
		default:
			break;
	}
	return result;
}

#pragma segment Main
CControlPanel::CControlPanel(short numItems)
{
	fNumItems = numItems;
}


#pragma segment Main
CControlPanel::~CControlPanel()
{
	
}


#pragma segment Main
void	CControlPanel::SetArguments(short item, EventRecord *theEvent, DialogPtr theDialog)
{
	fTheEvent = theEvent;
	fItemHit = item - fNumItems;
	fTheDialog = theDialog;
}


#pragma segment Main
long	CControlPanel::DoInit()
{
	long	result = (long) this;
	
	fTheList = new CListManager(kListBox, fTheDialog);
	if (fTheList) 
	{
		result = ReadPreferencesIntoList();
	}
	return result;
}

#pragma segment Main
long	CControlPanel::DoHit()
{
	long	result = (long) this;
	
	switch (fItemHit)
	{
		case kAddButton:
		{
			CChooserDialog	*theDialog;
			
			theDialog = new CChooserDialog();
			if (theDialog)
			{
				theDialog->Initialize(kChooseDialog);
				theDialog->SetChosenList(fTheList);
				theDialog->Run();
				delete theDialog;
			}
			break;
		}
		case kListBox:
			fTheList->Click(fTheEvent->where, fTheEvent->modifiers);
			hiliteItem(fTheDialog, kRemoveButton, fTheList->IsSelection() ? 0 : 255);
			break;
		case kRemoveButton:
			fTheList->DeleteSelection();
			hiliteItem(fTheDialog, kRemoveButton, 255);
			break;
		default:
			break;
	}
	return result;
}


#pragma segment Main
long	CControlPanel::DoClose()
{
	SavePreferences();
	delete fTheList;
	return (long) this;
}


#pragma segment Main
long	CControlPanel::DoUpdate()
{
	DrawSmallFrame(fTheDialog, kListBox);
	fTheList->Update();
	return (long) this;
}


#pragma segment Main
long	CControlPanel::DoActivate()
{
	hiliteItem(fTheDialog, kAddButton, 0);
	hiliteItem(fTheDialog, kRemoveButton, fTheList->IsSelection() ? 0 : 255);
	fTheList->Activate();
	return (long) this;
}


#pragma segment Main
long	CControlPanel::DoDeactivate()
{
	fTheList->Deactivate();
	hiliteItem(fTheDialog, kAddButton, 255);
	hiliteItem(fTheDialog, kRemoveButton, 255);
	return (long)this;
}


#pragma segment Main
long	CControlPanel::DoKey()
{
	Cell	theCell;
	char	key = (char)(fTheEvent->message & charCodeMask);
	if (key == 0x08)
	fTheList->DeleteSelection();
	return (long) this;
}

#pragma segment Main



#pragma segment Main
void CControlPanel::InsertItems()
{
	Handle			temp;
	Str32			rName;
	short			rID;
	OSType			rType;
	short			numTypes = Count1Resources('KILL');
	
	while (numTypes)
	{
		temp = Get1IndResource('KILL', numTypes);
		if (temp)
		{
			GetResInfo(temp, &rID, &rType, rName);
			ReleaseResource(temp);
			fTheList->InsertItem(rName);
		}
		numTypes--;
	}
}

#pragma segment Main
void CControlPanel::SavePreferences()
{
	Str32	theString;
	short	refNum;
	FSSpec	theFile;
	Handle	theHandle = NewHandle(1);
	long	result = (long) this;
	Cell	theCell;
	
	FindFolder(-1, kPreferencesFolderType, true, &theFile.vRefNum, &theFile.parID);
	GetIndString(theString, kPrefsName, 0);
	if (!theString[0]) PLstrcpy(theFile.name, "\pIncognito Prefs");
	else PLstrcpy(theFile.name, theString);
	refNum = FSpOpenResFile(&theFile, fsRdWrPerm);
	if (refNum == -1) return;
	
	RemovePreferences();
	theCell.v = 0;
	theCell.h = 0;
	if (fTheList->GetFirstItem(theString, theCell))
	do
	{
		AddResource(theHandle, 'KILL', theCell.v, theString);
		WriteResource(theHandle);
		DetachResource(theHandle);
	} while (fTheList->GetNextItem(theString, theCell));
	DisposeHandle(theHandle);
	CloseResFile(refNum);
}

#pragma segment Main
void CControlPanel::RemovePreferences()
{
	Handle			temp;
	Str32			rName;
	short			rID;
	OSType			rType;
	short			numTypes = Count1Resources('KILL');
	
	while (numTypes)
	{
		temp = Get1IndResource('KILL', numTypes);
		if (temp)
		{
			RemoveResource(temp);
		}
		numTypes--;
	}
	UpdateResFile(CurResFile());
}


#pragma segment Main
long CControlPanel::ReadPreferencesIntoList(void)
{
	Str255	theString;
	short	refNum;
	FSSpec	theFile;
	long	result = (long) this;
	
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
		InsertItems();
		CloseResFile(refNum);
	} else result = cdevResErr;
	return result;
}

#pragma segment Main
void	CControlPanel::SetupDialog()
{
	
}

