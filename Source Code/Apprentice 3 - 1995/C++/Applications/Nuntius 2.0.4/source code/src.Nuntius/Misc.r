// Copyright � 1992-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// Misc.r
/* ==============================   Includes   ================================= */

#include "SysTypes.r"
#include "Types.r"

#ifndef _MacAppTypes_
#include "MacAppTypes.r"
#endif

#ifndef __RSRCGLOBALS__
#include "RsrcGlobals.h"
#endif

#ifndef __MENUGLOBALS__
#include "MenuGlobals.h"
#endif

#include "Menus.r"
#include "Errors.r"
#include "Views.r"
include "MacAppRsrc.rsrc";

#define VERSION_STUFF 	\
	0x2,				\
	0x01,				\
	final,				\
	1,					\
	verUS,				\
	"2.0.3",			\
	"Nuntius v2.0.3 by Peter Speck (speck@dat.ruc.dk)"

resource 'vers' (1, preload) {  // preloaded as it is used in FatalError()
	VERSION_STUFF
};

//resource 'vers' (2, preload) {  // preloaded as it is used in FatalError()
//	VERSION_STUFF
//};

resource 'mem!' (1024, purgeable)
{
	0, 0, 0, 24 * 1024, 0
};

resource 'TxSt' (kAboutBoxVersionTS, "kAboutBoxVersionTS") {
	tsPlain, 10, black, "Times"
};

resource 'kind' (128) {
	kSignature, 0, {
		kPrefsFileType, "Nuntius Preferences document",
		kGroupDBFileType, "Nuntius group database document",
		kArticleStatusFileType, "Nuntius group status document",
		kGroupListDocFileType, "Nuntius group list document",
		kGroupTreeDocFileType, "Nuntius list of all groups document"
}};

#define SIZE_STUFF \
	reserved, \
	acceptSuspendResumeEvents, \
	reserved, \
	canBackground, \
	doesActivateOnFGSwitch, \
	backgroundAndForeground, \
	dontGetFrontClicks, \
	ignoreAppDiedEvents, \
	is32BitCompatible, \
	isHighLevelEventAware, \
	localAndRemoteHLEvents, \
	isStationeryAware,				 \
	reserved, \
	reserved, \
	reserved, \
	reserved, \
	3000 * 1024, \
	1900 * 1024

resource 'SIZE' (-1) {
	SIZE_STUFF
};

resource 'SIZE' (0) {
	SIZE_STUFF
};

resource 'SIZE' (1) {
	SIZE_STUFF
};

#define macroStandardHeaders			\
	"Path: �dotname�!�username�";		\
	"Newsgroups: �newsgroups�";			\
	"Subject: �subject�";				\
	"From: �realname� <�email�>";		\
	"X-Newsreader: Nuntius �nuntiusversion�_�cputype�";	\
	"X-XXMessage-ID: �message-id�";		\
	"X-XXDate: �dayofweek�, �day� �month� �year� �gmt-hour�:�minute�:�second� GMT";	\
	"References: �references�";			\
	"Distribution: �distribution�";		\
	"Organization: �organization�"
// end macro

resource 'STR#' (kCancelArticleHeaderStrings, "kCancelArticleHeaderStrings", purgeable) {{
	"Path: �dotname�!�username�";
	"Newsgroups: control";
	"Subject: cancel �references�";
	"From: �realname� <�email�>";
	"X-Newsreader: Nuntius �nuntiusversion�";
	"X-XXMessage-ID: �message-id�";
	"X-XXDate: �dayofweek�, �day� �month� �year� �gmt-hour�:�minute�:�second� GMT";
	"Control: cancel �references�";
	"Distribution: �distribution�";
	"Organization: �organization�";
	""; // header-body separator (empty line)
	"cancel �references�";
}};

type 'Hder' as 'STR#';

resource 'Hder' (13000, "US-ASCII", purgeable) {{
	macroStandardHeaders;
	""; // header-body separator (empty line)
}};

resource 'Hder' (13001, "DK-ASCII", purgeable) {{
	macroStandardHeaders;
	""; // header-body separator (empty line)
	// ignore that it really is DK-ASCII,
	// a real Dane can read "r|dgr|d med fl|de" (and pronounce it)
}};

resource 'Hder' (13002, "S-ASCII", purgeable) {{
	macroStandardHeaders;
	""; // header-body separator (empty line)
}};

resource 'Hder' (13005, "ISO 8859-1", purgeable) {{
	macroStandardHeaders;
	"Mime-Version: 1.0";
	"Content-Type: text/plain; charset=ISO-8859-1";
	"Content-Transfer-Encoding: 8bit";
	""; // header-body separator (empty line)
}};
resource 'Hder' (13009, "Transparent", purgeable) {{
	macroStandardHeaders;
	""; // header-body separator (empty line)
}};
