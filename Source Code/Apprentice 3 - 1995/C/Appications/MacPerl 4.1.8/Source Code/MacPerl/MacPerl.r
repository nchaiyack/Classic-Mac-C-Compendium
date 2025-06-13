/*********************************************************************
Project	:	MacPerl			-	Real Perl Application
File		:	MacPerl.r		-	User interface related resources
Authors	:	Matthias Neeracher & Tim Endres

A lot of this code is borrowed from 7Edit written by
Apple Developer Support UK

Language	:	MPW C

$Log: MacPerl.r,v $
Revision 1.1  1994/03/22  00:08:05  neeri
Initial revision

Revision 0.17  1994/01/16  00:00:00  neeri
4.1.2

Revision 0.16  1994/01/12  00:00:00  neeri
4.1.1

Revision 0.15  1993/12/28  00:00:00  neeri
4.1.1b3

Revision 0.14  1993/12/20  00:00:00  neeri
4.1.1b2

Revision 0.13  1993/12/15  00:00:00  neeri
4.1.1b1

Revision 0.12  1993/10/24  00:00:00  neeri
4.1.0

Revision 0.11  1993/10/18  00:00:00  neeri
b6

Revision 0.10  1993/10/13  00:00:00  neeri
b5

Revision 0.9  1993/10/11  00:00:00  neeri
b4

Revision 0.8  1993/09/19  00:00:00  neeri
Runtime

Revision 0.7  1993/09/08  00:00:00  neeri
b3

Revision 0.6  1993/08/27  00:00:00  neeri
Format…

Revision 0.5  1993/08/17  00:00:00  neeri
Preferences…

Revision 0.4  1993/08/15  00:00:00  neeri
Credits

Revision 0.3  1993/07/13  00:00:00  neeri
Options dialog

Revision 0.2  1993/05/31  00:00:00  neeri
Support Console Windows

Revision 0.1  1993/05/29  00:00:00  neeri
Compiles correctly

*********************************************************************/

#define SystemSevenOrLater 1

#include "Types.r"
#include "SysTypes.r"
#include "BalloonTypes.r"
#include "AEUserTermTypes.r"
#include "AERegistry.r"
#include "AEObjects.r"

#include "MPRsrc.h"

include "Mercutio.rsrc";

#ifndef RUNTIME

include ":MP:FontLDEF.rsrc";

include "MacPerl.rsrc" 'BNDL'(128);
include "MacPerl.rsrc" 'McPL'(0);
include "MacPerl.rsrc" 'ICN#'(128);
include "MacPerl.rsrc" 'icl4'(128);
include "MacPerl.rsrc" 'icl8'(128);
include "MacPerl.rsrc" 'ics#'(128);
include "MacPerl.rsrc" 'BNDL'(129) 	as 'MrPB'(128);
include "MacPerl.rsrc" 'MrPL'(0);
include "MacPerl.rsrc" 'ICN#'(132) 	as 'MrPI'(128);
include "MacPerl.rsrc" 'icl4'(132) 	as 'MrP4'(128);
include "MacPerl.rsrc" 'icl8'(132) 	as 'MrP8'(128);
include "MacPerl.rsrc" 'ics#'(132) 	as 'MrP#'(128);
include "MacPerl.rsrc" 'ALRT'(4096)	as 'MrPA'(4096);
include "MacPerl.rsrc" 'DITL'(4096)	as 'MrPD'(4096);
include "MacPerl.rsrc" 'FREF'(135);
include "MacPerl.rsrc" 'ICN#'(134);
include "MacPerl.rsrc" 'icl4'(134);
include "MacPerl.rsrc" 'icl8'(134);
include "MacPerl.rsrc" 'ics#'(134);
#else

include ":RT:FontLDEF.rsrc";

include "MacPerl.rsrc" 'BNDL'(129) as 'BNDL'(128);
include "MacPerl.rsrc" 'MrPL'(0);
include "MacPerl.rsrc" 'ICN#'(132) as 'ICN#'(128);
include "MacPerl.rsrc" 'icl4'(132) as 'icl4'(128);
include "MacPerl.rsrc" 'icl8'(132) as 'icl8'(128);
include "MacPerl.rsrc" 'ics#'(132) as 'ics#'(128);
#endif
include "MacPerl.rsrc" 'DITL'(258);
include "MacPerl.rsrc" 'DLOG'(258);
include "MacPerl.rsrc" 'FOND'(32268);
include "MacPerl.rsrc" 'NFNT'(32268);
include "MacPerl.rsrc" 'FREF'(128);
include "MacPerl.rsrc" 'FREF'(129);
include "MacPerl.rsrc" 'FREF'(130);
include "MacPerl.rsrc" 'FREF'(131);
include "MacPerl.rsrc" 'FREF'(132);
include "MacPerl.rsrc" 'FREF'(133);
include "MacPerl.rsrc" 'FREF'(134);
include "MacPerl.rsrc" 'ICN#'(129);
include "MacPerl.rsrc" 'ICN#'(130);
include "MacPerl.rsrc" 'ICN#'(131);
include "MacPerl.rsrc" 'ICN#'(385);
include "MacPerl.rsrc" 'ICN#'(386);
include "MacPerl.rsrc" 'ICN#'(387);
include "MacPerl.rsrc" 'PICT'(128);
include "MacPerl.rsrc" 'icl4'(129);
include "MacPerl.rsrc" 'icl4'(130);
include "MacPerl.rsrc" 'icl4'(131);
include "MacPerl.rsrc" 'icl4'(385);
include "MacPerl.rsrc" 'icl4'(386);
include "MacPerl.rsrc" 'icl4'(387);
include "MacPerl.rsrc" 'icl8'(129);
include "MacPerl.rsrc" 'icl8'(130);
include "MacPerl.rsrc" 'icl8'(131);
include "MacPerl.rsrc" 'icl8'(385);
include "MacPerl.rsrc" 'icl8'(386);
include "MacPerl.rsrc" 'icl8'(387);
include "MacPerl.rsrc" 'icm#'(256);
include "MacPerl.rsrc" 'icm#'(257);
include "MacPerl.rsrc" 'icm#'(264);
include "MacPerl.rsrc" 'icm#'(265);
include "MacPerl.rsrc" 'icm#'(266);

#define GUSI_PREF_VERSION '0150'

#include "GUSI.r"

include "Perl.rsrc";

resource 'GU∑I' (GUSIRsrcID) {
	text, mpw, noAutoSpin, useChdir, approxStat, 
	noTCPDaemon, noUDPDaemon, 
	hasConsole,
	{};
};


#ifdef APPNAME	// only include 'cfrg' in native PowerPC apps

#include "CodeFragmentTypes.r"

resource 'cfrg' (0) {
	{
		kPowerPC,
		kFullLib,
		kNoVersionNum,kNoVersionNum,
		0,0,
		kIsApp,kOnDiskFlat,kZeroOffset,kWholeFork,
		APPNAME	// must be defined on Rez command line with -d option
	}
};

#endif

#ifdef OSL
include OSL 'proc';
#endif

#ifndef RUNTIME

resource 'vers' (1) {
	0x04, 0x18, release, 0x00, verUS,
	"4.1.8",
	"MacPerl - ported by Matthias Neeracher and Tim Endres."
	};

resource 'vers' (2) {
	0x04, 0x18, release, 0x00, verUS,
	"4.1.8",
	"Perl 4.1.8 (19Feb95)"
	};

resource 'STR#' (CreditID) {
	{
		"Thanks to: Charles Albrecht",
		"Kevin Altis",
		"Roberto Avanzi",
		"Peter Van Avermaet",
		"Joaquim Baptista",
		"Joe Bearly",
		"Benjamin Beberness",
		"David Blank-Edelman",
		"James Burgess",
		"Alun Carr",
		"Henry Churchyard",
		"Scott Collins",
		"Steve Dorner",
		"Torsten Ekedahl",
		"Barry Eynon",
		"Gus Fernandez",
		"Glenn Fleishman",
		"David Friedlander",
		"Michael Greenspon",
		"Sal Gurnani",
		"David Hansen",
		"Steve Hampson",
		"Brad Hanson",
		"C. Joe Holmes",
		"John Kamp",
		"Jim Kateley",
		"Pete Keleher",
		"Thomas Kimpton",
		"Gary LaVoy",
		"Thomas Lenggenhager",
		"Peter Lewis",
		"John Liberty",
		"Jann Linder",
		"Roger Linder",
		"Mike Meckler",
		"Chris Myers",
		"Peter Möller",
		"Bill Moore",
		"Asa Packer",
		"Mark Pease",
		"Brad Pickering",
		"Tom Pollard",
		"Simon Poole",
		"Malcolm Pradhan",
		"Alasdair Rawsthorne",
		"Charlie Reiman",
		"King Rhoton",
		"Shimizu Shu",
		"Sandra Silcot",
		"Paul Snively",
		"Omar Souka",
		"Hannu Strang",
		"Dan Strnad",
		"Man Wei Tam",
		"James Tisdall",
		"Werner Uhrig",
		"Maki Watanabe",
		"Mike West",
		"Peter Whaite",
		"Forrest Whitcher",
		"Hal Wine",
		"Barry Wolman",
		"Michael Wu",
		"… and everybody whose name I might have forgotten.",
		"",
		"",
		"",
		"",
	}
};
#else
resource 'vers' (1) {
	0x04, 0x18, release, 0x00, verUS,
	"4.1.8",
	"MacPerl Runtime - ported by Matthias Neeracher and Tim Endres."
};

resource 'vers' (2) {
	0x04, 0x18, release, 0x00, verUS,
	"4.1.8",
	"MacPerl 4.1.8 (19Feb95)"
};

#endif

resource 'SIZE' (-1) {
	dontSaveScreen,
	acceptSuspendResumeEvents,
	enableOptionSwitch,
	canBackground,
	multiFinderAware,
	backgroundAndForeground,
	dontGetFrontClicks,
	ignoreChildDiedEvents,
	is32BitCompatible,
	isHighLevelEventAware,
	localAndRemoteHLEvents,
	reserved,
	reserved,
	reserved,
	reserved,
	reserved,
	1536 * 1024,
	1024 * 1024
};

#ifndef RUNTIME
type 'MrPS' as 'SIZE';

resource 'MrPS' (-1) {
	dontSaveScreen,
	acceptSuspendResumeEvents,
	enableOptionSwitch,
	canBackground,
	multiFinderAware,
	backgroundAndForeground,
	dontGetFrontClicks,
	ignoreChildDiedEvents,
	is32BitCompatible,
	isHighLevelEventAware,
	localAndRemoteHLEvents,
	reserved,
	reserved,
	reserved,
	reserved,
	reserved,
	65536,
	65536
};

#endif

/************************** Window templates **************************/

resource 'WIND' (WindowTemplates, "", purgeable) {
	{18, 48, 312, 488},
	zoomDocProc,
	invisible,
	goAway,
	0x0,
	Untitled, 
	noAutoCenter
};

resource 'WIND' (WindowTemplates+1, "", purgeable) {
	{18, 48, 312, 488},
	zoomDocProc,
	invisible,
	goAway,
	0x0,
	"MacPerl",
	noAutoCenter
};

resource 'WIND' (WindowTemplates+2, "", purgeable) {
	{18, 48, 312, 488},
	zoomDocProc,
	invisible,
	goAway,
	0x0,
	Untitled,
	noAutoCenter
};

/************************** MacPerl'Answer Dialog **************************/

resource 'DLOG' (2001)	{
	{  0,   0, 150, 400},
	dBoxProc,
	invisible,
	noGoAway,
	0,
	2001,
	"",
	alertPositionMainScreen
};

resource 'DITL' (2001)	{
	{
		{119,  307, 137,  387}, Button 		{ enabled, "^1"},
		{119, 8406, 137, 8486}, Button 		{ enabled, "^2"},
		{119, 8313, 137, 8393}, Button 		{ enabled, "^3"},
		{ 13,   23,  45,   55}, Icon			{disabled, 0	},
		{ 13,   78, 103,  387}, StaticText	{disabled, "^0"}
	}
};


resource 'DLOG' (2002)	{
	{  0,   0, 150, 400},
	dBoxProc,
	invisible,
	noGoAway,
	0,
	2002,
	"",
	alertPositionMainScreen
};

resource 'DITL' (2002)	{
	{
		{119,  307, 137,  387}, Button 		{ enabled, "^1"},
		{119,  214, 137,  294}, Button 		{ enabled, "^2"},
		{119, 8313, 137, 8393}, Button 		{ enabled, "^3"},
		{ 13,   23,  45,   55}, Icon			{disabled, 2	},
		{ 13,   78, 103,  387}, StaticText	{disabled, "^0"}
	}
};

resource 'DLOG' (2003)	{
	{  0,   0, 150, 400},
	dBoxProc,
	invisible,
	noGoAway,
	0,
	2003,
	"",
	alertPositionMainScreen
};

resource 'DITL' (2003)	{
	{
		{119, 307, 137, 387}, Button 		{ enabled, "B1"},
		{119, 214, 137, 294}, Button 		{ enabled, "B2"},
		{119, 121, 137, 201}, Button 		{ enabled, "B3"},
		{ 13,  23,  45,  55}, Icon			{disabled, 2	},
		{ 13,  78, 103, 387}, StaticText	{disabled, "Prompt"}
	}
};

/************************** MacPerl'Ask Dialogs **************************/

resource 'DLOG' (2010) {
	{0, 0, 104, 400},
	dBoxProc,
	invisible,
	noGoAway,
	'tmDI',
	2010,
	"",
	alertPositionMainScreen
};

resource 'DITL' (2010, "Ask", purgeable) {
	{
		{73, 307, 91, 387}, Button 			{ enabled, "OK"},
		{73, 214, 91, 294},	Button 			{ enabled, "Cancel"},
		{13,  13, 31, 387},	StaticText 		{disabled, "^0"},
		{44,  15, 60, 385}, EditText 			{disabled, ""}
	}
};

/************************** MacPerl'Pick Dialog **************************/

resource 'DLOG' (2020) {
	{38, 80, 245, 427},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	2020,
	"",
	alertPositionMainScreen
};

resource 'DITL' (2020) {
	{
		{178,  30, 198,  88},	Button 		{ enabled, "OK"},
		{178, 258, 198, 316},	Button 		{ enabled, "Cancel"},
		{  2,   2,  19, 373},	StaticText 	{disabled, "Prompt"},
		{ 19,   2, 168, 345},	UserItem 	{disabled}
	}
};

/************************** Error Dialog **************************/

resource 'ALRT' (ErrorAlert, "", purgeable) {
	{82, 104, 182, 366},
	ErrorAlert,
	{	OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent
	},
	alertPositionMainScreen
};

resource 'DITL' (ErrorAlert, "", purgeable) {
	{	{ 68, 180,  88, 240}, Button {enabled, "OK"},
		{  9,  57,  56, 240}, StaticText {enabled, "^0^1^2^3"},
		{  9,   7,  41,  39}, Icon {enabled, 1}
	}
};

/************************** Save Changes Dialog **************************/

resource 'ALRT' (SaveAlert, "", purgeable) {
	{86, 60, 190, 432},
	SaveAlert,
	{	OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent
	},
	alertPositionParentWindow
};

resource 'DITL' (SaveAlert) {
	{	{ 74, 303,  94, 362}, Button 		{ enabled, "Save"},
		{ 74,  65,  94, 150}, Button 		{ enabled, "Don’t Save"},
		{ 74, 231,  94, 290}, Button 		{ enabled, "Cancel"},
		{ 10,  65,  59, 363}, StaticText {disabled, "Save changes to “^0”?"},
		{ 10,  20,  42,  52}, Icon 		{disabled, 2},
		{  0,   0,   0,   0}, HelpItem 	{disabled, HMScanhdlg { SaveAlert } },
	}
};

/************************** Revert Dialog **************************/

resource 'ALRT' (RevertAlert, "", purgeable) {
	{86, 60, 190, 432},
	RevertAlert,
	{	OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent
	},
	alertPositionParentWindow
};

resource 'DITL' (RevertAlert) {
	{	{ 74, 303,  94, 362}, Button 		{ enabled, "Revert"},
		{ 74, 231,  94, 290}, Button 		{ enabled, "Cancel"},
		{ 10,  65,  59, 363}, StaticText {disabled, "Revert to the last saved version of “^0”?"},
		{ 10,  20,  42,  52}, Icon 		{disabled, 2},
		{  0,   0,   0,   0}, HelpItem 	{disabled, HMScanhdlg { RevertAlert } },
	}
};

/************************** Abort Dialog **************************/

resource 'ALRT' (AbortAlert, "", purgeable) {
	{86, 60, 190, 432},
	AbortAlert,
	{	OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent
	},
	alertPositionParentWindow
};

resource 'DITL' (AbortAlert) {
	{	{ 74, 303,  94, 362}, Button 		{ enabled, "Quit"},
		{ 74, 231,  94, 290}, Button 		{ enabled, "Cancel"},
		{ 10,  65,  59, 363}, StaticText {disabled, "Quit (and abort current Perl script)?"},
		{ 10,  20,  42,  52}, Icon 		{disabled, 2},
		{  0,   0,   0,   0}, HelpItem 	{disabled, HMScanhdlg { AbortAlert } },
	}
};

/************************** Format Dialog **************************/

resource 'DLOG' (FormatDialog, "", purgeable) {
	{68, 52, 245, 435},
	movableDBoxProc,
	invisible,
	goAway,
	0x0,
	FormatDialog,
	"Format",
	alertPositionParentWindow
};

resource 'DITL' (FormatDialog, purgeable) {
	{	{104, 268, 124, 341},	Button 		{ enabled, "OK"				},
		{135, 268, 156, 341},	Button 		{ enabled, "Cancel"			},
		{ 28,  11, 156, 170},	UserItem 	{ enabled						},
		{ 60, 181, 156, 219},	UserItem 	{ enabled						},
		{ 28, 233, 156, 234},	UserItem 	{disabled						},
		{104, 268, 124, 341},	UserItem 	{disabled						},
		{ 30, 183,  46, 217},	EditText 	{disabled, ""					},
		{ 42, 245,  62, 372},	CheckBox 	{ enabled, "Make Default"	},
		{  7,  15,  25,  53},	StaticText	{disabled, "Font"				},
		{  7, 184,  25, 217},	StaticText 	{disabled, "Size"				},
		{  0,   0,   0,   0}, 	HelpItem 	{ disabled, HMScanhdlg { FormatDialog } },
	}
};

/************************** Preferences Dialog **************************/

resource 'DLOG' (PrefDialog, "", purgeable) {
	{0, 0, 250, 400},
	movableDBoxProc,
	invisible,
	goAway,
	'tmDI',
	PrefDialog,
	"Preferences",
	alertPositionMainScreen
};

resource 'DITL' (PrefDialog, "", purgeable) {
	{	{  0,  28,  32,  60}, UserItem { enabled },
		{  0,  98,  32, 130}, UserItem { enabled },
		{  0, 168,  32, 200}, UserItem { enabled },
		{ 53,   0,  54, 400}, UserItem { disabled },
		{  0,   0,   0,   0}, HelpItem { disabled, HMScanhdlg { PrefDialog } },
		{ 20, 330,  40, 380}, Button   { enabled, "Done"},
		{ 20, 330,  40, 380}, UserItem { disabled }
	}
};

resource 'dctb' (PrefDialog, "", purgeable) {
	{
	}
};

resource 'STR#' (PrefDialog) {
	{	"Libraries",
		"Scripts",
		"Input",
		"Change Path:",
		"Add Path:"
	}
};

resource 'DITL' (PrefLibID, "", purgeable) {
	{	{ 60,  10, 220, 375}, UserItem { disabled },
		{225,  25, 245, 145}, Button	 { enabled, "Remove Path(s)" 	},
		{225, 255, 245, 375}, Button	 { enabled, "Add Path…"			}
	}
};

resource 'ALRT' (PrefLibDelID, "", purgeable) {
	{108, 158, 238, 476},
	PrefLibDelID,
	{	Cancel, visible, sound1,
		Cancel, visible, sound1,
		Cancel, visible, sound1,
		Cancel, visible, sound1
	},
	alertPositionParentWindow
};

resource 'DITL' (PrefLibDelID, "", purgeable) {
	{	{ 98, 227, 119, 292}, Button { enabled, "Delete"},
		{ 98,  27, 119,  92}, Button { enabled, "Cancel"},
		{  9,  63,  89, 294}, StaticText {disabled, "Are you sure you want to delete the selected paths ?"},
		{ 10,  11,  42,  43}, Icon { disabled, 1 }
	}
};

resource 'DITL' (PrefScriptID, "", purgeable) {
	{	{ 80,  25, 100,  80}, RadioButton { enabled, "Edit" 	},
		{100,  25, 120,  80}, RadioButton { enabled, "Run" 	},
		{140,  25, 160, 375}, CheckBox	 { enabled, "Check for #! line"},
		{ 93,  80, 113, 375}, StaticText  {disabled, "Scripts opened from Finder"}
	}
};

resource 'DITL' (PrefInputID, "", purgeable) {
	{	{ 80,  25, 100, 200}, CheckBox { enabled, "Enable inline input" 	},
	}
};

/************************** General Error Dialog **************************/

resource 'ALRT' (300, "Error Alert", purgeable) {
	{108, 158, 238, 476},
	300,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	},
	alertPositionMainScreen
};

resource 'DITL' (300, "", purgeable) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{98, 227, 119, 292},
		Button {
			enabled,
			"Oh well"
		},
		/* [2] */
		{9, 63, 89, 294},
		StaticText {
			enabled,
			"Sorry an error has occured in the area o"
			"f ^0. \nThe error code = ^1\n(^2)"
		},
		/* [3] */
		{10, 11, 42, 43},
		Icon {
			enabled,
			0
		}
	}
};

/************************** Printing Progress Dialog **************************/

resource 'DLOG' (1005, "printing...", purgeable) {
	{148, 157, 185, 354},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	1005,
	"printing…",
	alertPositionParentWindow
};

resource 'DITL' (1005, "printing...", purgeable) {
	{	/* array DITLarray: 1 elements */
		/* [1] */
		{10, 10, 27, 235},
		StaticText {
			disabled,
			"Type \0x11. to cancel printing"
		}
	}
};

/************************** 7.0 only alert **************************/

resource 'ALRT' (302, "7.0 Only Alert") {
	{50, 60, 192, 350},
	302,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	},
	noAutoCenter
};

resource 'DITL' (302) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{108, 213, 128, 273},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{11, 63, 79, 278},
		StaticText {
			disabled,
			"MacPerl requires System 7.0 or later to run."
		},
		/* [3] */
		{11, 12, 43, 44},
		Icon {
			enabled,
			0
		}
	}
};

/************************** Get Script Dialog **************************/

resource 'DLOG' (GetScriptDialog, purgeable) {
	{0, 0, 193, 344},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	512,
	"",
	alertPositionMainScreen
};

resource 'DITL' (GetScriptDialog, purgeable) {
	{	{135, 252, 155, 332},	Button 	{	enabled, "Run"						},
		{104, 252, 124, 332},	Button 	{	enabled,	"Cancel"					},
		{  0,   0,   0,   0},	HelpItem { disabled, HMScanhdlg { -6042 } },
		{  8, 235,  24, 337},	UserItem {	enabled								},
		{ 32, 252,  52, 332},	Button 	{	enabled, "Eject"					},
		{ 60, 252,  80, 332},	Button 	{	enabled, "Desktop"				},
		{ 29,  12, 159, 230},	UserItem {	enabled								},
		{  6,  12,  25, 230},	UserItem {	enabled								},
		{ 91, 251,  92, 333},	Picture  { disabled,	11							},
		{170,  12, 186, 110},	CheckBox	{  enabled, "Debug"					}
	}
};

/************************** Save Dialog **************************/

resource 'DLOG' (SaveScriptDialog, purgeable) {
	{30, 8, 260, 352},
	dBoxProc,
	invisible,
	noGoAway,
	'tmDI',
	SaveScriptDialog,
	"Put File",
	noAutoCenter 
};

resource 'DITL' (SaveScriptDialog, purgeable) {
	{	{161, 252, 181, 332}, Button 		{ enabled, "Save" 				},
		{130, 252, 150, 332}, Button 		{ enabled, "Cancel" 				},
		{  0,   0,   0,   0}, HelpItem 	{ disabled, HMScanhdlg {-6043}},
		{  8, 235,  24, 337}, UserItem 	{ enabled 							},
		{ 32, 252,  52, 332}, Button 		{ enabled, "Eject" 				},
		{ 60, 252,  80, 332}, Button 		{ enabled, "Desktop" 			},
		{ 29,  12, 127, 230}, UserItem 	{ enabled 							},
		{  6,  12,  25, 230}, UserItem 	{ enabled 							},
		{119, 250, 120, 334}, Picture 	{ disabled, 11						},
		{157,  15, 173, 227}, EditText 	{ enabled, "" 						},
		{136,  15, 152, 227}, StaticText { disabled, "Save as:" 			},
		{ 88, 252, 108, 332}, UserItem 	{ disabled 							},
		{187,  17, 206, 283}, Control 	{ enabled, SaveScriptDialog	},
	}
};

resource 'CNTL' (SaveScriptDialog, preload, purgeable) {
	{187,  17, 206, 283},		/*enclosing rectangle of control*/
	popupTitleLeftJust, 			/*title position*/
	visible, 						/*make control visible*/ 
	50, 								/*pixel width of title*/
	SaveScriptDialog, 			/*'MENU' resource ID*/
	popupMenuCDEFProc,			/*pop-up control definition ID*/ 
	0, 								/*reference value*/
	"Type:"							/*control title*/
};

resource 'MENU' (SaveScriptDialog) {
	SaveScriptDialog,
	textMenuProc,
	0x7FFFFFFF,
	enabled,
	"Type",
	{	"Plain Text", 				noIcon, noKey, noMark, plain,
		"System 6 & 7 Runtime", noIcon, noKey, noMark, plain,
		"System 7 Runtime", 		noIcon, noKey, noMark, plain
	}
};

/************************** No Perl Script Dialog **************************/

resource 'ALRT' (NoPerlAlert, "", purgeable) {
	{86, 40, 230, 472},
	NoPerlAlert,
	{	OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent
	},
	alertPositionParentWindow
};

resource 'DITL' (NoPerlAlert) {
	{	{114, 323, 134, 402}, Button 		{ enabled, "Proceed"},
		{114, 231, 134, 310}, Button 		{ enabled, "Abort"},
		{ 10,  65,  99, 403}, StaticText {disabled, 
			"I'm not sure “^0” is really a Perl script (the #! line is missing). "
			"Still want to proceed with trying to execute this script?"
													},
		{ 10,  20,  42,  52}, Icon 		{disabled, 2}
	}
};

/************************** File too bulky to open Dialog **************************/

resource 'ALRT' (ElvisAlert, "", purgeable) {
	{86, 40, 210, 452},
	ElvisAlert,
	{	OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent
	},
	alertPositionParentWindow
};

resource 'DITL' (ElvisAlert) {
	{	{ 94, 323, 114, 402}, Button 		{ enabled, "Save"},
		{ 94, 231, 114, 310}, Button 		{ enabled, "Cancel"},
		{ 10,  65,  79, 403}, StaticText {disabled, 
			"This file is too big to be edited in MacPerl. "
			"You may, however, save it as a MacPerl script or runtime."
													},
		{ 10,  20,  42,  52}, Icon 		{disabled, 2}
	}
};

/************************** Menus **************************/

resource 'MENU' (appleID) {
	appleID,
	1,
	0x7FFFFFFD,
	enabled,
	apple,
	{	/* array: 2 elements */
		/* [1] */
		"About MacPerl…", noIcon, noKey, noMark, plain,
		/* [2] */
		"-", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (fileID) {
	fileID,
	1,
	0b101101111011,
	enabled,
	"File",
	{	"New", 			noIcon, "N", 	noMark, plain,
		"Open…", 		noIcon, "O", 	noMark, plain,
		"-", 				noIcon, noKey, noMark, plain,
		"Close", 		noIcon, "W", 	noMark, plain,
		"Save", 			noIcon, "S", 	noMark, plain,
		"Save As…", 	noIcon, "S", 	noMark, extend,
		"Revert", 		noIcon, noKey, noMark, plain,
		"-", 				noIcon, noKey, noMark, plain,
		"Page Setup…", noIcon, noKey, noMark, plain,
		"Print…", 		noIcon, "P", 	noMark, plain,
		"-", 				noIcon, noKey, noMark, plain,
		"Quit", 			noIcon, "Q", 	noMark, plain
	}
};

resource 'MENU' (editID) {
	editID,
	1,
#ifndef RUNTIME
	0b101111010101111101,
#else
	0b10101111101,
#endif
	enabled,
	"Edit",
	{	"Undo", noIcon, "Z", noMark, plain,
		"-", noIcon, noKey, noMark, plain,
		"Cut", noIcon, "X", noMark, plain,
		"Copy", noIcon, "C", noMark, plain,
		"Paste", noIcon, "V", noMark, plain,
		"Clear", noIcon, noKey, noMark, plain,
		"Select All", noIcon, "A", noMark, plain,
#ifndef RUNTIME
		"-", noIcon, noKey, noMark, plain,
		"Jump to…", noIcon, "J", noMark, plain,		
#endif
		"-", noIcon, noKey, noMark, plain,
		"Format…", noIcon, "Y", noMark, plain,
#ifndef RUNTIME
		"-", noIcon, noKey, noMark, plain,
		"Create Publisher…", noIcon, noKey, noMark, plain,
		"Subscribe To…", noIcon, noKey, noMark, plain,
		"Section Options…", noIcon, noKey, noMark, plain,
		"Show Borders", noIcon, noKey, noMark, plain,
#endif
		"-", noIcon, noKey, noMark, plain,
		"Preferences…", noIcon, noKey, noMark, plain,
	}
};

resource 'MENU' (windowID, preload) {
	windowID,
	1,
	allEnabled,
	enabled,
	"Window",
	{
	}
};

resource 'MENU' (perlID, preload) {
	perlID,
	1,
	allEnabled,
	enabled,
	"Script",
	{	"Run Script…", 		noIcon, "R", noMark, plain,
		"Run Front Window", 	noIcon, "R", noMark, extend
	}
};

#ifndef RUNTIME

resource 'STR#' (fileID) {
	{
/*  1 */	"File Handling items.",

/*  2 */	"Creates a new file.",
/*  3 */	"Creates a new file.  To enable this item, choose Close from the File menu.",
/*  4 */	"Opens an existing file.",
/*  5 */	"Opens an existing file.  To enable this item, choose Close from the File menu.",
/*  6 */	"Closes the current file.",
/*  7 */	"Closes the current file.  A window must be open before this item is enabled.",
/*  8 */	"Saves the current file.",
/*  9 */	"Saves the current file.  The window must be titled before this is enabled.",
/* 10 */	"Saves the current file under a different name.",
/* 11 */	"Saves the current file under a different name.",
/* 12 */	"Reverts to the last saved version of the current file.",
/* 13 */	"Reverts to the last saved version of the current file.  Enabled when you make a change to the file",
/* 14 */	"Sets up paper size, orientation, and other printing options.",
/* 15 */	"Sets up paper size, orientation, and other printing options.",
/* 16 */	"Prints the text to the printer.",
/* 17 */	"Prints all or part of the current file.  To enable this item, choose New or Open from the File menu.",
/* 18 */	"Quit from MacPerl."
	}
};

resource 'hmnu' (fileID, "File") {
	HelpMgrVersion,			/* Help Version						*/
	0,								/* options								*/
	0,								/* theProc								*/
	0,								/* variant								*/
	HMSkipItem {				/* skip missing msg					*/
		},
	{
		/* File */
		HMStringResItem {
			fileID,1,				/* menu title message					*/
			fileID,1,				/* disabled title message				*/
			fileID,0,				/* reserved message						*/
			fileID,0					/* reserved message						*/
		},

		/* New */
		HMStringResItem {
			fileID,2,				/* menu item message					*/
			fileID,3,				/* disabled item message				*/
			fileID,0,				/* reserved message						*/
			fileID,0					/* reserved message						*/
		},

		/* Open */
		HMStringResItem {
			fileID,4,				/* menu item message					*/
			fileID,5,				/* disabled item message				*/
			fileID,0,				/* reserved message						*/
			fileID,0					/* reserved message						*/
		},

		/* Disabled Line */
		HMSkipItem {				/* skip missing msg						*/
		},

		/* Close */
		HMStringResItem {
			fileID,6,				/* menu item message					*/
			fileID,7,				/* disabled item message				*/
			fileID,0,				/* reserved message						*/
			fileID,0					/* reserved message						*/
		},

		/* Save */
		HMStringResItem {
			fileID,8,				/* menu item message					*/
			fileID,9,				/* disabled item message				*/
			fileID,0,				/* reserved message						*/
			fileID,0					/* reserved message						*/
		},

		/* Save As… */
		HMStringResItem {
			fileID,10,				/* menu item message					*/
			fileID,11,				/* disabled item message				*/
			fileID,0,				/* reserved message						*/
			fileID,0					/* reserved message						*/
		},

		/* Revert */
		HMStringResItem {
			fileID,12,				/* menu item message					*/
			fileID,13,				/* disabled item message				*/
			fileID,0,				/* reserved message						*/
			fileID,0					/* reserved message						*/
		},

		/* Disabled Line */
		HMSkipItem {				/* skip missing msg						*/
		},

		/* Page Setup… */
		HMStringResItem {
			fileID,14,				/* menu item message					*/
			fileID,15,				/* disabled item message				*/
			fileID,0,				/* reserved message						*/
			fileID,0					/* reserved message						*/
		},

		/* Print */
		HMStringResItem {
			fileID,16,				/* menu item message					*/
			fileID,17,				/* disabled item message				*/
			fileID,0,				/* reserved message						*/
			fileID,0					/* reserved message						*/
		},

		/* Disabled Line */
		HMSkipItem {				/* skip missing msg						*/
		},

		/* Quit */
		HMStringResItem {
			fileID,18,				/* menu item message					*/
			fileID,0,				/* disabled item message				*/
			fileID,0,				/* reserved message						*/
			fileID,0					/* reserved message						*/
		}
	}
};

resource 'STR#' (editID) {
	{
/*  1 */	"Editing Items.",

/*  2 */	"Restore the document to the state it was before the last action.",
/*  3 */	"Restore the document to the state it was before the last action. (Not currently implemented)",
/*  4 */	"Delete the current text selection and copy it to the clipboard.",
/*  5 */	"Delete the current text selection and copy it to the clipboard. Select some text to enable this item.",
/*  6 */	"Copy the current text selection to the clipboard.",
/*  7 */	"Copy the current text selection to the clipboard. Select some text to enable this item.",
/*  8 */	"Take the text from the clipboard and add it at the current insertion point.",
/*  9 */	"Take the text from the clipboard and add it at the current insertion point. This item is enabled when there is some text on the clipboard",
/* 10 */	"Delete the currently selected text.",
/* 11 */	"Delete the currently selected text.  Select some text to enable this item.",
/* 12 */	"Select all the text in the document.",
/* 13 */	"Select all the text in this document.  This item is enabled when a window is open.",
/* 14 */ "Set the font and size of the document.",
/* 15 */	"Display the Create Publisher dialog to publish the selected text.",
/* 16 */	"Publish selected text. Select some text in a document window to enable this item.",
/* 17 */	"Put up the Subscriber dialog Subscribe to a Published section of text.",
/* 18 */	"Put up the Subscriber dialog Subscribe to a Published section of text.  Enabled in a document window when no publisher is selected.",
/* 19 */	"Display the Section Options dialog for a Publisher or Subscriber.",
/* 20 */	"Display the Section Options dialog for a Publisher or Subscriber.  Select a Publisher or Subscriber to enable this item.",
/* 21 */	"Show/Hide the all the Publisher or Subscriber borders.",
/* 22 */	"Show/Hide the all the Publisher or Subscriber borders. Enabled in a document window.",
/* 23 */ "Edit Preferences.",
/* 24 */ "Jump to File (and Line) given by current selection.",
/* 25 */ "Jump to File (and Line) given by current selection. Select a file name and/or line number to enable this item."
	}
};



resource 'hmnu' (editID, "Edit") {
	HelpMgrVersion,					/* Help Version							*/
	0,										/* options									*/
	0,										/* theProc									*/
	0,										/* variant									*/
	HMSkipItem {						/* skip missing msg						*/
		},
	{
		/* edit */
		HMStringResItem {
			editID,1,				/* menu title message					*/
			editID,1,				/* disabled title message				*/
			editID,0,				/* reserved message						*/
			editID,0					/* reserved message						*/
		},

		/* Undo */
		HMStringResItem {
			editID,2,				/* menu item message					*/
			editID,3,				/* disabled item message				*/
			editID,0,				/* reserved message						*/
			editID,0					/* reserved message						*/
		},

		/* Disabled Item */
		HMSkipItem {				/* skip missing msg						*/
		},

		/* Cut */
		HMStringResItem {
			editID,4,				/* menu item message					*/
			editID,5,				/* disabled item message				*/
			editID,0,				/* reserved message						*/
			editID,0					/* reserved message						*/
		},

		/* Copy */
		HMStringResItem {
			editID,6,				/* menu item message					*/
			editID,7,				/* disabled item message				*/
			editID,0,				/* reserved message						*/
			editID,0					/* reserved message						*/
		},

		/* Paste */
		HMStringResItem {
			editID,8,				/* menu item message					*/
			editID,9,				/* disabled item message				*/
			editID,0,				/* reserved message						*/
			editID,0					/* reserved message						*/
		},

		/* Clear */
		HMStringResItem {
			editID,10,				/* menu item message					*/
			editID,11,				/* disabled item message				*/
			editID,0,				/* reserved message						*/
			editID,0					/* reserved message						*/
		},

		/* Select All */
		HMStringResItem {
			editID,12,				/* menu item message					*/
			editID,13,				/* disabled item message				*/
			editID,0,				/* reserved message						*/
			editID,0					/* reserved message						*/
		},

		/* Disabled Line */
		HMSkipItem {				/* skip missing msg						*/
		},

		/* Jump To*/
		HMStringResItem {
			editID,24,				/* menu item message						*/
			editID,25,				/* disabled item message				*/
			editID,0,				/* reserved message						*/
			editID,0					/* reserved message						*/
		},

		/* Disabled Line */
		HMSkipItem {				/* skip missing msg						*/
		},

		/* Format */
		HMStringResItem {
			editID,14,				/* menu item message					*/
			editID,0,				/* disabled item message				*/
			editID,0,				/* reserved message						*/
			editID,0					/* reserved message						*/
		},
		
		/* Disabled Line */
		HMSkipItem {				/* skip missing msg						*/
		},

		/* Create Publisher*/
		HMStringResItem {
			editID,15,				/* menu item message					*/
			editID,16,				/* disabled item message				*/
			editID,0,				/* reserved message						*/
			editID,0					/* reserved message						*/
		},

		/* Subscribe To..*/
		HMStringResItem {
			editID,17,				/* menu item message					*/
			editID,18,				/* disabled item message				*/
			editID,0,				/* reserved message						*/
			editID,0					/* reserved message						*/
		},

		/* Section Options*/
		HMStringResItem {
			editID,19,				/* menu item message					*/
			editID,20,				/* disabled item message				*/
			editID,0,				/* reserved message						*/
			editID,0					/* reserved message						*/
		},

		/* Show/Hide borders*/
		HMStringResItem {
			editID,21,				/* menu item message						*/
			editID,22,				/* disabled item message				*/
			editID,0,				/* reserved message						*/
			editID,0					/* reserved message						*/
		},
		
		/* Disabled Item */
		HMSkipItem {				/* skip missing msg						*/
		},

		/* Preferences */
		HMStringResItem {
			editID,23,				/* menu item message					*/
			editID,0,				/* disabled item message				*/
			editID,0,				/* reserved message						*/
			editID,0					/* reserved message						*/
		},
   }
};

resource 'STR#' (perlID) {
	{
/*  1 */	"Running Scripts",

/*  2 */	"Runs a Script.",

/*  3 */ "Runs the script in the front window.",

/*  4 */ "Runs the script in the front window. Enabled if a document window is in front.",

	}
};

resource 'hmnu' (perlID, "Script") {
	HelpMgrVersion,				/* Help Version							*/
	0,									/* options								*/
	0,									/* theProc								*/
	0,									/* variant								*/
	HMSkipItem {					/* skip missing msg						*/
		},
	{
		/* Script */
		HMStringResItem {
			perlID,1,				/* menu title message					*/
			perlID,1,				/* disabled title message				*/
			perlID,0,				/* reserved message						*/
			perlID,0					/* reserved message						*/
		},

		/* Run */
		HMStringResItem {
			perlID,2,				/* menu item message					*/
			perlID,0,				/* disabled item message				*/
			perlID,0,				/* reserved message						*/
			perlID,0					/* reserved message						*/
		},

		/* Run Front*/
		HMStringResItem {
			perlID,3,				/* menu item message						*/
			perlID,4,				/* disabled item message				*/
			perlID,0,				/* reserved message						*/
			perlID,0					/* reserved message						*/
		},
   }
};

resource 'STR ' (helpID) {
	"Perl Help/H"
};


resource 'STR#' (256) {
	{
		"Indicates that this window is a modifiable text document.",
		"Indicates that this window is a read-only text document.",
		"Indicates that this window is a console window that is currently expecting input.",
		"Indicates that this window is a read-only console window.",
		"Indicates that this window is a console window that is not currently expecting input.",
	}
};

#include "MPTerminology.r"
#include "MPBalloons.r"

#endif
