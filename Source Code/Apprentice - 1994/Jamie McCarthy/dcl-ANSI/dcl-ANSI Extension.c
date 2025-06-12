/*
 * dcl-ANSI Extension.c
 *
 * By Jamie McCarthy, April 92.  This is public domain.
 *
 * This source file provides a bridge between dcl-ANSI.c,
 * which just does a translation, and BBEdit.
 *
 */



/******************************/

#include <SetupA4.h>
#include <pascal.h>
#include <ExternalInterface.h>
#include <DialogUtilities.h>

#include "dcl-ANSI.h"

/******************************/

	/*
	 * The DITL item numbers for the various controls.
	 */

enum {
	kShowWhenOptionKeyDown = 3,
	kShowWhenOptionKeyUp,
	
	kLookInClipboard,
	kLookInSelection,
	
	kPutToClipboard,
	kPutToSelection,
	kPutToDialog,
	
	kLine1,
	kLine2
} ;

/******************************/

	/*
	 * Parameters passed to main().
	 */
ExternalCallbackBlock *gCallbacks;
WindowPtr gWindPtr;

	/*
	 * The preferences.
	 */
struct {
	short show;
	short look;
	short put;
} gOptionsDCL;	

	/*
	 * Do we have a window?
	 * Was something selected when we started?
	 */
Boolean gWeHaveAWindow;
Boolean gNonEmptySelection;

	/*
	 * Information about the selection.
	 */
long gSelStart, gSelEnd, gFirstChar;

	/*
	 * C string data.
	 */
Handle gCStrList128=NULL, gCStrList129=NULL;

	/*
	 * The text we pass to ParamText.
	 */
Str255 gFinalStr[4];

/******************************/

pascal void main(ExternalCallbackBlock *callbacks, WindowPtr w);

void maintainButtons(DialogPtr d);
void getCDeclaration(void);
void putFinalString(void);

void startupCStrings(void);
void shutdownCStrings(void);

/******************************/



pascal void main(ExternalCallbackBlock *callbacks, WindowPtr w)
{
	DialogPtr d;
	short item;
	short	actualPrefsLen;
	KeyMap theKeys;
	Boolean showPrefsDialog;
	
	RememberA0();
	SetUpA4();
	
	gCallbacks = callbacks;
	gWindPtr = w;
	gError = noErr;
	
	gWeHaveAWindow = (w != NULL);
	if (gWeHaveAWindow) {
		callbacks->GetSelection(&gSelStart, &gSelEnd, &gFirstChar);
		gNonEmptySelection = (gSelEnd > gSelStart);
	}
	
	callbacks->GetPreference('dcl!', sizeof(gOptionsDCL), &gOptionsDCL, &actualPrefsLen);
	if (actualPrefsLen < sizeof(gOptionsDCL)) {
		gOptionsDCL.show = kShowWhenOptionKeyUp;
		gOptionsDCL.look = kLookInClipboard;
		gOptionsDCL.put = kPutToDialog;
		showPrefsDialog = TRUE;
	} else {
		GetKeys(theKeys);
		showPrefsDialog =
			( ((theKeys[1] & 0x04) != 0) ^ (gOptionsDCL.show == kShowWhenOptionKeyUp) );
	}
	
	if (showPrefsDialog) {
		GrafPtr oldPort;
		GetPort(&oldPort);
		d = callbacks->CenterDialog(128);
		SetPort(d);
		
		SetupUserItem(d, kLine1, callbacks->FrameDialogItem);
		SetupUserItem(d, kLine2, callbacks->FrameDialogItem);
		
		do {
			maintainButtons(d);
			ModalDialog(callbacks->StandardFilter, &item);
			if (item <= kShowWhenOptionKeyUp && item >= kShowWhenOptionKeyDown) {
				gOptionsDCL.show = item;
			} else if (item <= kLookInSelection && item >= kLookInClipboard) {
				gOptionsDCL.look = item;
			} else if (item >= kPutToClipboard && item <= kPutToDialog) {
				gOptionsDCL.put = item;
			}
		} while (item != ok && item != cancel);
		
		DisposDialog(d);
		SetPort(oldPort);
	} else {
		item = ok;
	}
	
	if (!gNonEmptySelection) {
		gOptionsDCL.look = kLookInClipboard;
	}
	
	if (item == ok) {
		
		if (showPrefsDialog) {
			callbacks->SetPreference('dcl!', sizeof(gOptionsDCL), &gOptionsDCL, &actualPrefsLen);
		}
		
		getCDeclaration();
		
		startupDcl();
		if (gError == noErr) {
			declarator();
		}
		buildFinalString();
		shutdownDcl();
		
		putFinalString();
		
	}
	
	RestoreA4();
}



/******************************/



void maintainButtons(DialogPtr d)
{
		/*
		 * If there's no selection, we can't look there for input.
		 */
	if (!gNonEmptySelection) {
		XAbleDlgCtl(d, kLookInSelection, FALSE);
		gOptionsDCL.look = kLookInClipboard;
	}
	
	SetDlgCtl(d, kShowWhenOptionKeyDown,	(gOptionsDCL.show == kShowWhenOptionKeyDown));
	SetDlgCtl(d, kShowWhenOptionKeyUp,		(gOptionsDCL.show == kShowWhenOptionKeyUp));
	SetDlgCtl(d, kLookInClipboard,			(gOptionsDCL.look == kLookInClipboard));
	SetDlgCtl(d, kLookInSelection,			(gOptionsDCL.look == kLookInSelection));
	SetDlgCtl(d, kPutToClipboard,				(gOptionsDCL.put == kPutToClipboard));
	SetDlgCtl(d, kPutToSelection,				(gOptionsDCL.put == kPutToSelection));
	SetDlgCtl(d, kPutToDialog,					(gOptionsDCL.put == kPutToDialog));
}



void getCDeclaration(void)
{
	if (gOptionsDCL.look == kLookInClipboard) {
		long theLength, theOffset;
		gCDeclHndl = NewHandle(0);
		theLength = GetScrap(gCDeclHndl, 'TEXT', &theOffset);
		if (theLength < 0) {
			gError = theLength;
			gCDeclHndl = NULL;
		}
	} else {
		long theLength, theFreeMem;
		char *selStartPtr;
		theLength = gSelEnd - gSelStart;
		theFreeMem = CompactMem(theLength);
		if (theFreeMem < theLength) {
			gError = memFullErr;
			gCDeclHndl = NULL;
		} else {
			gCDeclHndl = NewHandle(theLength);
			selStartPtr = *gCallbacks->GetWindowContents(gWindPtr) + gSelStart;
			BlockMove(selStartPtr, *gCDeclHndl, theLength);
		}
	}
}



void putFinalString(void)
{
	switch (gOptionsDCL.put) {
		
		case kPutToClipboard: {
			long theLength;
			gError = ZeroScrap();
			if (gError == noErr) {
				theLength = strlen(gEnglish);
				gError = PutScrap(theLength, 'TEXT', &gEnglish[0]);
			}
		}	break;
			
		case kPutToSelection: {
			Handle theText;
			short englishLen;
			long diffLen;
			long oldTextLen;
			englishLen = strlen(gEnglish);
			diffLen = englishLen+gSelStart-gSelEnd;
			if (!gWeHaveAWindow) {
				gWindPtr = gCallbacks->NewDocument();
				gSelStart = gSelEnd = gFirstChar = 0;
			}
			theText = gCallbacks->GetWindowContents(gWindPtr);
			oldTextLen = GetHandleSize(theText);
			if (diffLen > 0) {
				SetHandleSize(theText, oldTextLen+diffLen);
				BlockMove(*theText + gSelEnd,
					*theText + gSelEnd + diffLen,
					oldTextLen - gSelEnd);
			} else {
				BlockMove(*theText + gSelEnd,
					*theText + gSelEnd + diffLen,
					oldTextLen - gSelEnd);
				SetHandleSize(theText, oldTextLen+diffLen);
			}
			BlockMove(gEnglish, *theText+gSelStart, englishLen);
			gCallbacks->ContentsChanged(gWindPtr);
			gCallbacks->SetSelection(gSelStart, gSelStart+englishLen, gFirstChar);
		}	break;
			
		case kPutToDialog: {
			short wFinalStr;
			short englishLen, englishOffset;
			GrafPtr oldPort;
			DialogPtr theReportDlg;
			short item;
			
				/*
				 * Break gEnglish into four Pascal strings and stuff them
				 * into gFinalStr[0-3].
				 */
			
			gFinalStr[0][0] = gFinalStr[1][0] = gFinalStr[2][0] = gFinalStr[3][0] = 0;
			wFinalStr = 0;
			englishLen = strlen(gEnglish);
			englishOffset = 0;
			while (englishLen > 0) {
				if (englishLen > 239) {
					gFinalStr[wFinalStr][0] = 240;
					BlockMove(&gEnglish[englishOffset], &gFinalStr[wFinalStr][1], 240);
					englishLen -= 240;
					englishOffset += 240;
					++wFinalStr;
				} else {
					gFinalStr[wFinalStr][0] = englishLen;
					BlockMove(&gEnglish[englishOffset], &gFinalStr[wFinalStr][1], englishLen);
					englishLen = 0;
				}
			}
			
			GetPort(&oldPort);
			ParamText(gFinalStr[0], gFinalStr[1], gFinalStr[2], gFinalStr[3]);
			theReportDlg = gCallbacks->CenterDialog(129);
			SetPort(theReportDlg);
			do {
				ModalDialog(gCallbacks->StandardFilter, &item);
			} while (item != ok);
			DisposDialog(theReportDlg);
			SetPort(oldPort);
		}	break;
			
	}
}



/******************************/



void startupCStrs(void)
{
	gCStrList128 = GetResource('CST#', 128);
	if (gCStrList128 == NULL) {
		gError = resNotFound;
			// kind of pointless;  we're going to get a bus error in just a few
			// milliseconds, but oh well...you dink with my resource file, you
			// pay the consequences...
	} else {
		HLock(gCStrList128);
		gCStrList129 = GetResource('CST#', 129);
		if (gCStrList129 == NULL) {
			gError = resNotFound;
		} else {
			HLock(gCStrList129);
		}
	}
}



short getNMiscCStrs(void)
{
	return *(long*)*gCStrList128;
}



char *getMiscCStr(short index)
{
	return *gCStrList128 + ((long*)*gCStrList128)[index];
}



short getNSpecifiers(void)
{
	return *(long*)*gCStrList129;
}



char *getSpecifier(short index)
{
	return *gCStrList129 + ((long*)*gCStrList129)[index];
}



void shutdownCStrs(void)
{
	if (gCStrList128 != NULL) ReleaseResource(gCStrList128);
	if (gCStrList129 != NULL) ReleaseResource(gCStrList129);
}
