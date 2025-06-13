// Copyright © 1992-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// Menus.r

/* ==============================   Includes   ================================= */

#ifndef _TYPES.R_
#include "Types.r"
#endif

#ifndef _MacAppTypes_
#include "MacAppTypes.r"
#endif

#ifndef __RSRCGLOBALS__
#include "RsrcGlobals.h"
#endif


#include "MenuBalloons.r"

/* ================================   qFinal   ================================== */

#ifndef qNoCode
#define qNoCode 0
#endif

#ifndef qFinal
#if qNoCode
#define qFinal !qDebug
#else
#define qFinal FALSE
#endif
#endif

/* ================================   Menus   ================================== */

resource 'CMNU' (mApple) {
	mApple,
	textMenuProc,
	AllItems,
	enabled,
	apple,
	{
		"About Nuntius…",		noIcon,	noKey,	noMark,	plain,	cAboutApp;
		"-",								noIcon,	noKey,	noMark,	plain,	nocommand
	}
};


/* ------------------------------------------------------------------------------ */

resource 'CMNU' (mFile) {
	mFile,
	textMenuProc,
	allEnabled,
	enabled,
	"File",
	{
		"New GroupList",				noIcon,	noKey,	noMark,	plain,	cNewGroupListDoc;
		"Open…",								noIcon, "O",		noMark, plain,	cOpen;
		"Open special",					noIcon, hierarchicalMenu, hGroupLists, plain, cGroupListsMenu;
		"Open list of all groups",noIcon,	noKey, noMark, plain,	cOpenListOfAllGroups;
		"-",										noIcon,	noKey,	noMark,	plain,	nocommand;
		"Close",								noIcon,	"W",		noMark,	plain,	cClose;
		"Save",									noIcon, "S",		noMark, plain,	cSave;
		"Save as…",							noIcon,	noKey,	noMark,	plain,	cSaveAs;
		"Save a copy as…",			noIcon,	noKey,	noMark,	plain,	cSaveCopy;
		"Revert to Saved",			noIcon, noKey,  noMark, plain,  cRevert;
		"-",										noIcon,	noKey,	noMark,	plain,	nocommand;
		"Extract binaries",	noIcon, "E",		noMark,	plain,	cExtractBinaries;
		"-",										noIcon,	noKey,	noMark,	plain,	nocommand;
		"Page Setup…",					noIcon,	noKey,	noMark,	plain,	cPageSetup;
		"Print…",								noIcon,	"P",		noMark,	plain,	cPrint;
		"-",										noIcon,	noKey,	noMark,	plain,	nocommand;
		"Quit",									noIcon,	"Q",		noMark,	plain,	cQuit
	}
};

/* ------------------------------------------------------------------------------ */
resource 'CMNU' (mGroupLists) {
	mGroupLists,
	textMenuProc,
	allEnabled,
	enabled,
	"GroupLists",
	{
	}
};


/* ------------------------------------------------------------------------------ */

resource 'CMNU' (mEdit) {
	mEdit,
	textMenuProc,
	allEnabled,
	enabled,
	"Edit",
	{
		"Undo",								noIcon,	"Z",		noMark,	plain,	cUndo;
		"-",									noIcon,	noKey,	noMark,	plain,	nocommand;
		"Cut",								noIcon,	"X",		noMark,	plain,	cCut;
		"Copy",								noIcon,	"C",		noMark,	plain,	cCopy;
		"Copy as quotation",	noIcon, "'",		noMark, plain,	cCopyAsQuote;
		"Paste",							noIcon,	"V",		noMark,	plain,	cPaste;
		"Clear",							noIcon,	noKey,	noMark,	plain,	cClear;
		"Select all",					noIcon, "A",		noMark, plain,	cSelectAll;
		"-",									noIcon, noKey,	noMark, plain,	nocommand;
		"Show Clipboard",			noIcon,	noKey,	noMark,	plain,	cShowClipboard
	}
};

/* ------------------------------------------------------------------------------ */

resource 'CMNU' (mDiscussions) {
	mDiscussions,
	textMenuProc,
	allEnabled,
	enabled,
	"Threads",
	{
		"Post article in new thread…",						noIcon, "N",   noMark, plain, cPostNewDiscussion;
		"-",																			noIcon,	noKey, noMark, plain,	nocommand;
		"Show all threads",												noIcon, noKey, noMark, plain, cShowAllDiscussions;
		"Show threads updated today",							noIcon, noKey, noMark, plain, cShowOnlyTodayDiscussions;
		"Show threads with unread articles",			noIcon, noKey, noMark, plain, cShowDiscsWithUnreadArticles;
		"Show threads with new articles",					noIcon, noKey, noMark, plain, cShowDiscsWithNewArticles;
		"When opening a group",										noIcon, hierarchicalMenu, hDiscussionsPrefs, plain, cDiscussionsPrefs;
		"-",																			noIcon,	noKey, noMark, plain,	nocommand;
		"Mark articles as new",	 									noIcon, noKey, noMark, plain, cMarkThreadAsNew;
		"Mark articles as unread",								noIcon, noKey, noMark, plain, cMarkThreadAsSeen;
		"Mark articles as read",									noIcon, "M",   noMark, plain, cMarkThreadAsRead;
	}
};

resource 'CMNU' (mDiscussionsPrefs) {
	mDiscussionsPrefs,
	textMenuProc,
	allEnabled,
	enabled,
	"mDiscussionsPrefs",
	{
		"Show all threads",												noIcon, noKey, noMark, plain, cPrefShowAllDiscussions;
		"Show threads updated today",							noIcon, noKey, noMark, plain, cPrefShowOnlyTodayDiscussions;
		"Show threads with unread articles",			noIcon, noKey, noMark, plain, cPrefShowDiscsWithUnreadArticles;
		"Show threads with new articles",					noIcon, noKey, noMark, plain, cPrefShowDiscsWithNewArticles;
	}
};
/* ------------------------------------------------------------------------------ */

resource 'CMNU' (mArticles) {
	mArticles,
	textMenuProc,
	allEnabled,
	enabled,
	"Articles",
	{
		"Post follow-up article…",								noIcon, "F",   noMark, plain, cPostFollowUpArticle;
		"Reply to author by mail",								noIcon, "R",   noMark, plain, cMailLetter;
		"-",																			noIcon,	noKey, noMark, plain,	nocommand;
		"Expand all articles",										noIcon, noKey, noMark, plain, cShowAllArticles;
		"Expand first article",										noIcon, noKey, noMark, plain, cShowOnlyFirstArticle;
		"Expand unread articles",									noIcon, noKey, noMark, plain, cShowOnlyUnreadArticles;
		"Expand new articles",										noIcon, noKey, noMark, plain, cShowOnlyNewArticles;
		"Expand no articles",											noIcon, noKey, noMark, plain, cShowNoneArticles;
		"When opening a thread",									noIcon, hierarchicalMenu, hArticlesPrefs, plain, cArticlesPrefs;
		"-",																			noIcon,	noKey, noMark, plain,	nocommand;
		"Save selection in note file",						noIcon, "T",   noMark, plain, cStoreNotes;
		"Show Full Headers",											noIcon, "H",   noMark, plain, cShowHideHeadersCommand;
		"Use ROT13",															noIcon, "D",   noMark, plain, cUseROT13;
		"-",																			noIcon,	noKey, noMark, plain,	nocommand;
		"Cancel article",													noIcon, noKey, noMark, plain, cCancelArticle;
	}
};

resource 'CMNU' (mArticlesPrefs) {
	mArticlesPrefs,
	textMenuProc,
	allEnabled,
	enabled,
	"mArticlesPrefs",
	{
		"Expand all articles",										noIcon, noKey, noMark, plain, cPrefShowAllArticles;
		"Expand first article",										noIcon, noKey, noMark, plain, cPrefShowOnlyFirstArticle;
		"Expand unread articles",									noIcon, noKey, noMark, plain, cPrefShowOnlyUnreadArticles;
		"Expand new articles",										noIcon, noKey, noMark, plain, cPrefShowOnlyNewArticles;
		"Expand none of the articles",						noIcon, noKey, noMark, plain, cPrefShowNoneArticles;
	}
};
/* ------------------------------------------------------------------------------ */
resource 'CMNU' (mPreferences) {
	mPreferences,
	textMenuProc,
	allEnabled,
	enabled,
	"Prefs",
	{
		"Font size",														noIcon, hierarchicalMenu, hFontSize, plain, cFontSizeMenu;
		"Font",																	noIcon, hierarchicalMenu, hFontName, plain, cFontNameMenu;
		"-",																		noIcon,	noKey, noMark, plain,	nocommand;
		"Your name…",														noIcon,	noKey, noMark, plain,	cOpenYourNamePrefs;
		"Binaries…",														noIcon,	noKey, noMark, plain,	cOpenBinariesPrefs;
		"Editing articles…",										noIcon, noKey, noMark, plain, cOpenEditorPrefs;
		"Mail…",																noIcon, noKey, noMark, plain, cOpenMailerPrefs;
		"News server…",													noIcon,	noKey, noMark, plain,	cOpenNewsServerPrefs;
		"Expire…",															noIcon,	noKey, noMark, plain,	cExpirePrefs;
		"Misc…",																noIcon,	noKey, noMark, plain,	cOpenMiscPrefs;
		"-",																		noIcon,	noKey, noMark, plain,	nocommand;
		"Check for new groups",									noIcon, noKey, noMark, plain, cUpdateGroupTree;
		"Rebuild list of all groups",						noIcon, noKey, noMark, plain, cRebuildGroupTree;
		"Check for new articles now",						noIcon, "U",   noMark, plain, cCheckForNewArticles;
		"Forget Password",											noIcon,	noKey, noMark, plain,	cForgetPassword;
		"Close idle NNTP connections",					noIcon, noKey, noMark, plain, cFlushUnusedNntpConnections;
		"Flush all caches",											noIcon, noKey, noMark, plain, cAllCaches;
	}
};

/* ------------------------------------------------------------------------------ */
resource 'CMNU' (mFontName) {
	mFontName,
	textMenuProc,
	allEnabled,
	enabled,
	"Font",
	{
	}
};

resource 'CMNU' (mFontSize) {
	mFontSize,
	textMenuProc,
	allEnabled,
	enabled,
	"Font size",
	{
		"9",																		noIcon,	noKey, noMark, plain,	nocommand;
		"10",																		noIcon,	noKey, noMark, plain,	nocommand;
		"12",																		noIcon,	noKey, noMark, plain,	nocommand;
		"14",																		noIcon,	noKey, noMark, plain,	nocommand;
		"18",																		noIcon,	noKey, noMark, plain,	nocommand;
		"24",																		noIcon,	noKey, noMark, plain,	nocommand;
	}
};

/* ------------------------------------------------------------------------------ */
resource 'CMNU' (mWindows) {
	mWindows,
	textMenuProc,
	allEnabled,
	enabled,
	"Windows",
	{
	}
};


/* ------------------------------------------------------------------------------ */
resource 'CMNU' (mNewsServerTranslateMenuIn) {
	mNewsServerTranslateMenuIn, textMenuProc, allEnabled, enabled, "mNewsServerTranslateMenuIn",
	{}
};
resource 'CMNU' (mNewsServerTranslateMenuOut) {
	mNewsServerTranslateMenuOut, textMenuProc, allEnabled, enabled, "mNewsServerTranslateMenuOut",
	{}
};
/* ------------------------------------------------------------------------------ */
#if !qFinal
// if THIS resource exist, it will be assumed to be a non-final version
resource 'CMNU' (mTest) {
	mTest,
	textMenuProc,
	allEnabled,
	enabled,
	"Test",
	{
		"Into MacsBug",							noIcon, noKey, noMark, plain, cMacsBug;
		"Into SysBug",							noIcon, noKey, noMark, plain, cSysBug;
		"Dump preferences",					noIcon, noKey, noMark, plain, cDumpPrefs;
		"Dump Threads",							noIcon, noKey, noMark, plain, cDebugDumpThreads;
		"Dump GroupDatabase",				noIcon, noKey, noMark, plain, cDebugDumpGroupDatabase;
		"Dump FatalErrorMsg",				noIcon, noKey, noMark, plain, cMakeFatalErrorDescription;
		"Dump ArticleText cache",		noIcon, noKey, noMark, plain, cDebugDumpArticleTextCache;
		"-",												noIcon, noKey, noMark, plain, nocommand;
		"Test 1",										noIcon, noKey, noMark, plain, cTest1;
		"Test 2",										noIcon, noKey, noMark, plain, cTest2;
		"Test 3",										noIcon, noKey, noMark, plain, cTest3;
		"Test 4",										noIcon, noKey, noMark, plain, cTest4;
		"-",												noIcon, noKey, noMark, plain, nocommand;
		"Show view inspector",			noIcon, noKey, noMark, plain, cShowViewInspector;
		"Show target inspector",		noIcon, noKey, noMark, plain, cShowTargetInspector;
		"-",												noIcon, noKey, noMark, plain, nocommand;
		"Make fatal error",					noIcon, noKey, noMark, plain, cMenuForcedFatalError;
		"-",												noIcon, noKey, noMark, plain, nocommand;
		"Flush volumes",						noIcon, noKey, noMark, plain, cToggleFlushVolumes;
}};
#endif
/* ------------------------------------------------------------------------------ */

resource 'CMNU' (mBuzzWords) {
	mBuzzWords,
	textMenuProc,
	allEnabled,
	enabled,
	"Buzzwords",
	{
		"Page Setup Change", noIcon, noKey, noMark, plain, cChangePrinterStyle
	}
};

/* ------------------------------------------------------------------------------ */

resource 'MBAR' (kMBarDisplayed, "kMBarDisplayed") {{
	mApple;
	mFile;
	mEdit;
	mDiscussions;
	mArticles;
	mPreferences;
	mWindows;
#if !qFinal
	mTest;
#endif
}};

resource 'MBAR' (kMBarHierarchical, "kMBarHierarchical") {{
	mGroupLists;
	mDiscussionsPrefs;
	mArticlesPrefs;
	mFontName;
	mFontSize;
	mNewsServerTranslateMenuIn;
	mNewsServerTranslateMenuOut;
}};
/* ------------------------------------------------------------------------------ */

//========================================================================

