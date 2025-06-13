/* Main.h */

#ifndef Included_Main_h
#define Included_Main_h

#include "Menus.h"

#ifndef Including_Main_c
	extern char*						GlobalSearchString;
	extern char*						GlobalReplaceString;

	extern MenuType*				mmWindowMenu;

	extern MenuItemType*		mAboutThisProgram;
	extern MenuItemType*		mNewFile;
	extern MenuItemType*		mOpenFile;
	extern MenuItemType*		mCloseFile;
	extern MenuItemType*		mSaveFile;
	extern MenuItemType*		mSaveAs;
	extern MenuItemType*		mSetTabSize;
	extern MenuItemType*		mPlay;
	extern MenuItemType*		mPlayThisTrackFromHere;
	extern MenuItemType*		mPlayAllTracksFromHere;
	extern MenuItemType*		mPlayAIFFFile;
	extern MenuItemType*		mQuit;

	extern MenuItemType*		mUndo;
	extern MenuItemType*		mCut;
	extern MenuItemType*		mCopy;
	extern MenuItemType*		mPaste;
	extern MenuItemType*		mClear;
	extern MenuItemType*		mSelectAll;
	extern MenuItemType*		mShiftLeft;
	extern MenuItemType*		mShiftRight;
	extern MenuItemType*		mBalanceParens;
	extern MenuItemType*		mEnterSelection;
	extern MenuItemType*		mFind;
	extern MenuItemType*		mFindAgain;
	extern MenuItemType*		mReplace;
	extern MenuItemType*		mReplaceAndFindAgain;
	extern MenuItemType*		mShowSelection;
	extern MenuItemType*		mNewSample;
	extern MenuItemType*		mNewFunction;
	extern MenuItemType*		mNewAlgoSample;
	extern MenuItemType*		mNewWaveTable;
	extern MenuItemType*		mNewAlgoWaveTable;
	extern MenuItemType*		mNewTrack;
	extern MenuItemType*		mNewInstrument;
	extern MenuItemType*		mBuildFunction;
	extern MenuItemType*		mUnbuildFunction;
	extern MenuItemType*		mUnbuildAllFunctions;
	extern MenuItemType*		mBuildEntireProject;
	extern MenuItemType*		mDisassembleFunction;
	extern MenuItemType*		mOpenObject;
	extern MenuItemType*		mEditTrackAttributes;
	extern MenuItemType*		mDeleteObject;
	extern MenuItemType*		mCalculator;
	extern MenuItemType*		mEvaluateCalc;
	extern MenuItemType*		mAlgoSampToSample;
	extern MenuItemType*		mSampleEditLoop1;
	extern MenuItemType*		mSampleEditLoop2;
	extern MenuItemType*		mSampleEditLoop3;
	extern MenuItemType*		mImportWAVFormat;
	extern MenuItemType*		mImportAIFFFormat;
	extern MenuItemType*		mImportRAWFormat;
	extern MenuItemType*		mExportWAVFormat;
	extern MenuItemType*		mExportAIFFFormat;
	extern MenuItemType*		mExportRAWFormat;
	extern MenuItemType*		mCopyObject;
	extern MenuItemType*		mPasteObject;
	extern MenuItemType*		mTransposeSelection;
	extern MenuItemType*		mGotoMeasureBar;
#endif

#endif
