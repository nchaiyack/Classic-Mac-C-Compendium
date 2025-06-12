/*----------------------------------------------------------------------------

	about.c

	This module presents the about box.
	
	Portions copyright © 1990, Apple Computer.
	Portions copyright © 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <string.h>

#include "glob.h"
#include "about.h"
#include "dlgutil.h"
#include "util.h"
#include "prefs.h"



#define	kAboutID			128		/* About box dialog */
#define kAboutText 			128		/* resource id of about box text 'TEXT' resource */

#define kBigIconPictItem	2		/* item number of big icon pict */
#define kLogoPictItem		3		/* item number of logo pict */
#define kAboutBoxUserItem 	5		/* item number of user item for text */

#define kBigIconPictColor	128		/* resource id of big icon color picture */
#define kBigIconPictBW		129		/* resource id of big icon B&W picture */
#define kLogoPictColor		130 	/* resource id of logo color picture */
#define kLogoPictBW			131 	/* resource id of logo B&W picture */



static short gMagicCookie;
	


/*----------------------------------------------------------------------------
	AboutBoxFilter 
	
	About box dialog filter.
----------------------------------------------------------------------------*/

static pascal Boolean AboutBoxFilter (DialogPtr dlg, EventRecord *theEvent, short *itemHit)
{
	char key;

	if (theEvent->what == keyDown && (theEvent->modifiers & cmdKey) != 0) {
		key = theEvent->message & charCodeMask;
		if (key == '1' || key == '2') {
			gMagicCookie = key - '0';
			*itemHit = ok;
			return true;
		}
	}
	return DialogFilter(dlg, theEvent, itemHit);
}



/*----------------------------------------------------------------------------
	DoAboutBox
	
	Present the about box.
----------------------------------------------------------------------------*/

void DoAboutBox (void)
{
	DialogPtr dlg;
	short item;
	PicHandle bigIconPict, logoPict;
	Handle vers1Resource;
	short bigIconPictID, logoPictID;
	CStr255 sampleNewsServerErrorMessage;
	Handle text;
	short fontNum;
	
	dlg = MyGetNewDialog(kAboutID);
	if (GetPixelDepth(&dlg->portRect) <= 2) {
		bigIconPictID = kBigIconPictBW;
		logoPictID = kLogoPictBW;
	} else {
		bigIconPictID = kBigIconPictColor;
		logoPictID = kLogoPictColor;
	}
	bigIconPict = GetPicture(bigIconPictID);
	HNoPurge((Handle)bigIconPict);
	logoPict = GetPicture(logoPictID);
	HNoPurge((Handle)logoPict);
	DlgSetPict(dlg, kBigIconPictItem, bigIconPict);
	DlgSetPict(dlg, kLogoPictItem, logoPict);
	vers1Resource = GetResource('vers', 1);
	HLock(vers1Resource);
	ParamText((StringPtr)*vers1Resource+6, "\p", "\p", "\p");
	HUnlock(vers1Resource);
	gMagicCookie = 0;
	GetFNum("\pGeneva", &fontNum);
	text = GetResource('TEXT', kAboutText);
	SetItemReadOnly(dlg, kAboutBoxUserItem, text, fontNum, 9);
	MyModalDialog(AboutBoxFilter, &item, false, true);
	MyDisposDialog(dlg);
	HPurge((Handle)bigIconPict);
	HPurge((Handle)logoPict);
	if (gMagicCookie == 1) {
		strcpy(sampleNewsServerErrorMessage, "999 This is a sample news server error message.");
		strcat(sampleNewsServerErrorMessage, CRLF);
		NewsServerErrorMessage("SAMPLE", sampleNewsServerErrorMessage);
	} else if (gMagicCookie == 2) {
		CustomizeNewsWatcher();
	}
}
