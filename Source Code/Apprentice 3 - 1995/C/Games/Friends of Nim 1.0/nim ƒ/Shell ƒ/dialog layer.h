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
	kPlainAlert=0,
	kStopAlert,
	kCautionAlert,
	kNoteAlert
};

#ifdef __cplusplus
extern "C" {
#endif

extern	DialogPtr SetupTheDialog(short resourceID, short userItemForDefaultButton,
	unsigned char *dialogTitle, UniversalProcPtr filterProc,
	UniversalProcPtr *modalFilterProc, UniversalProcPtr *userItemProc);
extern	short DisplayTheAlert(short alertType, short resourceID, unsigned char *paramString1,
	unsigned char *paramString2, unsigned char *paramString3, unsigned char *paramString4,
	UniversalProcPtr filterProc);
extern	void DisplayTheDialog(DialogPtr theDialog, Boolean makeVisible);
extern	void ShutDownTheDialog(DialogPtr theDialog, UniversalProcPtr modalFilterProc,
	UniversalProcPtr userItemProc);
extern	short GetOneDialogEvent(UniversalProcPtr modalFilterProc);
extern	void GetDialogItemString(DialogPtr theDialog, short itemNum, unsigned char *theString);
extern	void SetDialogItemString(DialogPtr theDialog, short itemNum, unsigned char *theString);
extern	void SetDialogUserItem(DialogPtr theDialog, short itemNum, UniversalProcPtr proc,
	UniversalProcPtr *outputProc);
extern	void GetDialogItemRect(DialogPtr theDialog, short itemNum, Rect *theRect);
extern	void HighlightDialogTextItem(DialogPtr theDialog, short itemNum, short start, short end);
extern	short GetDialogCurrentEditField(DialogPtr theDialog);
extern	TEHandle GetDialogCurrentTextHandle(DialogPtr theDialog);
extern	Boolean AnyHighlightedInDialogQQ(DialogPtr theDialog);
extern	short FakeSelect(DialogPtr theDialog, short itemNum);
extern	void SetButtonHighlight(DialogPtr theDialog, short itemNum, Boolean isOn);
extern	Boolean ButtonIsHighlightedQQ(DialogPtr theDialog, short itemNum);
extern	void SetButtonTitle(DialogPtr theDialog, short itemNum, Str255 theStr);
extern	void SetCheckboxState(DialogPtr theDialog, short itemNum, Boolean isOn);
extern	Boolean CheckboxIsCheckedQQ(DialogPtr theDialog, short itemNum);
extern	void SetDialogControlInfo(DialogPtr theDialog, short itemNum, short min, short max, short value);
extern	short GetDialogControlValue(DialogPtr theDialog, short itemNum);
extern	void SetDialogControlValue(DialogPtr theDialog, short itemNum, short theValue);
extern	void PositionDialog(ResType theType, short theID);
extern	pascal void OutlineDefaultButton(DialogPtr myDlog, short itemNum);
extern	pascal Boolean OneButtonFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem);
extern	pascal Boolean TwoButtonFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem);
extern	pascal Boolean SaveModalFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem);
extern	pascal Boolean PoorMansFilterProc(DialogPtr theDialog, EventRecord *theEvent, short *theItem,
	ListHandle theList);
extern	Boolean CheckCommandKey(DialogPtr theDialog, EventRecord *theEvent, short *theItem,
	unsigned char upperChar, short equivalentItem);

#ifdef __cplusplus
}
#endif

#endif
