#ifndef _RGNMASTER_H_
#define _RGNMASTER_H_

enum {
	kMainDialogID = 128,
	kSaveDialogID = 129,
	kOptionsDialogID = 130,
	kAppleMenuID = 130,
	kFileMenuID = 128,
	kEditMenuID = 129,
	kHelpStrsID = 128,
	kWrongSysVersionAlertID = 1945
};

enum {	
	convertAndReplaceItem = 1,
	convertOnlyItem,
	viewClipboardItem,
	viewRgnItem,
	rgnInfoItem,
	copyToClipItem,
	saveToAppItem,
	saveToFileItem,
	quitItem,
	optionsItem,
	aboutItem,
	helpWindItem,
	previewCheckbx,
	helpCheckbx,
	helpItem,
	frameItem1,
	frameItem2,
	frameItem3,
	frameItem4
};

// ---------------------------------------------------------------------------

extern Boolean gPreviewAlways;
extern short gAppFileRef;

void DrawMsg(Str255 theMsg);
void DoError(Str255 theMsg);

#endif // _RGNMASTER_H_