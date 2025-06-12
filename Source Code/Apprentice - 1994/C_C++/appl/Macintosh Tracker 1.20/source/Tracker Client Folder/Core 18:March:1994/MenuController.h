/* MenuController.h */

#pragma once

/* required definitions:  */
enum
	{
		mNoItem = 0,
		mEditCut = 0x0001,
		mEditPaste = 0x0002,
		mEditCopy = 0x0003,
		mEditClear = 0x0004,
		mFileNew = 0x0005,
		mFileOpen = 0x0006,
		mFileClose = 0x0007,
		mFileSave = 0x0008,
		mFileSaveAs = 0x0009,
		mFileQuit = 0x000a,
		mFilePageSetup = 0x000b,
		mFilePrint = 0x000c,
		mEditUndo = 0x000d,
		mEditSelectAll = 0x000e,
		mAppleAbout = 0x000f,
		mFontSelected = 0x0010
	};


void		InitMyMenus(void);
void		ShutDownMyMenus(void);
void		MyEnableItem(short ItemID);
void		MyDisableItem(short ItemID);
void		MySetItemMark(short ItemID, char ItemMark);
short		CreateMenu(Handle Name);
short		CreateHierarchicalMenu(Handle Name);
short		AddItemToMenu(short MenuID, Handle ItemName, char ItemShortcut);
void		DeleteItemFromMenu(short ItemID);
void		MyDeleteMenu(short MenuID);
void		PostMenuToBar(short MenuID);
void		RemoveMenuFromBar(short MenuID);
short		MenuMouseDown(EventRecord* TheEvent); /* track the menu request */
short		MenuKeyDown(EventRecord* TheEvent);  /* decode command key equivalent */
void		Reconstruct(short MenuID);
void		ChangeName(short ItemID, Handle NewName);
short		AddFontMenu(void);
short		GetLastFontID(void);
void		SetFontMark(short FontID);
