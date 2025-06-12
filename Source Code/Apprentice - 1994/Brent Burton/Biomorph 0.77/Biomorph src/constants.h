#ifndef CONSTANTS_H
#define CONSTANTS_H

//
// biomorph.h
//
// This file contains all the "k" constant macro definitions for
// resource types, dialog item #'s, etc.  Anything with a constant
// is defined here.
//


// ---------------  Constant Macros

#define kHighLevelEvent 23   // from Inside Mac VI, p. 5-21.

#define kEventCount	15	// number of pixels to redraw before checking for
						// pending events again.
#define kIamSleepy	15	// number of ticks to sleep when we are in bkgnd

// Window and Dialog IDs

#define kMainWIND	128
#define kSelectWIND	129
#define kControlDLOG	128
#define kAboutDLOG	129
#define kInfoDLOG	130

// Error handling rez ID's

#define kErrAlertID	228		// alert template ID
#define kErrStrList	128		// Rez ID of the STR# for error msgs
#define kBitmapAllocErr	1	// couldn't get the bitmap allocated
#define kMorphCodeErr	2	// couldn't get the code 'morf' resource
#define kFetchMenuErr	3	// couldn't get the MBAR loaded
#define kFetchWINDErr	4	// couldn't get the WINDs loaded
#define kSavePrompt		5	// "save file as..." expression
#define kUntitledName	6	// simply "Untitled"
#define kFileSaveErr	7	// "error saving ..."
#define kFileOpenErr	8	// "error opening..."

// Control dialog item numbers...

#define kZoomGroup	11  // "group" == draw a box around it
#define kRangeGroup	12
#define kZoomLabel	13
#define kRangeLabel	14

#define kZoomInB	2  // zoom in button
#define kZoomOutB	1  // zoom out button

#define kSTXMin		15  // static text items...
#define kSTXMax		16
#define kSTYMin		17
#define kSTYMax		18
#define kSTCreal	19
#define kSTCimag	20

#define kETXMin		5 // editable text items...
#define	kETXMax		6
#define kETYMin		7
#define kETYMax		8
#define kETCreal	9
#define kETCimag	10

#define kRedrawB	3 // redraw button
#define kSelectB	4 // get current image selection button


// Menu ID's

#define kMenuBar	128

#define kAppleM		128
#define kFileM		129
#define kEditM		130
#define kTypeM		131  // type of biomorph to calculate

// Menu item numbers

#define kAppleAboutItem	1

#define kFileNewItem	1
#define kFileOpenItem	2		// items 3 and 5 are disabled separators
#define kFileSaveAsItem	4
#define kFileQuitItem	6

#define kEditUndoItem	1
#define kEditCutItem	3
#define kEditCopyItem	4
#define kEditPasteItem	5
#define kEditClearItem	6

#define kTypeAddItem	1
#define kTypeDeleteItem	2
#define kTypeDefaultItem 4

// Miscellaneous constants

#define kMorphECR		'morf'  // external code resource type
#define kPictFileType	'PICT'
#define kFileCreator	'MORF'
#define kAboutOKButt	1		// About OK "go away" button
#define kAboutInfoButt	2		// show more info button
#define kAboutOKOutline	6		// user item to draw outline around button
#define kInfoOKButt		1		// Info... window OK button
#define kInfoOKOutline	2		// Info... default button outline


#endif
