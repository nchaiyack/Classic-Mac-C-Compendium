#ifndef __DIALOG_LAYER_H__
#define __DIALOG_LAYER_H__

enum	/* standard dialog resource id's */
{
	kSmallAlertID = 128,		/* for small stuff */
	kLargeAlertID,				/* for large stuff */
	kSaveAlertID				/* save changes alert */
};

enum	/* alert types */
{
	kStopAlert=0,
	kCautionAlert,
	kNoteAlert
};

DialogPtr SetupTheDialog(short resourceID, short defaultButton,
	unsigned char *dialogTitle, UniversalProcPtr filterProc,
	UniversalProcPtr *modalFilterProc);
short DisplayTheAlert(short alertType, short resourceID, unsigned char *paramString1,
	unsigned char *paramString2, unsigned char *paramString3, unsigned char *paramString4,
	UniversalProcPtr filterProc);
void DisplayTheDialog(DialogPtr theDialog, Boolean makeVisible);
void ShutDownTheDialog(DialogPtr theDialog, UniversalProcPtr modalFilterProc);
short GetOneDialogEvent(DialogPtr theDialog, UniversalProcPtr modalFilterProc);
void GetDialogItemString(DialogPtr theDialog, short itemNum, unsigned char *theString);
void SetDialogItemString(DialogPtr theDialog, short itemNum, unsigned char *theString);
void HighlightDialogTextItem(DialogPtr theDialog, short itemNum, short start, short end);
short GetDialogCurrentEditField(DialogPtr theDialog);
TEHandle GetDialogCurrentTextHandle(DialogPtr theDialog);
Boolean AnyHighlightedInDialogQQ(DialogPtr theDialog);
short FakeSelect(DialogPtr theDialog, short itemNum);
void SetButtonHighlight(DialogPtr theDialog, short itemNum, Boolean isOn);
Boolean ButtonIsHighlightedQQ(DialogPtr theDialog, short itemNum);
void SetButtonTitle(DialogPtr theDialog, short itemNum, Str255 theStr);
void SetCheckboxState(DialogPtr theDialog, short itemNum, Boolean isOn);
Boolean CheckboxIsCheckedQQ(DialogPtr theDialog, short itemNum);
void PositionDialog(ResType theType, short theID);
pascal void OutlineDefaultButton(DialogPtr myDlog, short itemNum);
pascal Boolean OneButtonFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem);
pascal Boolean TwoButtonFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem);
pascal Boolean ThreeButtonFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem);

#endif
