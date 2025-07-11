/*
	File:			VoicePickerDemo.h

	Contains:		Header for a sample application that uses the Voice Picker library. Look at 'DoShowVP'
				function to see how to display the Voice Picker. Also note that the library requires
				the following files to be in the project:
				� VoicePickerLib.68 || VoicePickerLib.PPC
				� VoicePicker.rsrc
				� VoicePicker Balloons.rsrc (if you want balloon help)
				In alternative you can include the Voice Picker source file:
				� VoicePicker.c
				� VoicePicker.rsrc
				� VoicePicker Balloons.rsrc (if you want balloon help)
				
	Written by:	Luigi Belverato
				P.O. Box 19,
				20080 Basiglio MI,
				Italy

	Copyright:	�1995 Luigi Belverato
	
	Change History (most recent first):
				06/28/95	0.0.0d1	Added DisposeRoutineDescriptor for Apple Events handlers

				06/23/95	0.0.0d0

	Notes: 		This code uses Apple's Universal Interfaces for C version 2.01f.
	
				Send bug reports to lbelvera@micronet.it
				(if I don't reply within a few days use snail mail as I might have changed internet provider)
*/

//-----------------------------------------------------------------------
// this text document was formatted with Geneva 9, tab size 4

#include "VoicePicker.h"

//Menu resource IDs

#define kmenuBar		128

#define mApple			128
#define iAbout			1

#define mFile			129
#define iClose			1
#define iQuit			3

#define mEdit			130
#define iUndo			1
#define iCut			3
#define iCopy			4
#define iPaste			5
#define iClear			6
#define iSelectAll		8

#define mTest			131
#define iShowVP		1

//Alerts

#define kAlrtOK			128
#define kAlrtAbout			129
#define kAlrtMissingFeatures	130

//Ind String Resources
#define kErrorStr		128
#define kNoMBAR		1

//Various

#define kSleep			20L

//Structures

typedef struct FeaturesRec	FeaturesRec;
struct FeaturesRec
{
	Boolean	appleEvents;
};

//Prototypes

void ToolboxInit(void);
void MenuBarInit(void);
void MainEventLoop(void);
void EventGetter(void);
void HandleEvent(EventRecord *event);
void HandleActivate(EventRecord *event);
void HandleDiskInsert(EventRecord *event);
void HandleKeyPress(EventRecord *event);
void HandleMouseDown(EventRecord *event);
void HandleOSEvent(EventRecord *event);
void HandleUpdate(EventRecord *event);
void AdjustMenus(void);
void CloseAnyWindow(WindowRef window);
Boolean IsDAWindow(WindowRef window);
Boolean IsCompatible(void);
void GatherFeatures(void);
Boolean TrapAvailable(short theTrap);
TrapType GetTrapType(short theTrap);
short NumToolboxTraps(void);
void MissingFeaturesError(void);
void DoErrorIndex(short stringIndex,Str255 theString);
void	HandleHighLevelEvent(EventRecord *theEvent);
void InstallAEEntries(void);
pascal OSErr HandleOAPP(AppleEvent *theAppleEvent, AppleEvent *reply,long handlerRefcon);
pascal OSErr HandleQUIT(AppleEvent *theAppleEvent, AppleEvent *reply,long handlerRefcon);
OSErr CheckGotRequiredParams (AppleEvent *theAppleEvent);
Boolean IsInBackground(void);

void HandleMenuCommand(long menuResult);
void HandleAppleChoice(short menuItem);
void HandleFileChoice(short menuItem);
void HandleEditChoice(short menuItem);
void HandleTestChoice(short menuItem);

//Apple
void DoAbout(void);

//File
void DoQuit(void);

//Test
void DoShowVP(void);
