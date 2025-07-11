/* Finder - Defines all resources used by the Finder                          */

#ifndef __TYPES.R__
#include "Types.r"
#endif

#ifndef __SYSTYPES.R__
#include "SysTypes.r"
#endif

#ifndef __MacAppTypes__
#include "MacAppTypes.r"
#endif


// Application icon and necessary bundling rsrcs

#define kSignature ' %|g'


type kSignature as 'STR ';
resource kSignature (0,
#if qNames
"Signature",
#endif
	purgeable) {
	"MUDDweller 1.1, by O. Maquelin"
};


resource 'FREF' (128,
#if qNames
"Application",
#endif
	purgeable) {
	'APPL',
	0,
	""
};


resource 'FREF' (129,
#if qNames
"Dungeon",
#endif
	purgeable) {
	'MUD ',
	1,
	""
};


resource 'BNDL' (128,
#if qNames
"Bundle",
#endif
	purgeable) {
	kSignature,
	0,
		{
		'ICN#',
			{
			0, 128,
			1, 129,
			},
		'FREF',
			{
			0, 128,
			1, 129,
			}
		}
};


resource 'ICN#' (128,
#if qNames
"Application",
#endif
purgeable) {
	{	/* array: 2 elements */
		/* [1] */
		$"3FFF FFF8 4000 0004 9FFF FFF2 A000 000A"
		$"A000 000A A01C 700A A015 500A A01C 700A"
		$"A000 000A A008 200A A000 000A A71C 71CA"
		$"A555 554A A71C 71CA A000 000A A200 208A"
		$"A000 000A A700 71CA A500 554A A700 71CA"
		$"A000 000A A000 000A 9FFF FFF2 4000 0004"
		$"3FFF FFF8 0001 0000 0001 0000 0003 8000"
		$"0002 8000 000E E000 FFF8 3FFE 000F E0",
		/* [2] */
		$"3FFF FFF8 7FFF FFFC FFFF FFFE FFFF FFFE"
		$"FFFF FFFE FFFF FFFE FFFF FFFE FFFF FFFE"
		$"FFFF FFFE FFFF FFFE FFFF FFFE FFFF FFFE"
		$"FFFF FFFE FFFF FFFE FFFF FFFE FFFF FFFE"
		$"FFFF FFFE FFFF FFFE FFFF FFFE FFFF FFFE"
		$"FFFF FFFE FFFF FFFE FFFF FFFE 7FFF FFFC"
		$"3FFF FFF8 0001 0000 0001 0000 0003 8000"
		$"0003 8000 000F E000 FFFF FFFE 000F E0"
	}
};

data 'ics#' (128,
#if qNames
"Application",
#endif
purgeable) {
		$"7FFC 8002 86C2 86C2 8002 B6DA B6DA 8002"
		$"B0DA B0DA 8002 7FFC 0100 07C0 FC7E 07C0"
		$"7FFC FFFE FFFE FFFE FFFE FFFE FFFE FFFE"
		$"FFFE FFFE FFFE 7FFC 0100 07C0 FFFE 07C0"
};

resource 'ICN#' (129,
#if qNames
"MUD",
#endif
purgeable) {
	{	/* array: 2 elements */
		/* [1] */
		$"0FFF FE00 0800 0300 0800 0280 080E 0240"
		$"080A 0220 080E 0210 0800 03F8 0804 0008"
		$"0800 0008 080E 3808 080A A808 080E 3808"
		$"0800 0008 0804 1008 0800 0008 0B8E 38E8"
		$"0AAA AAA8 0B8E 38E8 0800 0008 0900 1048"
		$"0800 0008 0B80 38E8 0A80 2AA8 0B80 38E8"
		$"0800 0008 0800 1008 0800 0008 0800 3808"
		$"0800 2808 0800 3808 0800 0008 0FFF FFF8",
		/* [2] */
		$"0FFF FE00 0FFF FF00 0FFF FF80 0FFF FFC0"
		$"0FFF FFE0 0FFF FFF0 0FFF FFF8 0FFF FFF8"
		$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"
		$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"
		$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"
		$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"
		$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"
		$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"
	}
};

data 'ics#' (129,
#if qNames
"MUD",
#endif
purgeable) {
		$"7FF0 4038 402C 433C 4304 4004 5B64 5B64"
		$"4004 5864 5864 4004 4064 4064 4004 7FFC"
		$"7FF0 7FF8 7FFC 7FFC 7FFC 7FFC 7FFC 7FFC"
		$"7FFC 7FFC 7FFC 7FFC 7FFC 7FFC 7FFC 7FFC"
};


resource 'vers' (1,
#if qNames
	"Application Version",
#endif
	purgeable) {
	0x01,
	0x02,
	final,
	0x00,
	verUs,
	"1.2",
	"MUDDweller 1.2, by O. Maquelin"
};

resource 'mem!' (1000,
#if qNames
	"Memory",
#endif
	purgeable) {
	10000,				// Add to temporary reserve
	10000,				// Add to low-memory reserve
	5000				// Add to stack space
};


resource 'seg!' (1000,	// Needed to calculate temporary reserve.
#if qNames				// The segments were chosen somewhat arbitrarily;
	"Segments",			// this should reserve more than enough space.
#endif
	purgeable) {
	{	"GDoCommand";
		"GFile";
		"GOpen";
		"GClose";
		"SFileTrans";
		"SDocDialogs";
		"SEDStuff";
		"SMUDDocRes";
		"SMTPStuff";
	};
};


resource 'res!' (1000,
#if qNames
	"Resident",
#endif
	purgeable) {
	{	"SMUDDocRes";
	};
};


resource 'SIZE' (-1) {
	saveScreen,
	acceptSuspendResumeEvents,
	enableOptionSwitch,
	canBackground,
	MultiFinderAware,
	backgroundAndForeground,
	dontGetFrontClicks,
	ignoreChildDiedEvents,
	is32BitCompatible,
	reserved,
	reserved,
	reserved,
	reserved,
	reserved,
	reserved,
	reserved,
#if qDebug
	1000 * 1024,
	750 * 1024
#else
	600 * 1024,		// try to leave enough room for printing
	400 * 1024
#endif
};


resource 'CURS' (1000, preload) {
	$"07C0 1F30 3F08 7F04 7F04 FF02 FF02 FFFE"
	$"81FE 81FE 41FC 41FC 21F8 19F0 07C0",
	$"07C0 1FF0 3FF8 7FFC 7FFC FFFE FFFE FFFE"
	$"FFFE FFFE 7FFC 7FFC 3FF8 1FF0 07C0",
	{7, 7}
};

resource 'CURS' (1001, preload) {
	$"07C0 1FF0 3FC8 7FC4 7F84 9F82 8702 8102"
	$"81C2 83F2 43FC 47FC 27F8 1FF0 07C0",
	$"07C0 1FF0 3FF8 7FFC 7FFC FFFE FFFE FFFE"
	$"FFFE FFFE 7FFC 7FFC 3FF8 1FF0 07C0",
	{7, 7}
};

resource 'CURS' (1002, preload) {
	$"07C0 1FF0 3FF8 5FF4 4FE4 87C2 8382 8102"
	$"8382 87C2 4FE4 5FF4 3FF8 1FF0 07C0",
	$"07C0 1FF0 3FF8 7FFC 7FFC FFFE FFFE FFFE"
	$"FFFE FFFE 7FFC 7FFC 3FF8 1FF0 07C0",
	{7, 7}
};

resource 'CURS' (1003, preload) {
	$"07C0 1FF0 27F8 47FC 43FC 83F2 81C2 8102"
	$"8702 9F82 7F84 7FC4 3FC8 1FF0 07C0",
	$"07C0 1FF0 3FF8 7FFC 7FFC FFFE FFFE FFFE"
	$"FFFE FFFE 7FFC 7FFC 3FF8 1FF0 07C0",
	{7, 7}
};

resource 'CURS' (1004, preload) {
	$"07C0 19F0 21F8 41FC 41FC 81FE 81FE FFFE"
	$"FF02 FF02 7F04 7F04 3F08 1F30 07C0",
	$"07C0 1FF0 3FF8 7FFC 7FFC FFFE FFFE FFFE"
	$"FFFE FFFE 7FFC 7FFC 3FF8 1FF0 07C0",
	{7, 7}
};

resource 'CURS' (1005, preload) {
	$"07C0 1830 2038 407C 407C F0FE FCFE FFFE"
	$"FE7E FE1E 7C04 7C04 3808 1830 07C0",
	$"07C0 1FF0 3FF8 7FFC 7FFC FFFE FFFE FFFE"
	$"FFFE FFFE 7FFC 7FFC 3FF8 1FF0 07C0",
	{7, 7}
};

resource 'CURS' (1006, preload) {
	$"07C0 1830 2008 701C 783C FC7E FEFE FFFE"
	$"FEFE FC7E 783C 701C 2008 1830 07C0",
	$"07C0 1FF0 3FF8 7FFC 7FFC FFFE FFFE FFFE"
	$"FFFE FFFE 7FFC 7FFC 3FF8 1FF0 07C0",
	{7, 7}
};

resource 'CURS' (1007, preload) {
	$"07C0 1830 3808 7C04 7C04 FE1E FE7E FFFE"
	$"FCFE F0FE 407C 407C 2038 1830 07C0",
	$"07C0 1FF0 3FF8 7FFC 7FFC FFFE FFFE FFFE"
	$"FFFE FFFE 7FFC 7FFC 3FF8 1FF0 07C0",
	{7, 7}
};
