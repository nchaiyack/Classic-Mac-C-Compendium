// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// Views.r

/* ==============================   Includes   ================================= */

#ifndef _TYPES.R_
#include "Types.r"
#endif

#ifndef _MacAppTypes_
#include "MacAppTypes.r"
#endif

//#define XIncludeMyViewTypes TRUE

#ifndef __ViewTypes__
#include "ViewTypes.r"
#endif

#ifndef __RSRCGLOBALS__
#include "RsrcGlobals.h"
#endif

#include "FloatWindow.r"

#include "BalloonTypes.r"

/* ==============================   GroupTreeList   ================================= */
resource 'TxSt' (kGroupTreeTextStyle, "kGroupTreeTextStyle") {
	tsPlain, 10, black, "Geneva"
};
/* ==============================   GroupList   ================================= */
resource 'TxSt' (kGroupListTextStyle, "kGroupListTextStyle") {
	tsPlain, 10, black, "Geneva"
};
/* ==============================   DiscList   ================================= */

resource 'TxSt' (kDiscListViewTextStyle, "kDiscListViewTextStyle") {
	tsPlain, 10, black, "Geneva"
};

resource 'hdlg' (kDiscListViewHelp, "kDiscListViewHelp") {
	HelpMgrVersion,
	0,
	hmDefaultOptions,
	0,
	0,
	HMSkipItem {},
	{
	HMStringItem {
		{0, 0}, {0, 0, 0, 0},
		"This is the list of threads",
		"", "", ""
	},
	HMStringItem {
		{0, 0}, {0, 0, 0, 0},
		"This is the subject of the article. If this field is empty, the article has the same subject as the one above",
		"This article is not avaible", "", ""
	},
	HMStringItem {
		{0, 0}, {0, 0, 0, 0},
		"This field is contains a ¥ as the article is new",
		"This article is not avaible", "", ""
	},
	HMStringItem {
		{0, 0}, {0, 0, 0, 0},
		"This fields contains Ð as you have seen the article before, but not read it",
		"This article is not avaible", "", ""
	},
	HMStringItem {
		{0, 0}, {0, 0, 0, 0},
		"This field contains nothing as you have read the article before",
		"This article is not avaible", "", ""
	},
	HMStringItem {
		{0, 0}, {0, 0, 0, 0},
		"This is the author of the article with the subject to the left",
		"This article is not avaible", "", ""
	},
}};

/* ==============================   ArticleView   ================================= */
resource 'TxSt' (kArticleViewTextStyle, "kArticleViewTextStyle") {
	tsPlain, 10, black, "Courier"
};

resource 'TxSt' (kJapaneseFontTextStyle, "kJapaneseFontTextStyle") {
	tsPlain, 12, black, "Osaka"
};

resource 'hdlg' (kArticleViewHelp, "kArticleViewHelp") {
	HelpMgrVersion,
	0,
	hmDefaultOptions,
	0,
	0,
	HMSkipItem {},
	{
	HMStringItem {
		{6, 6}, {0, 0, 0, 0},
		"Click here to go to the previous thread. You can do it the from the keyboard by pressing left-arrow key.",
		"Click here to go to the previous thread.\nIs dimmed because you are viewing the first selected thread",
		"",
		""
	},
	HMStringItem {
		{6, 6}, {0, 0, 0, 0},
		"Click here to go further. You can do it the from the keyboard by pressing the space key",
		"",
		"",
		""
	},
	HMStringItem {
		{6, 6}, {0, 0, 0, 0},
		"Click here to go to the next thread. You can do it the from the keyboard by pressing right-arrow key.",
		"Click here to go to the next thread.\nIs dimmed because you are viewing the last selected thread",
		"",
		""
	},
}};
