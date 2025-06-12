/*----------------------------------------------------------------------------

	menus.c

	This module handles adjusting menus.
	
	Portions copyright © 1990, Apple Computer.
	Portions copyright © 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include "glob.h"
#include "menus.h"
#include "util.h"



/*----------------------------------------------------------------------------
	AdjustOneMenu
	
	Adjusts the state of a single menu.
	
	Entry:	menuID = menu id of menu to adjust.
			numCommands = number of commands in menu.
			*oldStatePtr = old menu enable/disable flags.
			newState = new menu enable/disable flags.
			
	Exit:	function result = true if menu bar must be redrawn.
			*oldStatePtr = updated menu enable/disable flags.
----------------------------------------------------------------------------*/

static Boolean AdjustOneMenu (short menuID, short numCommands,
	unsigned long *oldStatePtr, unsigned long newState)
{
	unsigned long x, y, oldState;
	Boolean mustRedraw = false;
	short i;
	MenuHandle theMenu;

	theMenu = GetMHandle(menuID);
	oldState = *oldStatePtr;
	if (oldState == newState) return false;
	if ((newState & 1) == 0) {
		if ((oldState & 1) == 0) return false;
		DisableItem(theMenu, 0);
		*oldStatePtr &= 0xfffffffe;
		return true;
	}
	if ((oldState & 1) == 0) {
		EnableItem(theMenu,0);
		mustRedraw = true;
	}
	x = oldState;
	y = newState;
	for (i = 1 ; i <= numCommands; i++) {
		x >>= 1;
		y >>= 1;
		if ((x & 1) != (y & 1)) {
			if ((y & 1) == 0) {
				DisableItem(theMenu, i);
			} else {
				EnableItem(theMenu, i);
			}
		}
	}
	*oldStatePtr = newState;
	return mustRedraw;
}



/*----------------------------------------------------------------------------
	SetMenusTo
	
	Sets the menus to a given state.
	
	Entry:	newAppleMenuEnabled = true if Apple menu should be enabled, 
				false if disabled.
			newFileMenuState = new File menu enable/disable flags.
			newEditMenuState = new Edit menu enable/disable flags.
			newNewsMenuState = new News menu enable/disable flags.
			newSearchMenuState = new Search menu enable/disable flags.
			newSpecialMenuState = new Special menu enable/disable flags.
			newWindMenuState = new Windows menu enable/disable flags.
----------------------------------------------------------------------------*/

void SetMenusTo (
	Boolean newAppleMenuEnabled,
	unsigned long newFileMenuState, 
	unsigned long newEditMenuState,
	unsigned long newNewsMenuState, 
	unsigned long newSearchMenuState,
	unsigned long newSpecialMenuState, 
	unsigned long newWindMenuState)
{
	Boolean r0,r1,r2,r3,r4,r5,r6;
	static Boolean appleMenuEnabled = false;
	static unsigned long fileMenuState = 0;
	static unsigned long editMenuState = 0;
	static unsigned long newsMenuState = 0;
	static unsigned long searchMenuState = 0;
	static unsigned long specialMenuState = 0;
	static unsigned long windMenuState = 0;
	
	r0 = newAppleMenuEnabled != appleMenuEnabled;
	if (r0) {
		if (newAppleMenuEnabled) {
			EnableItem(GetMHandle(kAppleMenu), 0);
		} else {
			DisableItem(GetMHandle(kAppleMenu), 0);
		}
		appleMenuEnabled = newAppleMenuEnabled;
	}
	r1 = AdjustOneMenu(kFileMenu, kNumFileMenuItems, &fileMenuState, newFileMenuState);
	r2 = AdjustOneMenu(kEditMenu, kNumEditMenuItems, &editMenuState, newEditMenuState);
	r3 = AdjustOneMenu(kNewsMenu, kNumNewsMenuItems, &newsMenuState, newNewsMenuState);
	r4 = AdjustOneMenu(kSearchMenu, kNumSearchMenuItems, &searchMenuState, newSearchMenuState);
	r5 = AdjustOneMenu(kSpecialMenu, kNumSpecialMenuItems, &specialMenuState, newSpecialMenuState);
	r6 = AdjustOneMenu(kWindMenu, kNumWindMenuItems, &windMenuState, newWindMenuState);
	if (r0 || r1 || r2 || r3 || r4 || r5 || r6) DrawMenuBar();
}
	


/*----------------------------------------------------------------------------
	AdjustMenus
	
	Enables and disables menus and commands based on which kind of window
	is active.
----------------------------------------------------------------------------*/

void AdjustMenus (void)
{
	WindowPtr wind;
	TWindow **info;
	EWindowKind kind;
	
	if (gInBackground) return;
	wind = FrontWindow();
	if (IsDAWindow(wind)) {
		SetMenusTo(true, kDAFileEnabled, kDAEditEnabled, 
			kDANewsEnabled, kDASearchEnabled, 
			kDASpecialEnabled,kDAWindEnabled);
	} else if (!gStartupOK) {
		SetMenusTo(true, kStartupBadFileEnabled, kStartupBadEditEnabled,
			kStartupBadNewsEnabled, kStartupBadSearchEnabled, 
			kStartupBadSpecialEnabled, kStartupBadWindEnabled);
	} else if (wind == nil) {
		SetMenusTo(true, kNoneFileEnabled, kNoneEditEnabled, 
			kNoneNewsEnabled, kNoneSearchEnabled, 
			kNoneSpecialEnabled, kNoneWindEnabled);
	} else {
		info = (TWindow**)GetWRefCon(wind);
		kind = (**info).kind;
		switch (kind) {
			case kFullGroup:
				SetMenusTo(true, kGroupFileEnabled, kGroupEditEnabled, 
					kGroupNewsEnabled, kGroupSearchEnabled, 
					kGroupSpecialEnabled, kGroupWindEnabled);
				break;
			case kNewGroup:
				SetMenusTo(true, kNewGroupFileEnabled, kNewGroupEditEnabled, 
					kNewGroupNewsEnabled, kNewGroupSearchEnabled, 
					kNewGroupSpecialEnabled, kNewGroupWindEnabled);
				break;
			case kUserGroup:
				SetMenusTo(true, kUserGroupFileEnabled, kUserGroupEditEnabled, 
					kUserGroupNewsEnabled, kUserGroupSearchEnabled, 
					kUserGroupSpecialEnabled, kUserGroupWindEnabled);
				break;
			case kSubject:
				SetMenusTo(true, kSubjectFileEnabled, kSubjectEditEnabled, 
					kSubjectNewsEnabled, kSubjectSearchEnabled, 
					kSubjectSpecialEnabled, kSubjectWindEnabled);
				break;
			case kArticle:
				SetMenusTo(true, kArticleFileEnabled, kArticleEditEnabled, 
					kArticleNewsEnabled, kArticleSearchEnabled, 
					kArticleSpecialEnabled, kArticleWindEnabled);
				break;
			case kMiscArticle:
				SetMenusTo(true, kMiscArticleFileEnabled, kMiscArticleEditEnabled, 
					kMiscArticleNewsEnabled, kMiscArticleSearchEnabled, 
					kMiscArticleSpecialEnabled, kMiscArticleWindEnabled);
				break;
			case kMailMessage:
				SetMenusTo(true, kMailMessageFileEnabled, kMailMessageEditEnabled, 
					kMailMessageNewsEnabled, kMailMessageSearchEnabled, 
					kMailMessageSpecialEnabled, kMailMessageWindEnabled);
				break;
			case kPostMessage:
				SetMenusTo(true, kPostMessageFileEnabled, kPostMessageEditEnabled, 
					kPostMessageNewsEnabled, kPostMessageSearchEnabled, 
					kPostMessageSpecialEnabled, kPostMessageWindEnabled);
				break;
		}
		if (kind == kArticle || kind == kMiscArticle || kind == kPostMessage ||
			kind == kMailMessage)
		{
			SetItem(GetMHandle(kEditMenu),kShowHideHeaderItem,
				(**info).headerShown ? "\pHide Header" : "\pShow Header");
		}
	}
}
