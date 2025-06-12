#ifndef __DIALOGS__
#include <Dialogs.h>
#endif

#ifndef __APPLETALK__
#include <AppleTalk.h>
#endif

#ifndef __DIALOGUTIL__
#include <DialogUtil.h>
#endif

#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif

#ifndef __EXCEPTIONS__
#include <Exceptions.h>
#endif

#ifndef __EVENTS__
#include <Events.h>
#endif

#ifndef __PLSTRINGFUNCS__
#include <PLStringFuncs.h>
#endif

#ifndef __CHOOSERDIALOG__
#include "ChooserDialog.h"
#endif

#pragma segment Main

CChooserDialog::CChooserDialog()
{
	MPPParamBlock p;
	short err;

	p.MPPioCompletion = 0L;
	p.SETSELF.newSelfFlag = 1;
	PSetSelfSend(&p, false);
	ResetLastKey(0);
}

#pragma segment Main
void CChooserDialog::InitZoneList()
{
	DoBuildZoneList();
	if (!fZoneList->GetHeight())
	{
		fZoneList->InsertItem("\p*");
		fZoneList->SelectItem("\p*");
	} else HiliteUserZone();
}


#pragma segment Main
void CChooserDialog::InitDeviceList()
{
	Str32	theZone;
	
	fBuffer[0] = 1;
	fBuffer[1] = '=';
	fBuffer[2] = 1;
	fBuffer[3] = '=';

	PLstrcpy((StringPtr) &fBuffer[4], fZoneList->GetCurrentSelection(theZone) ? theZone : "\p*");	

	if (!fLookupBuffer)
	{
		fLookupBuffer = NewPtr(253*sizeof(EntityName));
		if (!fLookupBuffer) return;
	}
	
	if (fLookupParam.MPP.ioResult == 1)		// kill outstanding requests, if any
	{
		fLookupParam.NBPKILL.nKillQEl = (Ptr) &fLookupParam.MPP.qLink;
		PKillNBP(&fLookupParam, false);
	}

	fLookupParam.NBP.interval = 5;
	fLookupParam.NBP.count = 4;
	fLookupParam.NBP.NBPPtrs.entityPtr = (Ptr) fBuffer;
	fLookupParam.NBP.parm.Lookup.retBuffPtr = fLookupBuffer;
	fLookupParam.NBP.parm.Lookup.retBuffSize = GetPtrSize(fLookupBuffer);
	fLookupParam.NBP.parm.Lookup.maxToGet = GetPtrSize(fLookupBuffer)/sizeof(EntityName);
	PLookupName(&fLookupParam, true);
}

#pragma segment Main
void CChooserDialog::ChangeActive(short theItem)
{
	CListManager	*theList;
	
	EraseBigFrame(fTheDialog, GetActive());
	switch (GetActive())
	{
		case kZoneList:
			theList = fZoneList;
			break;
		case kChosenList:
			theList = fChosenList;
			break;
		case kDeviceList:
			theList = fDeviceList;
			break;
	}
	theList->Deactivate();
	fActiveItem = theItem;
	switch (GetActive())
	{
		case kZoneList:
			theList = fZoneList;
			break;
		case kChosenList:
			theList = fChosenList;
			break;
		case kDeviceList:
			theList = fDeviceList;
			break;
	}
	theList->Activate();
	DrawBigFrame(fTheDialog, GetActive());
}



#pragma segment Main
CChooserDialog::~CChooserDialog()
{
	delete fChosenList;
	delete fDeviceList;
	delete fZoneList;
	if (fLookupParam.MPP.ioResult == 1)		// kill outstanding requests, if any
	{
		fLookupParam.NBPKILL.nKillQEl = (Ptr) &fLookupParam.MPP.qLink;
		PKillNBP(&fLookupParam, false);
	}
	if (fLookupBuffer) DisposePtr(fLookupBuffer);
	DisposeDialog(fTheDialog);
}


#pragma segment Main



#pragma segment Main
void CChooserDialog::Run()
{
	short	itemHit;
	GrafPtr	oldPort;
	ModalFilterUPP	theFilter;
	
	theFilter = NewModalFilterProc(EventFilterProc);
	GetPort(&oldPort);
	SetPort(fTheDialog);
	hiliteItem(fTheDialog, kAddButton, 255);
	hiliteItem(fTheDialog, kRemoveButton, 255);
	
	do
	{
		ModalDialog(theFilter, &itemHit);
		switch (itemHit)
		{
			case kAddButton:
				DoAddButton();
				break;
			case kRemoveButton:
				DoRemoveButton();
				break;
		case kZoneList:
		case kChosenList:
		case kDeviceList:
			if (itemHit - GetActive())
			{
				ChangeActive(itemHit);
			}
			switch (itemHit)
			{
				case kZoneList:
					DoZoneHit();
					break;
				case kChosenList:
					DoChosenHit();
					break;
				case kDeviceList:
					DoDeviceHit();
					break;
			}
			default:
				break;
		}
		AdjustButtons();
	} while (itemHit != kDoneButton);
	
	DisposeRoutineDescriptor(theFilter);
	SetPort(oldPort);
}

#pragma segment Main
void CChooserDialog::SetChosenList(CListManager *originalList)
{
	Cell	theCell = {0,0};
	short	height = originalList->GetHeight();
	Str32	theItem;
	
	fOriginalList = originalList;
	if (fOriginalList->GetFirstItem(theItem, theCell))
	{
		fChosenList->InsertItem(theItem);
		while (fOriginalList->GetNextItem(theItem, theCell))
		{
			fChosenList->InsertItem(theItem);
		}
	}
}

#pragma segment Main
short CChooserDialog::DoTabCycle(short theDirection)
{
	short	newActive;
	
	switch (GetActive())
	{
		case kZoneList:
			newActive = !theDirection ? kDeviceList : kChosenList;
			break;
		case kDeviceList:
			newActive = !theDirection ? kChosenList : kZoneList;
			break;
		case kChosenList:
			newActive = !theDirection ? kZoneList : kDeviceList;
			break;
	}
	ChangeActive(newActive);
	return GetActive();	
}

#pragma segment Main
void CChooserDialog::Initialize(short resID)
{
	DialogPtr	theDialog;
	CListManager	*theList;
	
	theDialog = GetNewDialog(resID, nil, (WindowPtr) -1);
	require(theDialog, getnewdialog);
	SetWRefCon(theDialog, (long) this);
	
	theList = new CListManager(kChosenList, theDialog);
	require(theList, chosenlist);
	fChosenList = theList;
	theList->Deactivate();
	
	theList = new CListManager(kZoneList, theDialog);
	require(theList, zonelist);
	theList->Deactivate();
	fZoneList = theList;
	InitZoneList();
	
	theList = new CListManager(kDeviceList, theDialog, 2);
	require(theList, devicelist);
	theList->Activate();
	fDeviceList = theList;
	InitDeviceList();
	
	fActiveItem = kDeviceList;
	DrawBigFrame(theDialog, kDeviceList);
	DrawSmallFrame(theDialog, kChosenList);
	DrawSmallFrame(theDialog, kDeviceList);
	DrawSmallFrame(theDialog, kZoneList);

	fTheDialog = theDialog;
	return;

devicelist:
	delete fZoneList;
zonelist:
	delete fChosenList;
chosenlist:
	delete theDialog;
getnewdialog:
	return;
}

#pragma segment Main
void CChooserDialog::DoUpdate(EventRecord *theEvent)
{
	if (theEvent->message == (long) fTheDialog)
	{
		DrawBigFrame(fTheDialog, GetActive());
		DrawSmallFrame(fTheDialog, kZoneList);
		DrawSmallFrame(fTheDialog, kDeviceList);
		DrawSmallFrame(fTheDialog, kChosenList);
		fZoneList->Update();
		fChosenList->Update();
		fDeviceList->Update();
	}
	else
	{
		BeginUpdate((WindowPtr) theEvent->message);
		EndUpdate((WindowPtr) theEvent->message);
	}
}


#pragma segment Main
void CChooserDialog::DoNull(void)
{
	EntityName	theEntity;
	AddrBlock	theAddress;

	if (fLookupParam.MPP.ioResult >= noErr && fLookupParam.NBP.parm.Lookup.numGotten > 0)
	{
		while (fNumGotten < fLookupParam.NBP.parm.Lookup.numGotten)
		{
			fNumGotten += 1;
			if (!NBPExtract(fLookupBuffer, fLookupParam.NBP.parm.Lookup.numGotten, fNumGotten, &theEntity, &theAddress))
			{
				Cell	theCell;
				
				if (!fDeviceList->InsertItem(theEntity.objStr))
				{
					if (fDeviceList->FindItem(theEntity.objStr, theCell))
					{
						fDeviceList->AddItem(theEntity.objStr, theCell);
					}
				}
				else fDeviceList->FindItem(theEntity.objStr, theCell);
				theCell.h+=1;
				fDeviceList->SetItem(theEntity.typeStr, theCell);
			}
		}
	}
}


#pragma segment Main
void CChooserDialog::DoKey(EventRecord *theEvent)
{
	char			key = (char)(theEvent->message & charCodeMask);
	CListManager	*theList;
	Cell			theCell;
	Str32			theItem;
	
	switch (GetActive())
	{
		case kZoneList:
			theList = fZoneList;
			break;
		case kChosenList:
			theList = fChosenList;
			break;
		case kDeviceList:
			theList = fDeviceList;
			break;
	}
	switch (key)
	{
		case 0x08:
			DoRemoveButton();
			ResetLastKey(theEvent->when);
			AdjustButtons();
			break;
		case 0x09:
			DoTabCycle(theEvent->modifiers  & shiftKey);
			ResetLastKey(theEvent->when);
			AdjustButtons();
			break;
		case 0x1c:		// left
			if (theList->IsSelection(theCell))
			{
				theCell.h-=1;
				if (theCell.h < 0) break;
				theList->DeselectList();
				theList->SelectItem(theCell);
			}
			else
			{
				theCell.h = 0;
				theCell.v = 0;
				theList->SelectItem(theCell);
			}
			AdjustButtons();
			theList->AutoScroll();
			ResetLastKey(theEvent->when);
			break;
		case 0x1d:		// right
			if (theList->IsSelection(theCell))
			{
				theCell.h +=1;
				if (theCell.h < theList->GetWidth())
				{
					theList->DeselectList();
					theList->SelectItem(theCell);
				}
			}
			else
			{
				theCell.h = theList->GetWidth()-1;
				theCell.h < 0 ? 0 : theCell.h;
				theCell.v = 0;
				theList->SelectItem(theCell);
			}
			AdjustButtons();
			theList->AutoScroll();
			ResetLastKey(theEvent->when);
			break;
		case 0x1e:		// up arrow
			if (theList->IsSelection(theCell))
			{
				theCell.v-=1;
				if (theCell.v < 0) break;
				theList->DeselectList();
				theList->SelectItem(theCell);
			}
			else
			{
				theCell.h = 0;
				theCell.v = theList->GetHeight() - 1;
				theList->SelectItem(theCell);
			}
			AdjustButtons();
			theList->AutoScroll();
			ResetLastKey(theEvent->when);
			break;
		case 0x1f:		// down arrow
			if (theList->IsSelection(theCell))
			{
				theCell.v+= 1;
				if (theCell.v < theList->GetHeight())
				{
					theList->DeselectList();
					theList->SelectItem(theCell);
				}
			}
			else
			{
				theCell.h = theCell.v = 0;
				theList->SelectItem(theCell);
			}
			theList->AutoScroll();
			AdjustButtons();
			ResetLastKey(theEvent->when);
			break;
		default:
			DoTyping(theEvent, key);
			break;
	}
}


#pragma segment Main
void CChooserDialog::AdjustButtons(void)
{
	Str32	theItem;
	Cell	theCell;
	
	switch (GetActive())
	{
		case kZoneList:
			hiliteItem(fTheDialog, kRemoveButton, 255);
			hiliteItem(fTheDialog, kAddButton, 255);
			break;
		case kChosenList:
			hiliteItem(fTheDialog, kRemoveButton, fChosenList->IsSelection() ? 0 : 255);
			hiliteItem(fTheDialog, kAddButton, 255);
			break;
		case kDeviceList:
			hiliteItem(fTheDialog, kRemoveButton, 255);
			if (fDeviceList->IsSelection(theCell))
			{
				fDeviceList->GetItem(theItem, theCell);
				hiliteItem(fTheDialog, kAddButton, fChosenList->IsItemPresent(theItem) ? 255 : 0);
			}
			else
			{
				hiliteItem(fTheDialog, kAddButton, 255);
			}
			break;
	}
}


#pragma segment Main
void CChooserDialog::DoZoneHit()
{
	Cell	theOldCell, theNewCell;
	
	if (fZoneList->IsSelection(theOldCell))
	{
		fZoneList->Click(fTheEvent.where, fTheEvent.modifiers);
		fZoneList->IsSelection(theNewCell);
		if ((theOldCell.h == theNewCell.h) && (theOldCell.v == theNewCell.v));
		else
		{
			fDeviceList->Empty();
			fNumGotten = 0;
			InitDeviceList();
		}
	}
	else
	{
		fZoneList->Click(fTheEvent.where, fTheEvent.modifiers);
		if (fZoneList->IsSelection())
		{
			fDeviceList->Empty();
			fNumGotten = 0;
			InitDeviceList();
		}
	}
	AdjustButtons();	
}


#pragma segment Main
void CChooserDialog::DoChosenHit()
{
	fChosenList->Click(fTheEvent.where, fTheEvent.modifiers);
	AdjustButtons();
}


#pragma segment Main
void CChooserDialog::DoDeviceHit()
{
	Str32	theItem;
	
	if (fDeviceList->Click(fTheEvent.where, fTheEvent.modifiers))	// if dblÐclick
	{
		DoAddButton();
	}
	AdjustButtons();
}

#pragma segment Main
void CChooserDialog::DoTyping(EventRecord *theEvent, char key)
{
	Cell			theCell;
	long			when = theEvent->when;
	CListManager	*theList;

	if (key < ' ') return;
	if ((when - fLastCharTime) > 30)
	{
		ResetLastKey(when);
	}
	fLastCharTime = when;
	fCharString[++(fCharString[0])] = key;
	switch (GetActive())
	{
		case kZoneList:
			theList = fZoneList;
			break;
		case kDeviceList:
			theList = fDeviceList;
			break;
		case kChosenList:
			theList = fChosenList;
			break;
		default:
			break;
	}
	theList->DeselectList();
	theList->BinarySearchInsensitive(fCharString, theCell);
	theList->SelectItem(theCell);
	AdjustButtons();
}


#pragma segment Main
void CChooserDialog::ResetLastKey(long when)
{
	fCharString[0] = 0;
	fLastCharTime = when;
}

#pragma segment Main
OSErr CChooserDialog::DoBuildZoneList(void)
{
	ATPParamBlock atppb;
	char zones[kZonesSize], * zptr, data[255];
	OSErr err;

	BDSElement dBDS;											// the BDS for GetZoneList call 
	short index, count, i;
	short ignore;
	short nodeNetAddress, bridgeNode;

	dBDS.buffSize = kZonesSize;									// set up BDS 
	dBDS.buffPtr = zones;

	atppb.ATPatpFlags = 0;

	// Get network address of node & node ID of bridge (if any). 

	err = GetNodeAddress(&ignore, &nodeNetAddress);
	if (err) return (err);

	if (!(bridgeNode = GetBridgeAddress())) return (noErr);
	// We have added all zero zones to the ist, so we are done. 

	atppb.ATPaddrBlock.aNet = nodeNetAddress;
	atppb.ATPaddrBlock.aNode = bridgeNode;						// Get node of bridge. 
	atppb.ATPaddrBlock.aSocket = kZIPSocket;					// The socket we want. 
	atppb.ATPreqLength = 0;
	atppb.ATPreqPointer = nil;
	atppb.ATPbdsPointer = (Ptr) & dBDS;
	atppb.ATPnumOfBuffs = 1;
	atppb.ATPtimeOutVal = kATPTimeOutVal;
	atppb.ATPretryCount = kATPRetryCount;

	index = 1;
	count = 0;

	do
	{
		atppb.ATPuserData = kGetZoneListCall + index;			// Indicate GetZoneList request. 
		err = PSendRequest(&atppb, false);						// Send sync request. 
		if (err) return (err);

		count += dBDS.userBytes & kZoneCount;					// find out how many returned 
		zptr = zones;											// put current pointer at start 
		do
		{														// get each zone 
			for (i = zptr[0]; i; --i) data[i - 1] = zptr[i];
			fZoneList->InsertItem((Str32) zptr);
			zptr += (*zptr + 1);								// bump up current pointer
			++index;											// increment which zone
		} while (index <= count);

	} while ((dBDS.userBytes & kMoreZones) == 0);				//	 keep going until none left

	return (noErr);
}


#pragma segment Main
OSErr CChooserDialog::HiliteUserZone(void)
{
	Str32 zone;
	short zoneLen, i;
	OSErr err;

	if (!(err = OldStyleGetMyZone(zone)))
	{
		for (zoneLen = zone[i = 0]; i < zoneLen; ++i) zone[i] = zone[i + 1];
		fZoneList->SelectItem(zone);
	}
	return (err);
}


#pragma segment Main
OSErr CChooserDialog::OldStyleGetMyZone(StringPtr str)
{
	ATPParamBlock atppb;
	OSErr err;

	BDSElement dBDS;										// the BDS for GetZoneList call
	short ignore;
	short nodeNetAddress, bridgeNode;

	dBDS.buffSize = sizeof(Str32);
	dBDS.buffPtr = (Ptr)str;

	atppb.ATPatpFlags = 0;

	// Get network address of node & node ID of bridge (if any).

	err = GetNodeAddress(&ignore, &nodeNetAddress);
	if (err) return (err);

	if (!(bridgeNode = GetBridgeAddress())) return (noErr); // We have added all zero
															// zones to the ist, so we are done.

	atppb.ATPaddrBlock.aNet = nodeNetAddress;
	atppb.ATPaddrBlock.aNode = bridgeNode;					// Get node of bridge.
	atppb.ATPaddrBlock.aSocket = kZIPSocket;				// The socket we want. 
	atppb.ATPreqLength = 0;
	atppb.ATPreqPointer = nil;
	atppb.ATPbdsPointer = (Ptr) & dBDS;
	atppb.ATPnumOfBuffs = 1;
	atppb.ATPtimeOutVal = kATPTimeOutVal;
	atppb.ATPretryCount = kATPRetryCount;

	atppb.ATPuserData = kGetMyZoneCall;						// Indicate GetMyZone request.
	return (PSendRequest(&atppb, false));					// Send sync request
}

#pragma segment Main
pascal Boolean CChooserDialog::EventFilterProc(DialogPtr theDialog, EventRecord *theEvent, short *itemHit)
{
	GrafPtr	oldPort;
	CChooserDialog	*myDialog = (CChooserDialog *)GetWRefCon(theDialog);
	
	GetPort(&oldPort);
	SetPort(theDialog);
	
	switch (theEvent->what)
	{
		case nullEvent:
			myDialog->DoNull();
			break;
		case keyDown:
		case autoKey:
			if (theEvent->modifiers & cmdKey)
			{
				switch (theEvent->message & charCodeMask)
				{
					case 'a':
					case 'A':
						myDialog->DoAddButton();
						break;
					case 'd':
					case 'D':
						*itemHit = kDoneButton;
						SetPort(oldPort);
						return true;
						break;
					case 'R':
					case 'r':
						myDialog->DoRemoveButton();
						break;
				}
			} else myDialog->DoKey(theEvent);
			break;
		case updateEvt:
			myDialog->DoUpdate(theEvent);
			break;
		case mouseDown:
			myDialog->fTheEvent = *theEvent;
			break;
		default:
			break;
	}
	SetPort(oldPort);
	return false;
}

#pragma segment Main
void CChooserDialog::DoAddButton(void)
{
	Str32	theItem;
	Cell	theCell;
	
	if (getValue(fTheDialog, kAddButton) == 0)
	if (GetActive() == kDeviceList)
	while (fDeviceList->IsSelection(theCell))		// get the selected device
	{
		fDeviceList->GetItem(theItem, theCell);
		if (fChosenList->InsertItem(theItem))		// insert it into the current list
		{
			fOriginalList->InsertItem(theItem);		// and in the one passed in.
		}
		fDeviceList->DeselectItem(theCell);
	}
	AdjustButtons();
}


#pragma segment Main
void CChooserDialog::DoRemoveButton(void)
{
	Str32	theItem;
	Cell	theCell;
	
	if (getValue(fTheDialog, kRemoveButton) == 0)
	if (GetActive() == kChosenList)
	{
		while (fChosenList->IsSelection(theCell))
		{
			fChosenList->GetItem(theItem, theCell);
			fOriginalList->RemoveItem(theItem);
			fChosenList->RemoveItem(theItem);
		}
		AdjustButtons();
	}
}



void EraseBigFrame(DialogPtr theDialog, short item)
{
	Rect box;
	GrafPtr oldPort;
	PenState theState;
	RGBColor theBackColor, theForeColor;

	GetPort(&oldPort);
	SetPort(theDialog);
	GetPenState(&theState);
	PenNormal();
	PenSize(2, 2);
	GetBackColor(&theBackColor);
	GetForeColor(&theForeColor);
	RGBForeColor(&theBackColor);
	getBox(theDialog, item, &box);
	InsetRect(&box, -4, -4);
	FrameRect(&box);
	SetPenState(&theState);
	RGBForeColor(&theForeColor);
	SetPort(oldPort);
}

void DrawSmallFrame(DialogPtr theDialog, short item)
{
	Rect box;
	GrafPtr oldPort;
	PenState theState;

	GetPort(&oldPort);
	SetPort(theDialog);
	GetPenState(&theState);
	PenNormal();
	getBox(theDialog, item, &box);
	InsetRect(&box, -1, -1);
	FrameRect(&box);
	SetPenState(&theState);
	SetPort(oldPort);
}

void DrawBigFrame(DialogPtr theDialog, short item)
{
	Rect box;
	GrafPtr oldPort;
	PenState theState;

	GetPort(&oldPort);
	SetPort(theDialog);
	GetPenState(&theState);
	PenNormal();
	PenSize(2, 2);
	getBox(theDialog, item, &box);
	InsetRect(&box, -4, -4);
	FrameRect(&box);
	SetPenState(&theState);
	SetPort(oldPort);
}

