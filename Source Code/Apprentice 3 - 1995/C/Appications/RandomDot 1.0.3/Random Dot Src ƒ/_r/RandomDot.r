/* RandomDot.r
	by David Phillip Oster October 1994 oster@netcom.com
	for:
	Stuart Inglis singlis@waikato.ac.nz
	Department of Computer Science
	University of Waikato, Hamilton, New Zealand
*/
#include "Types.r"
#include "SysTypes.r"
#include "BalloonTypes.r"
#include "::_h:RandomDotRes.h"

/* used to offset STR#s for help resources.
 */
#define hMenuBase	2000
#define hDlogBase	3000

include "RandomDot.RESOURCES";

#define kPopUpProc		1008

/*	*** CNTLs
 */
resource 'CNTL' (128, purgeable, preload) {	/* a convenient way to make a scrollbar. */
	{0, -116, 40, -100},
	0,
	-1,
	0,
	0,
	scrollBarProc,
	0,
	""
};

/* 128k ROM and newer window template
 */
resource 'WIND' (rWin, purgeable) {
	{40, 40, 300, 472},
	zoomDocProc,
	invisible,
	-1,
	0x0,
	untitled
};

resource 'DLOG' (rAbout, purgeable) {
	{80, 66, 282, 476},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	rAbout,
	"About"
};


resource 'DITL' (rAbout, purgeable) {{
	/* [1] */	{153, 326, 173, 384},	Button {	enabled,	"OK"	},
	/* [2] */	{124, 326, 144, 384},	Button {	enabled,	"Help"	},
	/* [3] */	{12, 83,  49, 376},		StaticText {enabled,	"Random Dot: Version 1.0.3. A program to make random dot autostereograms."	},
	/* [4] */	{49, 13, 112, 376},		StaticText {enabled,	"Copyright © by David Phillip Oster, 1994, \nAll Rights Reserved. \nSee “Random Dot Help” on the “?” menu for more information."	},
	/* [5] */	{117, 13, 183, 313},	StaticText {enabled,	"See also “Displaying 3-D Images: Algorithms for Single-Image Random Dot Stereograms” Harold W. Thimbleby, Stuart Inglis, & Ian H. Witten, Computer October 1994"	},
	/* [6] */	{13, 30, 45, 62},		Icon	   {enabled, 128 }
}};


resource 'dctb' (rAbout, purgeable) {{
	wContentColor, 48059, 48059, 48059,
	wFrameColor, 0, 0, 0,
	wTextColor, 0, 0, 0,
	wHiliteColor, 0, 0, 0,
	wTitleBarColor, 65535, 65535, 65535
}};

resource 'ALRT' (rError, "Error", purgeable) {
	{56, 64, 184, 438},
	rError,
	{	OK, visible, sound1,
		OK, visible, sound1,
		OK, visible, sound1,
		OK, visible, sound1	
}};

resource 'DITL' (rError, "Error", purgeable) {{
	{98, 300, 118, 360},	Button {	enabled,	"OK"	},
	{9, 55, 93, 360},	StaticText {	disabled,	"^0  (^1)"	},
	{9, 12, 41, 44},	Icon {		disabled,		0	}
}};

resource 'DLOG' (rProgress, "Progress", purgeable) {
	{150, 120, 233, 426},
	movableDBoxProc,
	invisible,
	noGoAway,
	0x1,
	rProgress,
	""
};

resource 'DITL' (rProgress, "Progress", purgeable) {{
	{25, 16, 43, 290},	UserItem	{disabled},
	{52, 222, 73, 287},	Button 		{enabled,"Cancel"},
	{7, 16, 20, 293},	StaticText	{disabled,""}
}};

resource 'dctb' (rProgress, purgeable) {{
}};

/* change to dBoxProc before we ship.
 */
resource 'DLOG' (rHelp, "Help", purgeable) {
	{30, 96, 340, 402},
	dBoxProc,
	invisible,
	noGoAway,
	0x1,
	rHelp,
	"Help"
};

resource 'DITL' (rHelp, "Help", purgeable) {{
	{281, 231, 302, 296},	Button 		{enabled,"OK"},
	{4, 7, 274, 297},	UserItem	{disabled}
}};


#define kAllItems	0x7FFFFFFF
#define kItem1		(1 << (1 - 1))
#define kItem2		(1 << (2 - 1))
#define kItem3		(1 << (3 - 1))
#define kItem4		(1 << (4 - 1))
#define kItem5		(1 << (5 - 1))
#define kItem6		(1 << (6 - 1))
#define kItem7		(1 << (7 - 1))
#define kItem8		(1 << (8 - 1))
#define kItem9		(1 << (9 - 1))

resource 'MENU' (kAppleMenu, "kAppleMenu") {
	kAppleMenu,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	apple,
	{
		"About Random Dot…", noIcon, noKey, noMark, plain,
		"-", noIcon, noKey, noMark, plain
}};

#define kAppleHmnu hMenuBase+kAppleMenu
resource 'STR#' (kAppleHmnu, purgeable) {{
	/* 1 */	"The Apple menu contains commands for bringing up items in the Apple Menu Item folder in your System Folder.",
	/* 2 */	"This brings up a dialog box with information about this program."
}};

/* The 'hmnu' resource connects menu items to STR# items.
 */
resource 'hmnu' (kAppleMenu, purgeable) {
	HelpMgrVersion,
	hmDefaultOptions,
	0,	/* balloon def id */
	0,	/* var code, hang right */
	HMSkipItem {},	/* default item */
	{
	HMStringResItem {kAppleHmnu, 1, kAppleHmnu, 1, kAppleHmnu, 1, kAppleHmnu, 1 },
	HMStringResItem {kAppleHmnu, 2, kAppleHmnu, 2, kAppleHmnu, 2, kAppleHmnu, 2 }
}};

resource 'MENU' (kFileMenu, "kFileMenu") {
	kFileMenu,
	textMenuProc,
	kAllItems - (kItem3|kItem6|kItem9 | (kItem2|kItem4|kItem5|kItem8)),
	enabled,
	"File",
	{
		"Open…", noIcon, "O", noMark, plain,
		"Close", noIcon, "W", noMark, plain,
		"-", noIcon, noKey, noMark, plain,
		"Save", noIcon, "S", noMark, plain,
		"Save As…", noIcon, noKey, noMark, plain,
		"-", noIcon, noKey, noMark, plain,
		"Page Setup…", noIcon, noKey, noMark, plain,
		"Print", noIcon, "P", noMark, plain,
		"-", noIcon, noKey, noMark, plain,
		"Quit", noIcon, "Q", noMark, plain
}};

#define kFileHmnu hMenuBase+kFileMenu
resource 'STR#' (kFileHmnu, purgeable) {{
	/* 1 */	"The File menu contains commands for the general operation of this program.",
	/* 2 */	"Open\n\nOpens a PICT or a PGM file.",
	/* 3 */	"Close\n\nCloses the frontmost window.",
	/* 4 */	"Save\n\nSaves the frontmost window under the most recently saved name.",
	/* 5 */	"Save As\n\nSaves the frontmost window under a new saved name.",
	/* 6 */	"Page Setup\n\nbrings up a dialog box of for setting up the printer.",
	/* 7 */	"Print\n\nbrings up a dialog box of printing.",
	/* 8 */	"Quit\n\nQuits this program."
}};

/* The 'hmnu' resource connects menu items to STR# items.
 */
resource 'hmnu' (kFileMenu, purgeable) {
	HelpMgrVersion,
	hmDefaultOptions,
	0,	/* balloon def id */
	0,	/* var code, hang right */
	HMSkipItem {},	/* default item */
	{
	HMStringResItem {kFileHmnu, 1, kFileHmnu, 1, kFileHmnu, 1, kFileHmnu, 1 },
	HMStringResItem {kFileHmnu, 2, kFileHmnu, 2, kFileHmnu, 2, kFileHmnu, 2 },
	HMStringResItem {kFileHmnu, 3, kFileHmnu, 3, kFileHmnu, 3, kFileHmnu, 3 },
	HMSkipItem {},
	HMStringResItem {kFileHmnu, 4, kFileHmnu, 4, kFileHmnu, 4, kFileHmnu, 4 },
	HMStringResItem {kFileHmnu, 5, kFileHmnu, 5, kFileHmnu, 5, kFileHmnu, 5 },
	HMSkipItem {},
	HMStringResItem {kFileHmnu, 6, kFileHmnu, 6, kFileHmnu, 6, kFileHmnu, 6 },
	HMStringResItem {kFileHmnu, 7, kFileHmnu, 7, kFileHmnu, 7, kFileHmnu, 7 },
	HMSkipItem {},
	HMStringResItem {kFileHmnu, 8, kFileHmnu, 8, kFileHmnu, 8, kFileHmnu, 8 }
}};

resource 'MENU' (kEditMenu, "kEditMenu") {
	kEditMenu,
	textMenuProc,
	kAllItems - (kItem2 |(kItem1|kItem3|kItem4|kItem5|kItem6)),
	enabled,
	"Edit",
	{	/* array: 6 elements */
		"Undo", noIcon, "Z", noMark, plain,
		"-", noIcon, noKey, noMark, plain,
		"Cut", noIcon, "X", noMark, plain,
		"Copy", noIcon, "C", noMark, plain,
		"Paste", noIcon, "V", noMark, plain,
		"Clear", noIcon, noKey, noMark, plain
	}
};

#define kEditHmnu hMenuBase+kEditMenu
resource 'STR#' (kEditHmnu, purgeable) {{
	/* 1 */	"The Edit menu has commands relating to editing text. It is here only for compatibility.",
	/* 2 */	"Undo\n\nlets you undo your changes. Not implemented in this program.",
	/* 3 */	"Cut\n\nlets you move your selection to the clipboard. Not implemented in this program.",
	/* 4 */	"Copy\n\nlets you duplicate your selection in the clipboard. Not implemented in this program.",
	/* 5 */	"Paste\n\nlets you replace your selection with the contents of the clipboard. Not implemented in this program.",
	/* 6 */	"Clear\n\nlets you delete your selection. Not implemented in this program."
}};

/* The 'hmnu' resource connects menu items to STR# items.
 */
resource 'hmnu' (kEditMenu, purgeable) {
	HelpMgrVersion,
	hmDefaultOptions,
	0,	/* balloon def id */
	0,	/* var code, hang right */
	HMSkipItem {},	/* default item */
	{
	HMStringResItem {kEditHmnu, 1, kEditHmnu, 1, kEditHmnu, 1, kEditHmnu, 1 },
	HMStringResItem {kEditHmnu, 2, kEditHmnu, 2, kEditHmnu, 2, kEditHmnu, 2 },
	HMSkipItem {},
	HMStringResItem {kEditHmnu, 3, kEditHmnu, 3, kEditHmnu, 3, kEditHmnu, 3 },
	HMStringResItem {kEditHmnu, 4, kEditHmnu, 4, kEditHmnu, 4, kEditHmnu, 4 },
	HMStringResItem {kEditHmnu, 5, kEditHmnu, 5, kEditHmnu, 5, kEditHmnu, 5 },
	HMStringResItem {kEditHmnu, 6, kEditHmnu, 6, kEditHmnu, 6, kEditHmnu, 6 }
}};

resource 'MENU' (kRandomDotMenu, "kRandomDotMenu") {
	kRandomDotMenu,
	textMenuProc,
	kAllItems - (kItem1|kItem2|kItem3|kItem4),
	enabled,
	"Random Dot",
	{	
		"Gray Scale", noIcon, "1", noMark, plain,
		"Black & White Stereogram", noIcon, "2", noMark, plain,
		"Gray Scale Stereogram", noIcon, "3", noMark, plain,
		"Shimmering Stereogram", noIcon, "4", noMark, plain
}};

#define kRandomDotHmnu hMenuBase+kRandomDotMenu
resource 'STR#' (kRandomDotHmnu, purgeable) {{
	/* 1 */	"The RandomDot menu has commands relating to controlling this program.",
	/* 2 */	"<configure dialog goes here>",
	/* 3 */ "Gray Scale\n\nWhen this is checked, display depth information using shades of gray.",
	/* 4 */	"Black & White Stereogram\n\nWhen this is checked, display depth information using black & white as the stereogram colors.",
	/* 5 */	"Gray Scale Stereogram\n\nWhen this is checked, display depth information using shades of gray as the stereogram colors.",
	/* 6 */	"Shimmering Stereogram\n\nWhen this is checked, display depth information using animated shades of gray as the stereogram colors."
}};

/* The 'hmnu' resource connects menu items to STR# items.
 */
resource 'hmnu' (kRandomDotMenu, purgeable) {
	HelpMgrVersion,
	hmDefaultOptions,
	0,	/* balloon def id */
	0,	/* var code, hang right */
	HMSkipItem {},	/* default item */
	{
	HMStringResItem {kRandomDotHmnu, 1, kRandomDotHmnu, 1, kRandomDotHmnu, 1, kRandomDotHmnu, 1 },
	HMStringResItem {kRandomDotHmnu, 3, kRandomDotHmnu, 3, kRandomDotHmnu, 3, kRandomDotHmnu, 3 },
	HMStringResItem {kRandomDotHmnu, 4, kRandomDotHmnu, 4, kRandomDotHmnu, 4, kRandomDotHmnu, 4 },
	HMStringResItem {kRandomDotHmnu, 5, kRandomDotHmnu, 5, kRandomDotHmnu, 5, kRandomDotHmnu, 5 },
	HMStringResItem {kRandomDotHmnu, 6, kRandomDotHmnu, 6, kRandomDotHmnu, 6, kRandomDotHmnu, 6 }
}};

resource 'MBAR' (kMBAR, purgeable) {{
		kAppleMenu,
		kFileMenu,
		kEditMenu,
		kRandomDotMenu
}};

resource 'STR#' (kMainStrs, purgeable) {{
	"Random Dot Prefs",
	"Opening “",
	"”.",
	"Computing Stereogram",	/* kComputingStereogramS */
	"Save Image as:",		/* kSaveImageS */
	"Save Stereogram as:",	/* kSaveStereoGramS */
	"Random Dot Help…"		/* kOurHelpS */
}};

resource 'STR#' (kErrorStrs, preload) {{
	"An error occurred.",
	"Directory full.",	/* kDirFulErr */	
	"Disk full.",		/* kDskFulErr */	
	"No such volume.",	/* kNsvErr */	
	"I/O error.",		/* kIOErr */	
	"Bad Name.",		/* kBdNamErr */	
	"File not open.",	/* kFnOpnErr */	
	"End of file.",		/* kEofErr */	
	"Tried to position to before start of file (r/w).",	/* kPosErr */	
	"Memory full (open) or file won't fit (load).",	/* kMFulErr */	
	"Too many files open.",	/* kTmfoErr */	
	"File not found.",	/* kFnfErr */	
	"Diskette is write protected.",	/* kWPrErr */	
	"File is locked.",	/* kFLckdErr */	
	"Volume is locked.",	/* kVLckdErr */	
	"File is busy (delete).",	/* kFBsyErr */	
	"Duplicate filename. Probably can’t create output file because it already exists.",	/* kDupFNErr */	
	"File already open with with write permission.",	/* kOpWrErr */	
	"Refnum error.",	/* kRfNumErr */	
	"Get file position error.",	/* kGfpErr */	
	"Volume not on line error (was Ejected).",	/* kVolOffLinErr */	
	"Permissions error (on file open).",	/* kPermErr */	
	"Drive volume already on-line at MountVol.",	/* kVolOnLinErr */	
	"No such drive (tried to mount a bad drive num).",	/* kNsDrvErr */	
	"Not a mac diskette (sig bytes are wrong).",	/* kNoMacDskErr */	
	"Nolume in question belongs to an external fs.",	/* kExtFSErr */	
	"File system internal error:during rename the old entry was deleted but could not be restored.",	/* kFsRnErr */	
	"Bad master directory block.",	/* kBadMDBErr */	
	"Write permissions error.",	/* kWrPermErr */	
	"Directory not found.",	/* kDirNFErr */	
	"No free WDCB available.",	/* kTmwdoErr */	
	"Move into offspring error.",	/* kBadMovErr */	
	"Wrong volume type error [operation not supported for MFS].",	/* kWrgVolTypErr */	
	"Server volume has been disconnected.",	/* kVolGoneErr */	
	"Insufficient memory to complete operation. Use “Get Info” in the Finder to give this program more memory.",	/* kMemFullError */
	"That port appears to be in use by another program.", 	/* kPortInUse */
	"The preferences file is not available.",	/* kClosedPreferences */
	"This Pict file seems to be damaged.", 	/* kBadPict */
	"I don't know how to open this kind of file",	/* kBadFileType */
	"This pgm file seems to be damaged.",	/* kBadPGMMagicNumber */
	"I can only replace a Pict file.",		/* kPictOnly */
	"This program needs Color Quickdraw and System 7 or later." /* kNeed7AndColor */
}};


resource 'BNDL' (128, purgeable) {
	kCreator, 0, {
	'FREF',	{
			0, 128,
			1, 129,
			2, 130
	},
	'ICN#', {
			0, 128,
			1, 129,
			2, 130
		}
	}
};

type kCreator as 'STR ';


resource kCreator (0, purgeable) {
	"Copyright © 1994 David Phillip Oster"
};

resource 'FREF' (128, purgeable) {
	'APPL',
	0,
	""
};

resource 'FREF' (129, purgeable) {
	kPrefType,
	1,
	""
};

resource 'FREF' (130, purgeable) {	/* so you can drop a PICT file on it */
	'PICT',
	2,
	""
};


type 'TEXT' { string; }; 

resource 'TEXT' (128) {
"RandomDot: Version 1.0.3. A program to work with random dot stereograms"
"Copyright © by David Phillip Oster, 1994, All Rights Reserved. Write to me at oster@netcom.com for more information."
};

resource 'TEXT' (129) {
"Table Of Contents\n"
"-----------------\n"
"Introduction\n"
"Using SIRDS\n"
"How to See SIRDS\n"
"About This Macintosh Program\n"
"Fiction About SIRDS\n"
"\nIntroduction\n"
"------------\n"
"This program generates a “Single Image Random Dot Stereogram”, or SIRDS, from a "
"Macintosh PICT file or a "
"portable grey-map file that describes a 3D scene.  A SIRDS is an image which, "
"when viewed in the appropriate way, appears to the brain as a 3D scene. The "
"image is a stereogram composed of seemingly random dots.  The program "
"incorporates a new, simple, and symmetric algorithm for generating such images "
"from a solid model.  It improves on previous algorithms in several ways: "
"it is symmetric and hence free from directional left-to-right or "
"right-to-left bias, it corrects a slight distortion in the rendering of depth, "
"it removes hidden parts of surfaces, and it also eliminates a type of artifact "
"that we call an “echo.”\n\n"

"The program is described in Working Paper 1993/2 of the Department of Computer "
"Science, University of Waikato, Hamilton, New Zealand.  (This report can be "
"obtained from ftp.cs.waikato.ac.nz.)\n\n"


"Using SIRDS\n"
"-----------\n"
"SIRDS reads and writes PICT files and 256 level PGM files.  A level of 0 (black), "
"corresponds to a z-level of 0.0 (the far plane) through to level 255 (white) "
"corresponding to a z-level of 1.0 (the near plane).\n\n"

"To see your first Single Image Random Dot Stereogram, use the ‘Open’ menu item "
"to select a PICT or a PGM file.  A few examples come with the distribution, mainly the "
"examples given in our paper.\n\n"

"As soon as you have selected a file, the program will automatically generate "
"a SIRDS based on our symmetric algorithm -- with hidden line removal. "  
"This mode is termed the ‘linked (h/s)’ approach in our paper."
"This Macintosh program only generates stereograms with hidden lines removed.\n\n"

"‘mu’ is the distance that the "
"near plane is closer to the eyes than the far plane.  By default this "
"value is 0.33 (~= 1/3) This Macintosh program does not allow you to change ‘mu’.\n\n"

"You can save the SIRDS as a PICT file and you can print it.\n\n"

"Shimmering is approximated by choosing the ‘Shimmer’ menu choice.  This will "
"take the gray scale SIRDS and animate the palette. This "
"only works if your display is set for 256 colors. {An IBM-PC program is also available, this only "
"demonstrates the SHIMMERing effect and is called “shimmer.zip”, available " 
"at the above site}\n\n"


"How to See SIRDS\n"
"----------------\n"
"This program generates stereograms to be viewed while going wall-eyed (or "
"boss-eyed), this is where you focus twice past the screen (look for the "
"reflection of your nose...).  The two dots at bottom of the "
"window aid in viewing the object.\n\n"

"Looking at the two dots, try and focus past the screen; you will initially "
"see 4 dots, if you look further away these dots will converge into 3.  As you "
"stare at the centre dot an object will slowly appear (The time required for "
"this step varies on the person, from 1 second to 10 minutes).  {If you are "
"equally short-sighted in both eyes, removal of your glasses may help…}\n\n"

"I find that Cup and Hemisphere are the easiest files for a first time viewer to see.\n\n"

"The ‘Shimmering’ option also helps to ‘lock onto’ the image, as the "
"cycling colours do not allow you to focus on the plane of the screen.\n\n"

"About This Macintosh Program\n"
"----------------------------\n"
"This program is by David Phillip Oster. Since it is based on work placed in "
"the open literature by Harold W. Thimbleby, Stuart Inglis, & Ian H. Witten, "
"the source code is available for any use with only the following restriction:\n\n"
"you may use this source code any way you like, but you may not forbid anyone else "
"from also using the source code.\n\n"
"The source is available at: ftp://ftp.netcom.com/~ftp/pub/os/oster/SIRDS\n\n"
"This program compiles under THINK C 7, and Metrowerks.\n\n"
"This program is a fat binary, so it runs at full speed on 68000 and PowerPC based "
"Macintoshs. It has Balloon Help. This program runs best if you set your display to "
"‘Millions of Colors’ or to ‘256 colors’. Shimmering only works in 256 colors.\n\n"
"Fiction about SIRDS\n"
"-------------------\n"
"I recommend the novel “Snow Crash” by Neal Stephenson, Bantam Books, 1992, a cyberpunk novel about a "
"cyberspace designed according to the Macintosh User Inteface Guidelines. The plot "
"involves patterns that look to the uninitiated like mere white noise, but the trained "
"mind finds meaning so compelling in the patterns, that the human mind actually crashes, "
"becomes incapable of doing anything else, after perceiving the meaning in the patterns."
};

resource 'hfdr' (-5696) {
	HelpMgrVersion, hmDefaultOptions, 0, 0,
	{
		HMTEResItem{ 128 }
}};

resource 'vers' (1, purgeable) {
	0x1,
	0x3,
	final,
	0x0,
	verUS,
	"Copyright © 1994 by David Phillip Oster, Version 1.0.3",
	"Version 1.0.3"
};


