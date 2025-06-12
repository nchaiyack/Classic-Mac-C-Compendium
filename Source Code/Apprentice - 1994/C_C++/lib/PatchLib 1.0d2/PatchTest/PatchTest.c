/*	See the file Distribution for distribution terms.
	(c) Copyright 1994 Ari Halberstadt */

/*	Application to demonstrate the use of the patch library. First, a dialog
	is displayed without patching any traps. Then, a second dialog is
	displayed with the patch SetIText patched. The patch replaces the text
	passed to SetIText with a string pointed to by the global variable
	gPatchText. */

/*	94/04/18 aih - created. */

#include <Dialogs.h>
#include <Events.h>
#include <Fonts.h>
#include <Memory.h>
#include <Menus.h>
#include <OSEvents.h>
#include <QuickDraw.h>
#include <Traps.h>
#include "PatchLib.h"

/*----------------------------------------------------------------------------*/
/* global definitions and declarations */
/*----------------------------------------------------------------------------*/

/* the test dialog */
#define rDialog (128)

/* dialog items */
enum {
	iOK = 1,
	iTextTitle,
	iTextNoPatch,
	iTextPatch1,
	iTextPatch2,
	iTextPatch3,
	iLast
};

/* the patch data */
static PatchType gPatchSetIText;

/* the text to set when the patch is executed */
static const unsigned char *gPatchText;

/*----------------------------------------------------------------------------*/
/* standard Macintosh initializations */
/*----------------------------------------------------------------------------*/

/* initialize application heap */
static void HeapInit(long stack, short masters)
{
	SetApplLimit(GetApplLimit() - stack);
	MaxApplZone();
	while (masters-- > 0)
		MoreMasters();
}

/* initialize managers */
static void ManagersInit(void)
{
	EventRecord event;
	short i;
	
	/* standard initializations */
	InitGraf((Ptr) &qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NULL);
	FlushEvents(everyEvent, 0);
	InitCursor();
	
	/* so first window will be frontmost */
	for (i = 0; i < 4; i++)
		EventAvail(everyEvent, &event);
}

/*----------------------------------------------------------------------------*/
/* the patch */
/*----------------------------------------------------------------------------*/

/* subsititute some global string for whatever string is passed in */
static pascal void PatchSetIText(Handle hndl, const unsigned char *text)
{
	PATCH_ENTER();
	text = gPatchText;
	PATCH_RETURN(gPatchSetIText);
}

/*----------------------------------------------------------------------------*/
/* run the dialog */
/*----------------------------------------------------------------------------*/

/* create the dialog and run the program */
static void Run(Boolean patch)
{
	/* some strings to use in patches */
	const unsigned char *textStrings[] = {
		(StringPtr) "\p There isn't a patch in sight!",
		(StringPtr) "\p It's great to have a working patch!",
		(StringPtr) "\p This patch never made it to beta.",
		(StringPtr) "\p Thou Shalt Not Patch Unless Thou Hath A Flat.",
	};
	DialogPtr dlg;		/* the dialog */
	Handle text;		/* handle to text item */
	Rect box;			/* rectangle containing dialog item */
	short type;			/* type of dialog item */
	short item;			/* item in dialog */
	short textIndex;	/* index to text strings */
	
	/* create the dialog */
	dlg = GetNewDialog(rDialog, NULL, (WindowPtr) -1);
	if (! dlg) {
		DebugStr((StringPtr) "\p nil dialog pointer");
		return;
	}
	
	/* set title text */
	GetDItem(dlg, iTextTitle, &type, &text, &box);
	if (patch)
		SetIText(text, (StringPtr) "\p Running Dialog And Patching SetIText");
	else
		SetIText(text, (StringPtr) "\p Running Dialog Without Patching SetIText");
	
	/* create the patch */
	if (patch) {
		gPatchSetIText = PatchBegin(PatchSetIText, _SetIText);
		if (! gPatchSetIText) {
			DebugStr((StringPtr) "\p nil patch pointer");
			return;
		}
	}
	
	/* temporarily remove the patch and set a string */
	if (patch)
		PatchRemove(gPatchSetIText);
	GetDItem(dlg, iTextNoPatch, &type, &text, &box);
	SetIText(text, textStrings[0]);
	if (patch)
		PatchInstall(gPatchSetIText);

	/* set strings in the dialog */
	textIndex = 1;
	for (item = iTextNoPatch + 1; item < iLast; item++) {
		/*	Even though we're always passing the same string to SetIText,
			the patch will use the string pointed to be gPatchText. */
		gPatchText = textStrings[textIndex++];
		GetDItem(dlg, item, &type, &text, &box);
		SetIText(text, textStrings[0]);
	}
	
	/* wait until user hits an enabled item */
	ModalDialog(NULL, &item);
	
	/* cleanup */
	DisposeDialog(dlg);
	gPatchSetIText = NULL;
	PatchEndAll();
}

void main(void)
{
	HeapInit(0, 4);
	ManagersInit();
	Run(false);
	Run(true);
}
