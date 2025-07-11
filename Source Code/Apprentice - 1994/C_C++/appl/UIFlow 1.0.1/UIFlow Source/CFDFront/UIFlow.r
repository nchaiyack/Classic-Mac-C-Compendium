// ================================================================
#ifndef __TYPES.R__
	#include "Types.r"					// SIZE, WIND, STR  , MBAR etc.
#endif

#ifndef __MacAppTypes__
	#include "MacAppTypes.r"			// cmnu, etc.
#endif


#if qDebug
	include "Debug.rsrc";				// always include
#endif

include "MacApp.rsrc";
include "UIFlow.rsrc";
include "UIFlow" 'CODE';

//	cmnu menu Id numbers
//	Options Menu elements
#define	cExport				2000

#define	cOptPrint			2001
#define	cOptRelax			2002
#define cOptSolution		2003
#define	cOptFlow			2004

#define cRApp				2009
#define cRemote				2005
#define cLocal				2006

//	Grid Menu Elements
#define cGridOn				2007
#define cGridFine			2008

// Menu Resources
include"Defaults.rsrc"'cmnu'(mApple);
include"Defaults.rsrc"'cmnu'(mEdit);

resource 'cmnu' (2)
	{
	2,
	textMenuProc,
	0x7FFFFFFF,
	enabled,
	"File",
		{
		"New",noIcon,"N",noMark,plain,cNew;
		"Open",noIcon,"O",noMark,plain,cOpen;
		"-",noIcon,noKey,noMark,plain,noCommand;
		"Close",noIcon,"W",noMark,plain,cClose;
		"Save",noIcon,"S",noMark,plain,cSave;
		"Save As...",noIcon,noKey,noMark,plain,cSaveAs;
		"-",noIcon,noKey,noMark,plain,noCommand;
		"Export Input Deck",noIcon,"E",noMark,plain,cExport;
		"-",noIcon,noKey,noMark,plain,noCommand;
		"Page Setup...",noIcon,noKey,noMark,plain,cPageSetup;
		"Print One",noIcon,noKey,noMark,plain,cPrintOne;
		"Print",noIcon,"P",noMark,plain,cPrint;
		"-",noIcon,noKey,noMark,plain,noCommand;
		"Quit",noIcon,"Q",noMark,plain,cQuit;
		}
	};

resource 'cmnu' (4)
	{
	4,
	textMenuProc,
	0x7FFFFFFF,
	enabled,
	"Simulation",
		{
		"Set Remote Application",noIcon,noKey,noMark,plain,cRApp;
		"Remote",noIcon,"R",noMark,plain,cRemote;
		"Local",noIcon,"L",noMark,plain,cLocal;
		}
	};

resource 'cmnu' (5)
	{
	6,
	textMenuProc,
	allEnabled,
	enabled,
	"Options",
		{
		"Print Variables...",noIcon,noKey,noMark,plain,cOptPrint;
		"Relax Factors...",noIcon,noKey,noMark,plain,cOptRelax;
		"-",noIcon,noKey,noMark,plain,noCommand;
		"Flow Parameters...",noIcon,noKey,noMark,plain,cOptFlow;
		"Solution Parameters...",noIcon,noKey,noMark,plain,cOptSolution;
		}
	};
	
resource 'cmnu' (6)
	{
	7,
	textMenuProc,
	allEnabled,
	enabled,
	"Grid",
		{
		"Hide Grid",noIcon,"G",noMark,plain,cGridOn;
		}
	};
	
resource 'MBAR' (kMBarDisplayed,
	#if qNames
		"MultiFlow",
	#endif
		purgeable)
		{
			{mApple;2;mEdit;4;5;6;}
		};

