// Copyright � 1994-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// MenuGlobals.h


#ifdef  __MENUGLOBALS__
#error "Already included MenuGlobals.h"
#endif

#define __MENUGLOBALS__

/******************************************************** Menus */
#define mGroupLists							0x10
#define hGroupLists							"\0x10"

#define mDiscussions						0x18
#define mDiscussionsPrefs					0x19
#define hDiscussionsPrefs					"\0x19"

#define mArticles							0x20
#define mArticlesPrefs						0x21
#define hArticlesPrefs						"\0x21"

#define mPreferences						0x30
#define mFontName							0x31
#define hFontName							"\0x31"
#define mFontSize							0x32
#define hFontSize							"\0x32"

#define mWindows							123

#define mTest								0x3A

#define cGroupListsMenu						1900
#define cDiscussionsPrefs					1910
#define cArticlesPrefs						1920
#define cFontNameMenu						1930
#define cFontSizeMenu						1931

#define cMenuChangeFontSize					2000

#define kHelpMenuStrings					2100
#define cHelpMenuCommandOffset				2100

#define mNewsServerTranslateMenuIn			15180
#define mNewsServerTranslateMenuOut			15181
