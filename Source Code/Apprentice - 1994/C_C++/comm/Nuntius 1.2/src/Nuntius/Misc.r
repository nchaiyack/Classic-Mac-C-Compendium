// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
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

#if qDebug
include "Debug.rsrc";
#endif
include "MacApp.rsrc";
include "Dialog.rsrc";
include "Printing.rsrc";

#include "Menus.r"
#include "Errors.r"
#include "Views.r"

resource 'mem!' (1024, purgeable)
{
	0, 24 * 1024, 0
};

#define VERSION_STUFF 	\
	0x1,				\
	0x20,				\
	final,				\
	1,					\
	verUS,				\
	"Version 1.2",		\
	"Nuntius v1.2 by Peter Speck (speck@dat.ruc.dk)"

resource 'vers' (1, preload) {  // preloaded as it is used in FatalError()
	VERSION_STUFF
};

resource 'vers' (2, preload) {  // preloaded as it is used in FatalError()
	VERSION_STUFF
};

resource 'TxSt' (kAboutBoxVersionTS, "kAboutBoxVersionTS") {
	tsPlain, 10, black, "Times"
};

resource 'res!' (1100, "Nuntius, resident segments", purgeable) {{
	"MacApp&Nuntius";
	"GInit";
}};
