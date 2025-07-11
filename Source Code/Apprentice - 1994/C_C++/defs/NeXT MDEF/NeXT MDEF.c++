
// NeXT Menu MDEF Emulator
// Version 1.0
// Jan. 17, 1994 (Right after the Northridge earthquake�whew!!)
// Last Update: May 4, 1994
// by Hiep Dam, 3G Software
// Contact: America Online -> StarLabs
//          Delphi         -> StarLabs
//          Internet       -> starlabs@aol.com, starlabs@delphi.com
//			Snail mail	   -> Hiep Dam
//							  2226 Parkside Ave #302
//							  Los Angeles, CA 90031

// ----------------------------------------------------------------------

// FREE! FREE! FREE!
// This code & MDEF are in the public domain. Use it at your own discretion.
// I am not liable for your own foibles.

// ----------------------------------------------------------------------

// Notes:
//
// 		1) This MDEF does not support meta-characters. It doesn't parse
//		   menu item text, and draws the menu items' text AS IS. If you want to
//		   add cmd-keys or sub-menus during *run-time*, use _SetItemCmd or the
//		   appropriate Menu Manager trap.
//
//		2) Nor does this menu MDEF support text styles or icons.
//		   It doesn't support "mctb" resources either (since the MDEF is
//		   already in color, this would defeat the MDEF's purpose). All this
//		   stuff, though, should be easy to add & implement...
//
//		3) This MDEF *does* support cmd-keys. Yeah!
//
//		4) This MDEF supports item marks, so checkmarks, etc are usable.
//
//		5) This MDEF makes several IMPORTANT *assumptions* about the environment
//		   it's running on. Running in an incorrect environment will result
//		   in messy crashes.
//			a) Color Quickdraw is present. It uses RGBColors to draw the menu,
//			   not the old-style 8-color Quickdraw model. There is no check
//			   to see if Color Quickdraw is available or not. If the MDEF is
//			   running in a b&w environment, there will be an "unimplemented
//			   trap" error. You can add b&w support yourself or add checks to
//			   prevent the MDEF from running if Color Quickdraw isn't available.
//			b) System 7 is present. The cmd-key clover symbol is drawn with
//			   Chicago (since that's the only font for sure that has this symbol)
//			   but the symbol is drawn at 9-point. If TrueType is present, as
//			   in the case of System 7, you'll get a funky but readable cmd-key
//			   clover. If not present, oh well...
//
//		6) The skeletal portions of this code were derived from an example MDEF
//		   included in THINK Reference. See "Custom Menus" in the Menu Manager
//		   section.


// ----------------------------------------------------------------------

#include "NeXT MDEF.h"

// ----------------------------------------------------------------------

typedef enum MDEFconstants {
	// These constants were used with Geneva 9; if you use any other font
	// with this MDEF, you might have to change some values to get your
	// menu to look nice...
	rectPadding  = 1,			// Padding between menu item & whole menu
	heightPad    = 6,			// Vertical padding between text & menu item (x2)
	widthPad     = 6,			// Horizontal padding between text & menu item (x2)
	textHtPad	 = 1,			// Vertical offset for "balanced" menu item text
	cmdKeyPad	 = 6,			// Horizontal padding added if has cmd key
	subMenuPad	 = 12,			// Horizontal padding added if has submenu
	subMenuHt    = 6,			// Height of submenu triangle
	subMenuWd    = 6,			// Width of submenu triangle
	hilite       = true,		// Hilite status of menu item (selected or not)
	unhilite     = false,

	menuFont     = geneva,
	menuSize     = 9,
	menuFace     = 0,
	sysTextMode  = srcOr,
	changeFont   = true,
	
	kBlack		 = 0,			// Some RGBColor constants
	kWhite		 = 65535,
	kLtGray		 = 56797,       // light (56797) or darker (48059)
	kDkGray		 = 17467		// 17467 (lighter) or darker (8738)
//	kLtGray		 = 52428,		// Older color constants
//	kDkGray		 = 8738
};

// MDEFColors struct.
// Used for keeping track of menu colors, from function to function,
// since we aren't allowed to have globals (though we can if we want
// to, using the A4 register, but that's too skanky for me).

typedef struct MDEFcolors {
	struct RGBColor black;
	struct RGBColor white;
	struct RGBColor ltGray;
	struct RGBColor dkGray;
};


// ----------------------------------------------------------------------

// Some prototypes
void FillInColor(RGBColor *theColor, unsigned short r, unsigned short g, unsigned short b);
void InitMDEFColors(MDEFcolors *theColors);
short GetMenuItemHeight();
void GetMenuItemRect(Rect *menuRect, Rect *itemRect, short whichItem);
Boolean IsItemDisabled(MenuHandle whichMenu, short whichItem);
short GetMenuWidth(MenuHandle whichMenu, short numItems);
void DoSizeMsg(MenuHandle whichMenu, Rect *menuRect);
void DoDrawMsg(MenuHandle whichMenu, Rect *menuRect);
void DoChooseMsg(MenuHandle whichMenu, Rect *menuRect, Point hitPt, short *itemID);
void DrawMenuItem(MenuHandle whichMenu, Rect *menuRect, short whichItem,
					MDEFconstants hiliteState, MDEFcolors *menuColors);

// ----------------------------------------------------------------------

// Main.
// This is the entrypoint for the MDEF. So what does that mean? Simply
// this is the function that will be called when the MDEF is used.
// It checks what is the current message sent to it, case goes
// thru a switch statement to find the correct handler for the msg.

pascal void main(short msg, MenuHandle whichMenu, Rect *menuRect, Point hitPt,
						short *itemID) {
	// Change the font from system font (Chicago) to something more interesting?
	if (changeFont) {
		TextFont(menuFont);
		TextSize(menuSize);
		TextFace(menuFace);
	}

	switch (msg) {
		case mDrawMsg: {
			DoDrawMsg(whichMenu, menuRect);
		} break;
	
		case mChooseMsg: {
			DoChooseMsg(whichMenu, menuRect, hitPt, itemID);
		} break;
	
		case mSizeMsg: {
			DoSizeMsg(whichMenu, menuRect);
		} break;
	
		case mPopUpMsg: {
			short itemCount = CountMItems(whichMenu);
			menuRect->top = hitPt.h - ((*itemID - 1) * GetMenuItemHeight());
			menuRect->left = hitPt.v;
			menuRect->right = menuRect->left + GetMenuWidth(whichMenu, itemCount);
			menuRect->bottom = menuRect->top + (itemCount * GetMenuItemHeight());
		} break;
		
		case mDrawItemMsg: {
			// Unused for the moment since I don't know what this msg is for.
		} break;
		
		case mCalcItemMsg: {
			// Ditto for this dude.
		} break;
	}	// end of switch

	// Polite manners: if we changed the font, restore the system font upon
	// exiting...
	if (changeFont) {
		TextFont(systemFont);
		TextSize(12);
		TextFace(0);
	}
} // END main

// ----------------------------------------------------------------------

// IsItemDisabled.
// Finds out whether the given menu item is disabled or not. Note though
// that we also have to check if the *entire* menu is disabled or not, in
// addition to the menu item. We do this by checking bit 0 of the
// enableFlags field in the menuInfo structure of a menu.

Boolean IsItemDisabled(MenuHandle whichMenu, short whichItem) {
	return(!BitTst(&(*whichMenu)->enableFlags, 31 - whichItem) ||
		   !BitTst(&(*whichMenu)->enableFlags, 31 - 0));
} // END IsItemDisabled

// ----------------------------------------------------------------------

// GetMenuItemHeight.
// Get the height of any single menu item. If you wish to add icons
// to your menu, you'll have to change this to accomodate larger icons.
// This simply calls _GetFontInfo.

short GetMenuItemHeight() {
	FontInfo theInfo;
	GetFontInfo(&theInfo);
	return(theInfo.ascent + (heightPad * 2));
} // END GetMenuItemHeight

// ----------------------------------------------------------------------

// GetMenuHeight.
// Gets the height of a single menu item, and finds the height
// of the whole menu by multiplying a single menu item height
// by the number of menu items.

short GetMenuHeight(short numItems) {
	return(numItems * GetMenuItemHeight());
} // END GetMenuHeight

// ----------------------------------------------------------------------

// GetMenuWidth.
// Gets the width of the entire menu, by finding the width of the
// widest menu item in the menu. Polls each menu item for its width,
// keeping track of the largest width.
// Accounts for submenu "triangles" and cmd-keys in menu item width.

short GetMenuWidth(MenuHandle whichMenu, short numItems) {
	Str255 itemText;
	short maxLength = 0;
	short curLength = 0;
	short theChar;
	short cmdWidth;
	short checkWidth;
	
	checkWidth = StringWidth("\p�") + 4 + 4;	// 4 = left & right padding
	cmdWidth = StringWidth("\pW");			// "W" is widest character
	cmdWidth += CharWidth(17);				// "Cmd" key character

	for (short i = 1; i <= numItems; i++) {
		GetItem(whichMenu, i, itemText);
		curLength = StringWidth(itemText);

		GetItemCmd(whichMenu, i, &theChar);
		if (theChar == hMenuCmd)
			curLength += subMenuPad;
		else if (theChar != 0)
			curLength += (cmdWidth + cmdKeyPad);

		//GetItemMark(whichMenu, i, &theChar);
		//if (theChar != noMark)
		//	checkWidth = StringWidth("\p� ");
		curLength += checkWidth;

		if (curLength > maxLength)
			maxLength = curLength;
	}

	return(maxLength + (widthPad * 2));
} // END GetMenuWidth

// ----------------------------------------------------------------------

// GetMenuItemRect.
// Obvious.

void GetMenuItemRect(Rect *menuRect, Rect *itemRect, short whichItem) {
	short oneHeight = GetMenuItemHeight();

	itemRect->left   = menuRect->left;
	itemRect->right  = menuRect->right;
	itemRect->top    = menuRect->top + (oneHeight * (whichItem - 1));
	itemRect->bottom = itemRect->top + oneHeight;
} // END GetMenuItemRect

// ----------------------------------------------------------------------

// DoSizeMsg.
// Even more obvious (m�s o menos)

void DoSizeMsg(MenuHandle whichMenu, Rect *menuRect) {
	short itemCount = CountMItems(whichMenu);

	(*whichMenu)->menuWidth  = GetMenuWidth(whichMenu, itemCount);
	(*whichMenu)->menuHeight = GetMenuHeight(itemCount);
} // END DoSizeMsg

// ----------------------------------------------------------------------

// DoDrawMsg.
// Erases the entire menu, and calls each menu item individually to
// draw itself.

void DoDrawMsg(MenuHandle whichMenu, Rect *menuRect) {
	short itemCount = CountMItems(whichMenu);

	MDEFcolors menuColors;
	InitMDEFColors(&menuColors);

	RGBBackColor(&menuColors.ltGray);
	EraseRect(menuRect);

	for (short i = 1; i <= itemCount; i++) {
		DrawMenuItem(whichMenu, menuRect, i, unhilite, &menuColors);
	}

	RGBForeColor(&menuColors.black);
	RGBBackColor(&menuColors.white);
} // END DoDrawMsg

// ----------------------------------------------------------------------

// DrawMenuItem.
// The "meat" of this MDEF. Does the actual drawing.
// Draws the menu item's 3D frame, item text, cmd-keys, and submenu symbols.
// It does this and also accounts for menu item's enabled/disabled status,
// among other things. Pretty tedious.
// A quick hack but the drawing speed was fine on my IIsi. The drawing
// could be sped up a bit, I suppose, but since I can't see any noticeable
// delay as is, I won't optimize the drawing any. This shouldn't stop you,
// though!!   :)

#include "3DPushIn.c++"

// ----------------------------------------------------------------------

// FindMenuItem.
// Given a point, find if this point is within the rect of any menu
// item. If so, return the item, else 0.

short FindMenuItem(MenuHandle whichMenu, Rect *menuRect, Point hitPt) {
	Rect itemRect;
	short itemHit = 0;
	short itemCount = CountMItems(whichMenu);
	
	for (short i = 1; i <= itemCount; i++) {
		GetMenuItemRect(menuRect, &itemRect, i);
		if (PtInRect(hitPt, &itemRect)) {
			itemHit = i;
			break;
		}
	}
	return(itemHit);
} // END FindMenuItem

// ----------------------------------------------------------------------

// DoChooseMsg.
// This is the only other function which calls DrawMenuItem, other than
// DoDrawMsg. Takes care of the hiliting/unhiliting of menu items.

void DoChooseMsg(MenuHandle whichMenu, Rect *menuRect, Point hitPt, short *itemID) {
	Str255 itemText;
	MDEFcolors menuColors;
	InitMDEFColors(&menuColors);
	short mouseItem = FindMenuItem(whichMenu, menuRect, hitPt);

	if (mouseItem == 0) {			// out of bounds or disabled
		DrawMenuItem(whichMenu, menuRect, *itemID, unhilite, &menuColors);
		*itemID = 0;				// return "cancel" code
	}
	else if (mouseItem != *itemID) {
		// Ok, user chose new menu item. Check to see if it's a valid menu
		// item (i.e. not disabled or a divider)
		GetItem(whichMenu, mouseItem, itemText);
		if ((itemText[1] == '-') || IsItemDisabled(whichMenu, mouseItem)) {
			DrawMenuItem(whichMenu, menuRect, *itemID, unhilite, &menuColors);
			*itemID = 0;
		}
		else {
			DrawMenuItem(whichMenu, menuRect, *itemID, unhilite, &menuColors);	// unhilight previous
			DrawMenuItem(whichMenu, menuRect, mouseItem, hilite, &menuColors);	// hilight new
			*itemID = mouseItem; 		// return new
		}
	}
} // END DoChooseMsg

// ----------------------------------------------------------------------

// FillInColor.
// If you use RGBColors, no doubt you'll be using some function similar to this.
// Now why didn't Apple include a function like this? Less overhead, I guess.

void FillInColor(RGBColor *theColor, unsigned short r, unsigned short g, unsigned short b) {
	theColor->red   = r;
	theColor->green = g;
	theColor->blue  = b;
} // END FillInColor

// ----------------------------------------------------------------------

void InitMDEFColors(MDEFcolors *theColors) {
	FillInColor(&theColors->white, kWhite, kWhite, kWhite);
	FillInColor(&theColors->black, kBlack, kBlack, kBlack);
	FillInColor(&theColors->ltGray, kLtGray, kLtGray, kLtGray);
	FillInColor(&theColors->dkGray, kDkGray, kDkGray, kDkGray);
} // END InitMDEFColors

// END NeXT MDEF.c++