/*************************************************************************************************

SGF.h -- Copyright Chris Larson, 1993 -- All rights reserved.
         Based partly upon StandardGetFolder example by Steve Falkenburg (MacDTS)
         and partly on the code in Inside Macintosh: Files.
                       
         Dialog box layouts taken from Inside Macintosh: Files.
         
 This file contains the declarations necessary to compile the StandardGetFolder.c file.
 It should only be #included by StandardGetFolder.c. To use the StandardGetFolder call,
 #include the StandardGetFolder.h file in your source.

	Bug Reports/Comments to cklarson@engr.ucdavis.edu

	Version 1.0

*************************************************************************************************/

#ifndef __CKLSGF__
#define __CKLSGF__

// ----------
// Include Files
// ----------

#include <Aliases.h>
#include <Finder.h>
#include <Folders.h>
#include <GestaltEqu.h>
#include <Packages.h>
#include "PStrings.h"			// My set of pascal string routines
#include <Traps.h>

// ----------
// Constant Declarations
// ----------

#define rGetFolderDialogID			16042	// Res ID of the StandardGetFolder dialog.

#define rLeftStringID				16000	// Res ID of the button title wrapper string, left side.
#define rRightStringID				16001	// Res ID of the button title wrapper string, right side.
#define rDesktopNameStringID		16002	// Res ID of the desktop folder name substitute string.
#define rSelectKeyStringID			16003	// Res ID of the string holding the select key character.

#define sfItemPromptStatText		10		// Item number of the prompt.
#define sfItemSelectFolderButton	11		// Item number of the select button.

#define kIsFolderFlag				0x10	// flag indicating the file is a directory.

#define kIsAliasFlag				0x8000	// flag indicating that the file is an alias.
#define kInvisibleFlag				0x4000	// flag indicating that the file is invisible.

#define kNullVol					0x8000	// Null value for volume reference numbers (TN 77).

#define kAllFiles					-1		// Constant specifying that all file types be filtered.

#define kInFront					((WindowPtr)-1) // Specifies that this window appear in front.

#define kHitButtonDelay				8L		// The number of ticks to fake a button press.

#define kTitleOffset				8		// Padding length for the button title.

#define kEnterKey					0x03	// Character code of the enter key.
#define kReturnKey					0x0D	// Character code of the return key.

#define kInverted					1		// highlight value of a pressed button.
#define kNormal						0		// Highlight value of an unpressed button.
#define kInactive					255		// Constant indicating to dim a control.

#define kExistingPopUpStrings		-6045	// Resource ID of strings to replace.
#define kNewPopUpStrings			6045	// Resource ID of replacement strings.

// ----------
// Variable Types
// ----------

typedef void (*WindowFunc)(EventRecord*);	// Declaration of pointer to application window function.

// ----------
// Function Prototypes
// ----------

pascal Boolean GetFolderFileFilterYD (CInfoPBPtr paramBlock, StandardFileReply *myData);
pascal short GetFolderDialogHookYD (short itemHit, DialogPtr theDialog, StandardFileReply *myData);
pascal Boolean GetFolderModalFilterYD (DialogPtr theDialog, EventRecord *theEvent, short *itemHit,
										StandardFileReply *myData);

void CallAppWindowFunction (EventRecord *theEvent);
void SetButtonTitle (ControlHandle theButton, Str255 newTitle, Rect *buttonRect, DialogPtr theDialog);
void HitButton (DialogPtr theDialog, short itemNumber);

pascal Handle Patch (ResType type, short id);

#endif
