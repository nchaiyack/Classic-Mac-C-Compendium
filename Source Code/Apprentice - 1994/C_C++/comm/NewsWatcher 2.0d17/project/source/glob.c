/*----------------------------------------------------------------------------

	glob.c

	This file contains declarations for all the global variables.
	
	Portions copyright � 1990, Apple Computer.
	Portions copyright � 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include "glob.h"
#include "menus.h"

short		gNumGroups = 0;				/* number of newsgroups */
TGroup		**gGroupArray = nil;		/* handle to full group array */
Handle		gGroupNames = nil;			/* handle to group name strings */
WindowPtr	gFullGroupWindow = nil;		/* pointer to full group list window */
Boolean		gMustDoZoomOnShowFullGroupList;
										/* true if full group list needs zooming */

TPrefRec 	gPrefs;						/* preferences */

Boolean		gDone = false;				/* flag set true upon program termination */
Boolean 	gCancel = false;			/* flag set when user cancels an action */

Handle		gLifeBoat;					/* lifeboat memory -- de-allocated when 
										   memory gets low */
Boolean		gSinking = true;			/* flag set after lifeboat has been 
										   jettisoned */
Boolean 	gOutOfMemory = false;		/* flag set when out of memory - and luck */

Boolean		gInBackground = false;		/* background/foreground flag */
Rect		gDesktopExtent;				/* desktop extent */

Cursor		gIBeamCurs;					/* ibeam cursor */
Boolean		gHasColorQD;				/* true if we have color QuickDraw */
Rect		gWindLimits;				/* window grow limits */

CStr255		gAutoFetchHost;				/* autofetch host */
CStr255		gAutoFetchName;				/* autofetch username */
char		gAutoFetchPass[32];			/* autofetch password */
char		gAutoFetchPath[32];			/* autofetch path */

Rect		gDragRect;					/* drag limit rectangle */

Boolean		gStartingUp = true;			/* true during initialization */
Boolean		gStartupOK;					/* true if no errors during startup */

Boolean		gFullGroupListDirty = false;	/* true if full group list dirty */

FSSpec 		gPrefsFile;					/* preferences file location */
FSSpec 		gFilterFile;				/* filter file location */
