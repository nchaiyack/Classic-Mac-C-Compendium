/* Main.h */

#ifndef Included_Main_h
#define Included_Main_h

/* forwards */
struct MenuItemType;

#ifndef CompilingMainC
	extern struct MenuItemType*			mAboutThisProgram;
	extern struct MenuItemType*			mOpenFile;
	extern struct MenuItemType*			mCloseFile;
	extern struct MenuItemType*			mSaveArticle;
	extern struct MenuItemType*			mAppendArticle;
	extern struct MenuItemType*			mSaveSelection;
	extern struct MenuItemType*			mAppendSelection;
	extern struct MenuItemType*			mQuit;
	extern struct MenuItemType*			mUndo;
	extern struct MenuItemType*			mCut;
	extern struct MenuItemType*			mCopy;
	extern struct MenuItemType*			mPaste;
	extern struct MenuItemType*			mClear;
	extern struct MenuItemType*			mSelectAll;
	extern struct MenuItemType*			mEnterSelection;
	extern struct MenuItemType*			mFind;
	extern struct MenuItemType*			mFindAgain;
	extern struct MenuItemType*			mBalanceParens;
	extern struct MenuItemType*			mGotoLine;
	extern struct MenuItemType*			mShowSelection;
#endif

#endif
