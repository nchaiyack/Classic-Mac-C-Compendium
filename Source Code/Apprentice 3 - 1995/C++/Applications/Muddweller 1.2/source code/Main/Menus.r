/* Menus - Defines the menus of MUDDweller                                    */

#ifndef __TYPES.R__
#include "Types.r"
#endif

#ifndef __MacAppTypes__
#include "MacAppTypes.r"
#endif


/* -------------------------------------------------------------------------- */

#define mCommunication		4			/* Configure menu */
#define mSend				5			/* Send menu */

#define puFont				1002		/* Font popup */
#define puMacros			1003		/* Macros popup */

#define cConfigure			1001		/* configure connection */
#define cPrefs				1002		/* user preferences */
#define cConnect			1003		/* connect or disconnect */
#define cBreak				1004		/* Send a break signal */
#define cComm				1005		/* configure communication */
#define cInterrupt			1006		/* send an interrupt signal */
#define cUpdate				1007		/* update changed files */
#define cUpdateAll			1008		/* update all files */
#define cDownload			1009		/* download file */
#define cSetup				1010		/* setup downloads */
#define cSendFile			1011		/* send a file */
#define cUseCTB				1012		/* using CTB */
#define cUpload				1013		/* upload file */
#define cLogFile			1014		/* open or close log */
#define cMacros				1015		/* edit macros */


/* -------------------------------------------------------------------------- */

include "Defaults.rsrc"  'cmnu' (mBuzzwords);
include "Defaults.rsrc"  'cmnu' (mApple);
include "Defaults.rsrc"  'cmnu' (mEdit);


/* -------------------------------------------------------------------------- */

resource 'cmnu' (mFile,
#if qNames
"mFile",
#endif
nonpurgeable) {
	mFile,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	"File",
	{
	"New",				noIcon, "N",	noMark, plain, cNew;
	"Open�",			noIcon, "O",	noMark, plain, cOpen;
	"-",				noIcon, noKey,	noMark, plain, nocommand;
	"Close",			noIcon, "W",	noMark, plain, cClose;
	"Save",				noIcon, "S",	noMark, plain, cSave;
	"Save As�",			noIcon, noKey,	noMark, plain, cSaveAs;
	"Save a Copy�",		noIcon, noKey,	noMark, plain, cSaveCopy;
//	"Revert",			noIcon, noKey,	noMark, plain, cRevert;
//	"-",				noIcon, noKey,	noMark, plain, nocommand;
//	"Page Setup�",		noIcon, noKey,	noMark, plain, cPageSetup;
//	"Print�",			noIcon, "P",	noMark, plain, cPrint;
	"-",				noIcon, noKey,	noMark, plain, nocommand;
	"Quit",				noIcon, "Q",	noMark, plain, cQuit
	}
};


/* -------------------------------------------------------------------------- */

resource 'cmnu' (mCommunication,
#if qNames
"Configure",
#endif
nonpurgeable) {
	mCommunication,
	textMenuProc,
	0x7FFFFFFF,
	disabled,
	"Configure",
	{
	"TCP/IP Address�",	noIcon, noKey,	noMark, plain, cConfigure;
	"Use Comm Toolbox",	noIcon, noKey,	noMark, plain, cUseCTB;
	"-",				noIcon, noKey,	noMark, plain, nocommand;
	"Preferences�",		noIcon, noKey,	noMark, plain, cPrefs;
	"Communication�",	noIcon, noKey,	noMark, plain, cComm;
	"File Transfers�",	noIcon, noKey,	noMark, plain, cSetup;
	"Macros�",			noIcon, noKey,	noMark, plain, cMacros;
	"-",				noIcon, noKey,	noMark, plain, nocommand;
	"Log to File�",		noIcon, noKey,	noMark, plain, cLogFile;
	"Open Connection",	noIcon, noKey,	noMark, plain, cConnect;
	}
};


/* -------------------------------------------------------------------------- */

resource 'cmnu' (mSend,
#if qNames
"Send",
#endif
nonpurgeable) {
	mSend,
	textMenuProc,
	0x7FFFFFFF,
	disabled,
	"Send",
	{
	"Update",			noIcon, "B",	noMark, plain, cUpdate;
	"Full Update",		noIcon, noKey,	noMark, plain, cUpdateAll;
	"-",				noIcon, noKey,	noMark, plain, nocommand;
	"Upload File�",		noIcon, "U",	noMark, plain, cUpload;
	"Download File�",	noIcon, "D",	noMark, plain, cDownload;
	"-",				noIcon, noKey,	noMark, plain, nocommand;
	"Send Break",		noIcon, noKey,	noMark, plain, cBreak;
//	"Send Interrupt",	noIcon, noKey,	noMark, plain, cInterrupt;
	"Send File�",		noIcon, noKey,	noMark, plain, cSendFile;
	}
};


/* --------------------------------------------------------------------------- */

resource 'cmnu' (puFont) {
	puFont,
	textMenuProc,
	0x7FFFFFFF,
	enabled,
	"Screen font: ",
	{
	}
};


/* --------------------------------------------------------------------------- */

resource 'cmnu' (puMacros) {
	puMacros,
	textMenuProc,
	0x7FFFFFFF,
	enabled,
	"Macro: ",
	{
	"Login",			noIcon, noKey,	noMark, plain, nocommand;
	"Logout",			noIcon, noKey,	noMark, plain, nocommand;
	"-",				noIcon, noKey,	noMark, plain, nocommand;
	}
};


/* -------------------------------------------------------------------------- */

resource 'MBAR' (kMBarDisplayed,
#if qNames
"kMBarDisplayed",
#endif
nonpurgeable) {

	{mApple; mFile; mEdit; mCommunication; mSend}
};
