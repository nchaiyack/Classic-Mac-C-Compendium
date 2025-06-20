#ifndef __CHOOSERDIALOG__
#define __CHOOSERDIALOG__

#pragma once

#ifndef __APPLETALK__
#include <AppleTalk.h>
#endif

#ifndef __DIALOGS__
#include <Dialogs.h>
#endif

#ifndef __CLISTMANAGER__
#include "CListManager.h"
#endif

#define		kATPTimeOutVal			3							// re-try ATP SendRequest every 3 seconds 
#define		kATPRetryCount			5							// for five times 
#define		kZonesSize				578							// size of buffer for zone names 
#define		kGetMyZoneCall			0x07000000					// GetMyZone indicator 
#define		kGetZoneListCall		0x08000000					// GetZoneList indicator 
#define		kZIPSocket				6							// the Zone Information Protocol socket 
#define		kMoreZones				0xFF000000 					// mask to see if more zones to come 
#define		kZoneCount				0x0000FFFF 					// mask to count zones in buffer 

#define		kTupleSize	104
#define		kMaxTuples	1

enum
{
	kAddButton = 1,
	kDoneButton,
	kZoneList,
	kDeviceList,
	kRemoveButton,
	kZoneText,
	kDeviceText,
	kChosenList,
	kDeviceTypeText,
	kChosenText
};

#ifdef __cplusplus
extern "C"
{
#endif
void EraseBigFrame(DialogPtr theDialog, short item);
void DrawSmallFrame(DialogPtr theDialog, short item);
void DrawBigFrame(DialogPtr theDialog, short item);
#ifdef __cplusplus
}
#endif

class CChooserDialog : public SingleObject
{
	public:
	
				CChooserDialog();
				~CChooserDialog();
		void	Run();
		void	Initialize(short resID);
		void	SetChosenList(CListManager *fOriginalList);

	protected:
		
		short	GetActive(void) {return fActiveItem;}
		short	DoTabCycle(short theDirection);
		void	DoUpdate(EventRecord *theEvent);
		void	DoNull(void);
		void	DoKey(EventRecord *theEvent);
		void	AdjustButtons(void);
		void DoZoneHit();
		void DoChosenHit();
		void DoDeviceHit();
		void	DoTyping(EventRecord *theEvent, char key);
		void	InitZoneList();
		void	ResetLastKey(long when);
		OSErr	DoBuildZoneList(void);
		OSErr	HiliteUserZone(void);
		OSErr	OldStyleGetMyZone(StringPtr str);
		void	ChangeActive(short theItem);
		void	InitDeviceList();

		static pascal Boolean EventFilterProc(DialogPtr theDialog, EventRecord *theEvent, short *itemHit);
	void DoAddButton(void);

	void DoRemoveButton(void);



		Ptr				fLookupBuffer;
		char			fBuffer[36];
		long			fLastCharTime;	// time of last keydown
		Str32			fCharString;	// string that holds characters that were typed 4 sel
		short			fActiveItem;
		short			fNumGotten;
		DialogPtr		fTheDialog;
		EventRecord		fTheEvent;
		CListManager	*fZoneList, *fDeviceList, *fChosenList, *fOriginalList;
		MPPParamBlock	fLookupParam;
};


#endif
