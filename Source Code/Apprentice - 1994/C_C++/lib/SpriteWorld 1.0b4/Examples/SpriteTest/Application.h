///--------------------------------------------------------------------------------------
//	Application.h
//
//	Created:	Sunday, April 11, 1993
//	By:		Tony Myles
//
//	Copyright: © 1993-94 Tony Myles, All rights reserved worldwide.
///--------------------------------------------------------------------------------------


#ifndef __APPLICATION__
#define __APPLICATION__

#ifndef __APPLEEVENTS__
#include <AppleEvents.h>
#endif


enum
{
	kStackNeeded = 32000,
	kNumberOfMoreMastersCalls = 3
};

enum
{
	kMenuBarResID = 128,
	kWindowResID = 128
};

enum
{
	kAppleMenuID = 128,
	kFileMenuID = 129,
	kEditMenuID = 130,
	kSpriteMenuID = 131,
	kTestMenuID = 132
};

enum
{
		// file menu
	kAboutItem = 1,
	kQuitItem = 10,

		// edit menu
	kUndoItem = 1,
	kCutItem = 3,
	kCopyItem,
	kPasteItem,
	kClearItem,
	kSelectAllItem,

		// sprite menu
	kCreateSpriteItem = 1,
	kCompileSpriteItem,
	kHideTitleItem = 4,
	kHideBallsItem,

		// test menu
	kTestItem = 1,
	kCopyBitsTestItem = 3,
	kBlitPixieTestItem,
	kRunCompiledSpriteItem,
	kCollisionDetectionItem = 7
};

enum
{
	kForeGroundSleepTime = 0,
	kBackGroundSleepTime = 0
};

enum
{
	kErrorAlertResID = 128,
	kCantRunAlertResID = 129,
	kErrorStringListResID = 128,
	kUnknownErrorStringIndex = 1,
	kCantFindResourceStringIndex = 2,
	kNotEnoughMemoryStringIndex = 3
};

#define kSeriousDamageString "\pCould not even get error string!\rThis application is seriously damaged!"


#ifdef __cplusplus
extern "C" {
#endif

void main(void);
Boolean Initialize(short numberOfMasters);
Boolean CheckSystem(void);
Boolean CheckMemory(void);
Boolean HasAppleEvents(void);
Boolean HasWaitNextEvent(void);
OSErr InstallAppleEventHandlers(void);
void CreateMenuBar(void);
void CreateWindow(void);
Boolean EnterApplication(void);
void ExitApplication(void);
void ServiceEvents(void);
void DispatchEvent(EventRecord* event);
void HandleMouseEvent(EventRecord* event);
void HandleKeyEvent(char key, short modifiers);
void HandleUpdateEvent(WindowPtr updateWindowP);
void HandleActivateEvent(WindowPtr updateWindowP);
void HandleOSEvent(long message);
void HandleDiskEvent(long message);
void HandleNullEvent(void);
void HandleMenuCommand(long menuItemIdentifier);
void HandleAppleMenuCommand(short menuItem);
void HandleFileMenuCommand(short menuItem);
void HandleEditMenuCommand(short menuItem);
void HandleSpriteMenuCommand(short menuItem);
void HandleTestMenuCommand(short menuItem);
void AdjustMenuItems(void);
pascal OSErr HandleOpenApp(AppleEvent srcAppleEvent, AppleEvent replyAppleEvent, long refCon);
pascal OSErr HandleOpenDoc(AppleEvent srcAppleEvent, AppleEvent replyAppleEvent, long refCon);
pascal OSErr HandlePrintDoc(AppleEvent srcAppleEvent, AppleEvent replyAppleEvent, long refCon);
pascal OSErr HandleQuit(AppleEvent srcAppleEvent, AppleEvent replyAppleEvent, long refCon);
void ErrorAlert(OSErr err, short errorStringIndex);
void CantFindResource(void);
void CantRunOnThisMachine(void);
short NumToolboxTraps(void);
TrapType GetTrapType(short trap);
Boolean TrapAvail(short trap);

#ifdef __cplusplus
};
#endif


#endif /* __APPLICATION__ */