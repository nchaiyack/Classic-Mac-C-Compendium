/*
	Terminal 2.2
	"Init.c"

	Note: In the final application use ResEdit to clear the locked
	bit of the CODE resource containing this segment. In this way the
	initialisation code gets loaded high in the application heap and
	all non-movable objects created (like strings, windows, master
	pointers, buffers...) will be allocated at the start of the
	application heap without any heap fragmentation when the
	initialisation code is again unloaded.
*/

#ifdef THINK_C
#include "MacHeaders"
#endif
#ifdef applec
#pragma load ":(Objects):MacHeadersMPW"
#pragma segment Init
#endif

#include "Text.h"
#include "Main.h"
#include "Strings.h"
#include "Utilities.h"
#include "CRC.h"
#include "Port.h"
#include "File.h"
#include "Macros.h"

/* Trap numbers */

#define TN_UnknownOS		0xA09F
#define TN_CommToolbox		0xA08B

#define TN_UnknownTool		0xA89F
#define TN_PopUpMenuSelect	0xA80B
#define TN_WaitNextEvent	0xA860
#define TN_ScriptUtil		0xA8B5
#define TN_KeyTrans			0xA9C3
#define TN_OSDispatch		0xA88F

#define INSET	2
#define BAR		16			/* Scroll bar width */
#define TLEFT	2			/* Default terminal window position */
#define TTOP	40
#define PLEFT	26			/* Default progress window position */
#define PTOP	83

/* ----- Read information from 'CNFG' resource ------------------------- */

static void ReadConfig(void)
{
	register Configuration **h;

	/* Read configuration resource. If not available use defauts. */

	if (h = (Configuration **)GetResource('CNFG', 128)) {
		HLock((Handle)h);
		memcpy(&Config, *h, sizeof(Config));
		GetFNum((**h).fontname, &Config.font);
		ReleaseResource((Handle)h);
	} else {
		Config.font = monaco;
		Config.size = 9;
		Config.lines = 24;
		Config.columns = 81;
		Config.buffer = 0x8000;
		Config.input = 0x2000;
		Config.output = 64;
		Config.script = 4096;
	}
}

/* ----- Read information from 'BNDL' resource ------------------------- */

typedef struct {
	short	local;		/* Local ID */
	short	actual;		/* Actual resource ID */
} LOCAL;

typedef struct {
	long	type;		/* Resource type 'ICN#' or 'FREF' */
	short	count;		/* Number of resources - 1 (must be 2) */
	LOCAL	id[3];
} TYPE;

typedef struct {
	long	signature;	/* Signature of the application */
	short	version;	/* Resource ID of version data */
	short	types;		/* Number of resource types - 1 (must be 1) */
	TYPE	info[2];
} BUNDLE;

typedef struct {
	long	type;		/* File type */
	short	local;		/* Local ID for icon list */
} FREF;

static void Bundle(void)
{
	register BUNDLE **h;
	register BUNDLE *p;
	register FREF **f;
	register short i, j;

	Application.version = 0;
	Application.icon = 128;
	Application.signature =
		Application.otype =
		Application.btype =
		Application.stype =
		Application.ztype = '????';

	if (h = (BUNDLE **)GetIndResource('BNDL', 1)) {
		HLock((Handle)h);
		p = *h;
		Application.signature = p->signature;
		Application.version = p->version;
		for (i = 0; i <= p->types; i++)
			switch (p->info[i].type) {
			case 'ICN#':
				for (j = 0; j <= p->info[i].count; j++)
					if (p->info[i].id[j].local == 0) {
						Application.icon = p->info[i].id[j].actual;
						break;
					}
				break;
			case 'FREF':
				for (j = 0; j <= p->info[i].count; j++) {
					if (f = (FREF **)
							GetResource('FREF', p->info[i].id[j].actual)) {
						register long t = (**f).type;
						ReleaseResource((Handle)f);
						switch (p->info[i].id[j].local) {
						case 1:	/* Options */
							Application.otype = t;
							break;
						case 2:	/* MacBinary */
							Application.btype = t;
							break;
						case 3:	/* Script */
							Application.stype = t;
							break;
						case 4:	/* Z-modem part */
							Application.ztype = t;
							break;
						}
					}
				}
				break;
			}
		ReleaseResource((Handle)h);
	}
}

/* ----- Make sure window is on desktop -------------------------------- */

static void WindowScreen(
	Point *p,
	short dh,
	short dv)
{
	RgnHandle rgn;

	if (!(rgn = NewRgn()))
		return;
	SetRectRgn(rgn, p->h, p->v - 15, p->h + 50, p->v + 15);
	UnionRgn(GrayRgn, rgn, rgn);
	if (!EqualRgn(GrayRgn, rgn)) {
		p->h = dh;
		p->v = dv;
	}
	DisposeRgn(rgn);
}

/* ----- Read options -------------------------------------------------- */

static void ReadOptions(void)
{
	short err;
	short volume;
	long directory, scriptDirectory;
	long count;
	Byte volname[28];

	/* Get volume and folder where application is */

	{
		WDPBRec vol;

		vol.ioCompletion = 0;
		vol.ioNamePtr = 0;
		if (PBHGetVol(&vol, FALSE)) {
			volume = Mac.sysVRefNum;
			directory = 2;
		} else {
			volume = vol.ioWDVRefNum;
			directory = vol.ioWDDirID;
		}
		VolumeId(volname, &volume);
	}

	/* If there is a script folder in the application folder use it,
	else use application folder as script folder */

	{
		CInfoPBRec pb;

		scriptDirectory = directory;
		pb.dirInfo.ioCompletion = 0;
		pb.dirInfo.ioNamePtr = (StringPtr)MyString(STR_G, G_SCRIPTFOLDER);
		pb.dirInfo.ioVRefNum = volume;
		pb.dirInfo.ioFDirIndex = 0;
		pb.dirInfo.ioDrDirID = directory;
		if (!PBGetCatInfo(&pb, FALSE) && (pb.dirInfo.ioFlAttrib & 0x10))
			scriptDirectory = pb.dirInfo.ioDrDirID;
	}

	/* Read settings file from script folder */

	{
		short r;

		if (!(err = OpenFile(volume, scriptDirectory,
				MyString(STR_G,G_SETTINGS), &r))) {
			count = sizeof(Settings);
			err = FSRead(r, &count, (Ptr)&Settings);
			FSClose(r);
		}
	}

	/* If a valid settings file is found, use it, else use defaults */

	if (err || count != sizeof(Settings) || Settings.version != SETTINGS ||
			CalcCRC((Byte *)&Settings + sizeof(short),
			sizeof(Settings) - sizeof(short), 0) != Settings.crc) {

		/* Communication options */

		*Settings.portName = 0;
		Settings.portSetup = baud2400+stop10+noParity+data8;
		Settings.dropDTR = FALSE;

		/* TEXT file send options */

		Settings.prompt[0] = 0;
		Settings.linedelay = 0;
		Settings.chardelay = 0;
		Settings.handshake = 0;

		/* Binary file transfer options */

		Settings.Binary = TRUE;
		memcpy(Settings.volumeName, volname, volname[0] + 1);
		Settings.volume = volume;
		Settings.directory = directory;
		Settings.protocol = 0;
		Settings.ZModem = FALSE;
		Settings.ZAutoReceive = FALSE;

		/* XYModem options */

		Settings.XModemtimeout = 600;	/* 10 s */
		Settings.XModemCRC = TRUE;
		Settings.XModem1K = 0;
		Settings.batch = 0;

		/* ZModem options */

		Settings.ZEscapeCtl = FALSE;
		Settings.ZTimeout = 600;		/* 10 s */
		Settings.ZBuffer = 0;
		Settings.ZRetries = 10;
		Settings.ZPacket = 1024;
		Settings.ZWindow = 0;
		Settings.Zcrcq = 0;

		/* Terminal options */

		Settings.localEcho = FALSE;
		Settings.echo = FALSE;
		Settings.autoLF = FALSE;
		Settings.save = TRUE;
		/* Settings.scriptVolume */
		/* Settings.scriptDirectory */
		Settings.startVName[0] = 0;
		Settings.startVolume = 0;
		Settings.startDirectory = 0;
		Settings.startName[0] = 0;

		/* Other options */

		Settings.textCreator = 'PEDT';
		Settings.binType = TEXT;
		Settings.binCreator = 'PEDT';
		Settings.backspace = 0x08;
		Settings.escape = 0x1B;
		Settings.beep = FALSE;
		Settings.ctrl = 0;

		/* Window positions */

		Settings.terminalWindow.h = TLEFT;
		Settings.terminalWindow.v = TTOP;
		Settings.progressWindow.h = PLEFT;
		Settings.progressWindow.v = PTOP;
	} else {
		/* Make sure left corner of windows is on desktop */
		WindowScreen(&Settings.terminalWindow, TLEFT, TTOP);
		WindowScreen(&Settings.progressWindow, PLEFT, PTOP);
	}

	Settings.scriptVolume = volume;
	Settings.scriptDirectory = scriptDirectory;
	Settings.dirty = FALSE;

	/* Make sure volume reference numbers and volume names are ok.
	This is necessary if booting from different disks. */

	Settings.volume = 0;
	if (VolumeId(Settings.volumeName, &Settings.volume)) {
		Settings.volume = volume;
		memcpy(Settings.volumeName, volname, volname[0] + 1);
		Settings.directory = directory;
	}
	Settings.startVolume = 0;
	if (Settings.startVName[0])
		VolumeId(Settings.startVName, &Settings.startVolume);
}

/* ----- Set up new text record ---------------------------------------- */

static long InitTextRecord(
	register TextRecord *p,
	register long size)
{
	p->lines = 1;
	p->firstChar = p->newChar = p->length = p->viewChar = p->viewLine = 0;
	if (size = (p->text = (Byte *)NewPtr(size)) ? (p->size = size) : 0)
		(p->text)[0] = '\015';
	return size;
}

/* ----- Set up script menu items -------------------------------------- */

static void ScriptsInit(void)
{
	register MenuHandle mh;
	register short i;
	register Byte *p;
	register Byte *q;
	register Byte *suffix = (Byte *)MyString(STR_G, G_SUFFIX);
	Byte name[256];
	HParamBlockRec pb;
	short menuitem = 2;

	if (!(mh = GetMenu(SCRIPT)))
		return;
	for (i = 1; ; i++) {
		pb.ioParam.ioCompletion = 0;
		pb.ioParam.ioVRefNum = Settings.scriptVolume;
		pb.fileParam.ioDirID = Settings.scriptDirectory;
		pb.ioParam.ioNamePtr = (StringPtr)name;
		pb.fileParam.ioFDirIndex = i;
		if (PBHGetFInfo(&pb, FALSE))
			break;
		if (pb.fileParam.ioFlFndrInfo.fdType == TEXT)
			for (p = name + *name, q = suffix + *suffix; ; --p, --q) {
				if (q == suffix) {			/* Show the file */
					*name -= *suffix;		/* Strip suffix */
					/* This AppendMenu() followed by SetItem() is necessary
					if there are any of the menu meta characters ( ';' '^'
					'!' '<' '/' '(' ) in the name. */
					AppendMenu(mh, "\p ");
					SetItem(mh, ++menuitem, name);
					break;
				}
				if (p == name || *q != *p)	/* Don't show file */
					break;
			}
	}
}

/* ----- Create new document record ------------------------------------ */

static DocumentPeek NewDocument(void)
{
	register DocumentPeek window;
	FontInfo info;
	Rect r;

	if (window = (DocumentPeek)NewPtr(sizeof(DocumentRecord))) {

		/* Make new window and get font characteristics */

		r = QD(screenBits.bounds);	/* Will be adjusted later */
		if (!NewWindow((void *)window, &r,
				MyString(STR_G, G_TERMINAL),
				FALSE, noGrowDocProc, (WindowPtr)-1L, TRUE, 0)) {
			DisposPtr((Ptr)window);
			return 0;
		}
		SetPort((GrafPtr)window);
		TextFont(Config.font);
		TextFace(0);
		TextMode(srcOr);
		TextSize(Config.size);
		GetFontInfo(&info);
		info.widMax = CharWidth('M');	/* widMax not always correct */

		/* Initialize document fields */

		if (!InitTextRecord(&window->buf, Config.buffer)) {
			DisposeWindow((WindowPtr)window);
			return 0;
		}
		window->file = window->volume = 0;
		window->length = 0;

		/* Adjust window size and position */

		r.top = 21;
		r.left = 0;
		r.bottom = r.top + 2*INSET + Config.lines * (window->height = 
			info.ascent + info.descent + info.leading);
		r.right = r.left + 2*INSET + Config.columns * info.widMax;
		window->rect = r;
		InsetRect(&window->rect, INSET, INSET);
		window->cursor0.v = window->rect.top + info.ascent;
		window->cursor0.h = window->rect.left;
		window->cursor = window->cursor0;
		window->linesPage = Config.lines;
		window->character.top = - info.ascent;
		window->character.left = 0;
		window->character.bottom = info.descent;
		window->character.right = info.widMax;
		r.top = 0;
		r.right += BAR - 1;
		OffsetRect(&r,Settings.terminalWindow.h,Settings.terminalWindow.v);
		MoveWindow((WindowPtr)window, r.left, r.top, FALSE);
		SizeWindow((WindowPtr)window, r.right - r.left,
			r.bottom - r.top, TRUE);

		/* Create window controls */

		r.top = window->rect.top - INSET - 1;
		r.left = window->rect.right + INSET;
		r.bottom = window->rect.bottom + INSET + 1;
		r.right = r.left + BAR;
		if (window->vs = NewControl((WindowPtr)window, &r, EmptyStr, TRUE,
				0, 0, 0, scrollBarProc, 0)) {
			r.top = 2;
			r.left = 2;
			r.bottom = r.top + 16;
			r.right = r.left + 30;
			window->messOk = NewControl((WindowPtr)window, &r, "\p\022",
				FALSE, 0, 0, 0, pushButProc, 0);
			window->messRect.top = 1;
			window->messRect.left = 34;
			window->messRect.bottom = window->rect.top - INSET - 2;
			window->messRect.right = window->rect.right + INSET + BAR
				- 2;
			window->mess[0] = 0;
			ShowWindow((WindowPtr)window);
		} else {
			DisposeWindow((WindowPtr)window);
			return 0;
		}
	}
	return window;
}

/* ----- See if a given trap is available ------------------------------- */

static Boolean TrapAvailable(
	register short num,
	register short type)
{
	if (type == ToolTrap &&
			Mac.machineType > envMachUnknown &&
			Mac.machineType < envMacII) {
		num &= 0x03FF;
		if (num > 0x01FF)
			num = TN_UnknownTool;
	}
	return NGetTrapAddress(num, type) !=
		GetTrapAddress((type == OSTrap) ? TN_UnknownOS : TN_UnknownTool);
}

/* ----- One time initialization ---------------------------------------- */

#ifdef USECTB
	pascal OSErr InitCRM(void);
#endif

Boolean Init(void)
{
	register short err;
	register Handle h;
	pascal void Crash();
	EventRecord event;

	for (err = 0; err < 4; ++err)
		MoreMasters();
	InitGraf(&QD(thePort));
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(Crash);
	InitCursor();
	for (err = 0; err < 3; ++err)
		EventAvail(everyEvent, &event);
	TerminalWindow = 0;
	SysEnvirons(1, &Mac);
	if (Mac.machineType < 0 ||
		!TrapAvailable(TN_PopUpMenuSelect, ToolTrap)) {	/* Old Mac */
		Error(0, MyString(STR_M, M_OLD));
		return TRUE;
	}
	WNE = TrapAvailable(TN_WaitNextEvent, ToolTrap);
	KCHR = (TrapAvailable(TN_ScriptUtil, ToolTrap) &&
			TrapAvailable(TN_KeyTrans, ToolTrap)) ?
		GetResource('KCHR', GetScript(smRoman, smScriptKeys)) : 0;
	MFmemory = TrapAvailable(TN_OSDispatch, ToolTrap);
#ifdef USECTB
	if (CTB = TrapAvailable(TN_CommToolbox, OSTrap))
		InitCRM();
#endif
	Background = FALSE;
	ReadConfig();
	ReadOptions();
	Bundle();
	if (!(h = GetNewMBar(MENUBAR)))
		return TRUE;
	SetMenuBar(h);
	DisposHandle(h);
	if (h = (Handle)GetMHandle(APPLE))
		AddResMenu((MenuHandle)h, 'DRVR');
	ScriptsInit();
	DrawMenuBar();
	TerminalWindow = NewDocument();
	if (TerminalWindow &&
			(err = SerialOpen(Settings.portName, Settings.portSetup,
				Settings.handshake)))
		Error(err, EmptyStr);
	return TerminalWindow == 0;
}
