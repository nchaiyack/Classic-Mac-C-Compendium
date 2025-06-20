/* Main.h */

#ifndef Included_Main_h
#define Included_Main_h

#include "Screen.h"
#include "Menus.h"

struct MenuToFont
	{
		FontType			FontID;
		MenuItemType*	MenuItemID;
	};
typedef struct MenuToFont MenuToFont;

MyBoolean					ItsInTheFontList(MenuItemType* MenuID);
FontType					GetFontFromMenuItem(MenuItemType* MenuID);

#ifndef Including_Main_c
	extern MenuType*				mmAppleMenu;
	extern MenuType*				mmFileMenu;
	extern MenuType*				mmEditMenu;
	extern MenuType*				mmSearchMenu;
	extern MenuType*				mmWindowsMenu;
	extern MenuType*				mmFontMenu;
	extern MenuType*				mmSizeMenu;

	extern MenuItemType*		mAboutThisProgram;
	extern MenuItemType*		mNewFile;
	extern MenuItemType*		mOpenFile;
	extern MenuItemType*		mCloseFile;
	extern MenuItemType*		mSaveFile;
	extern MenuItemType*		mSaveAs;
	extern MenuItemType*		mAutoIndent;
	extern MenuItemType*		mSetTabSize;
	extern MenuItemType*		mConvertTabsToSpaces;
	extern MenuItemType*		mMacintoshLineFeeds;
	extern MenuItemType*		mUnixLineFeeds;
	extern MenuItemType*		mMsDosLineFeeds;
	extern MenuItemType*		mQuit;
	extern MenuItemType*		mUndo;
	extern MenuItemType*		mCut;
	extern MenuItemType*		mCopy;
	extern MenuItemType*		mPaste;
	extern MenuItemType*		mClear;
	extern MenuItemType*		mSelectAll;
	extern MenuItemType*		mEnterSelection;
	extern MenuItemType*		mFind;
	extern MenuItemType*		mFindAgain;
	extern MenuItemType*		mReplace;
	extern MenuItemType*		mReplaceAndFindAgain;
	extern MenuItemType*		mPrefixSelection;
	extern MenuItemType*		mBalanceParens;
	extern MenuItemType*		mShiftLeft;
	extern MenuItemType*		mShiftRight;
	extern MenuItemType*		mGotoLine;
	extern MenuItemType*		mShowSelection;
	extern MenuToFont*			mFontItemList;
	extern MenuItemType*		m9Points;
	extern MenuItemType*		m10Points;
	extern MenuItemType*		m12Points;
	extern MenuItemType*		m14Points;
	extern MenuItemType*		m18Points;
	extern MenuItemType*		m24Points;
	extern MenuItemType*		m30Points;
	extern MenuItemType*		m36Points;
	extern MenuItemType*		mOtherPoints;
#endif

#endif
