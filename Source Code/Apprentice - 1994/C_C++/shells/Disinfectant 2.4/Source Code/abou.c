/*______________________________________________________________________

	abou.c - About Window Manager.
	
	Copyright © 1988, 1989, 1990 Northwestern University.  Permission is 
	granted to use this code in your own projects, provided you give credit 
	to both John Norstad and Northwestern University in your about box or 
	document.
_____________________________________________________________________*/


#pragma load "precompile"
#include "utl.h"
#include "rez.h"
#include "glob.h"
#include "wstm.h"
#include "abou.h"
#include "help.h"

#pragma segment abou

/*______________________________________________________________________

	Constant Definitions.
_____________________________________________________________________*/


/* Movie states. */

typedef enum StateType {
	start,				/* start the movie */
	startNameLeft,		/* start sliding virus name left */
	slideNameLeft,		/* slide virus name left */
	slideNamesUp,		/* slide virus names up */
	slideFootDown,		/* slide foot down */
	slideFootUp,		/* slide foot up */
} StateType;

/* Timing constants, in ticks and pixels. */

#define initWait				300			/* initial wait */
#define deltaNameHScroll	2				/* amount to scroll name left each
														time */
#define deltaNameHTicks		1				/* wait between name scrolls left */
#define deltaNameVScroll	2				/* amount to scroll names up each
														time */
#define deltaNameVTicks		1				/* wait between name scrolls up */
#define footWait				120			/* wait before foot starts coming down */
#define deltaFootVScroll	8				/* amount to scroll foot each time */
#define deltaFootVTicks		1				/* wait between foot scrolls */
#define deltaSquishVTicks  2				/* wait between foot scrolls during
														squishing */

/*______________________________________________________________________

	Global Variables.
_____________________________________________________________________*/


static WindowPtr		AbouWindow = nil;	/* ptr to about window */
static WindowRecord	AbouWRecord;		/* about window record */
static WindowObject	AbouWindObject;	/* about window object */
static StateType		State;				/* movie state */
static long				WaitTill;			/* tick count to wait until */
static RgnHandle		UpdateRgn;			/* update region for ScrollRect calls */
static BitMap			FootMap;				/* offscreen foot bitmap */
static GrafPort		NamePort;			/* grafPort for drawing virus names 
														offscreen */
static BitMap			NameMap;				/* offscreen virus names bitmap */
static short			MaxNameWidth;		/* max virus name width */
static short			NamesLeft;			/* left coord of virus names */
static Rect				NamesRect;			/* rectangle enclosing names in NameMap */
static Rect				SquishRect;			/* squished names rectangle */
static Rect				FootCopyRect;		/* rectangle in foot bit map to be copied */
static Rect				AbouCopyRect;		/* rectangle in window to copy
														part of foot to */
static Handle			Tune;					/* Sousa march 'TUNE' resource handle */
static Handle			Instrument;			/* 'snd ' instrument handle for Sousa march */
static Handle			Splat;				/* handle to splat 'snd ' */
static SndChannelPtr	MarchChan;			/* sound channel pointer for march */
static SndChannelPtr	SplatChan;			/* sound channel pointer for splat */
static short			*PTune;				/* pointer to next command in tune */
static short			*PTuneEnd;			/* pointer to end of command in tune */
static Boolean			HaveSound;			/* true if we have the Sound Manager */

/*______________________________________________________________________

	abou_IsOpen - Check to See if About Window is Open.
	
	Exit:		function result = true if window open.
_____________________________________________________________________*/


Boolean abou_IsOpen (void)

{
	return AbouWindow != nil;
}

/*______________________________________________________________________

	Update - Process an Update Event.
_____________________________________________________________________*/


static void Update (void)

{
	PicHandle		mePict;				/* handle to my picture */
	Handle			credits;				/* handle to credits STR# rsrc */
	char				creditsStr[600];	/* credits string */
	char				*pCreditsStr;		/* pointer into credits string */
	char				*pCreditsRsrc;		/* pointer into credits STR# rsrc */
	short				nCreditsStr;		/* number of strings in credits rsrc */
	short				strLen;				/* length of credit string in rsrc */
	
	/* Draw my picture. */
	
	mePict = GetPicture(mePictID);
	HLock((Handle)mePict);
	DrawPicture(mePict, &RectList[meRect]);
	HUnlock((Handle)mePict);
	
	/* Draw the credits. */
	
	credits = GetResource('STR#', abouCreditsID);
	MoveHHi(credits);
	HLock(credits);
	pCreditsStr = creditsStr;
	nCreditsStr = **(short**)credits;
	pCreditsRsrc = *credits+2;
	while (nCreditsStr--) {
		strLen = *pCreditsRsrc++;
		memcpy(pCreditsStr, pCreditsRsrc, strLen);
		pCreditsStr += strLen;
		pCreditsRsrc += strLen;
	};
	HUnlock(credits);
	TextFont(applFont);
	TextSize(9);
	TextBox(creditsStr, pCreditsStr-creditsStr, 
		&RectList[creditsRect], teJustLeft);
	TextFont(systemFont);
	TextSize(0);
		
	/* Draw the movie in its current state. */
	
	if (State == slideNameLeft || State == slideNamesUp) {
		CopyBits(&NameMap, &AbouWindow->portBits, &NameMap.bounds,
			&NameMap.bounds, srcCopy, nil);
	} else if (State == slideFootDown || State == slideFootUp) {
		if (State == slideFootDown) 
			CopyBits(&NameMap, &AbouWindow->portBits, &NamesRect,
				&SquishRect, srcCopy, nil);
		CopyBits(&FootMap, &AbouWindow->portBits, &FootCopyRect,
			&AbouCopyRect, srcCopy, nil);
	};
}

/*______________________________________________________________________

	Suspend - Process a Suspend Event.
_____________________________________________________________________*/


static void Suspend (void)

{
	if (MarchChan) {
		SndDisposeChannel(MarchChan, true);
		MarchChan = nil;
	};
	if (SplatChan) {
		SndDisposeChannel(SplatChan, true);
		SplatChan = nil;
	};
}

/*______________________________________________________________________

	Help - Process Mouse Down Event in Help Mode.
	
	Entry:		where = mouse down location, local coords.
_____________________________________________________________________*/


static void Help (Point where)

{
#pragma unused (where)

	help_Open(tagAbouWind);
}
	
/*______________________________________________________________________

	Close - Close the Window.
_____________________________________________________________________*/


static void Close (void)

{
	if (HaveSound) {
		if (MarchChan) {
			SndDisposeChannel(MarchChan, true);
			MarchChan = nil;
		};
		if (SplatChan) {
			SndDisposeChannel(SplatChan, true);
			SplatChan = nil;
		};
		HUnlock(Tune);
		HUnlock(Instrument);
		HUnlock(Splat);
	};
	Prefs.abouState.moved = AbouWindObject.moved;
	wstm_Save(AbouWindow, &Prefs.abouState);
	DisposeRgn(UpdateRgn);
	DisposPtr(FootMap.baseAddr);
	ClosePort(&NamePort);
	DisposPtr(NameMap.baseAddr);
	CloseWindow(AbouWindow);
	AbouWindow = nil;
};
	
/*______________________________________________________________________

	Adjust - Adjust Menus.
_____________________________________________________________________*/


static void Adjust (void)

{
	MenuHandle				fileM;					/* handle to file menu */
	MenuHandle				editM;					/* handle to edit menu */
	MenuHandle				scanM;					/* handle to scan menu */
	
	fileM = GetMHandle(fileMID);
	editM = GetMHandle(editMID);
	scanM = GetMHandle(scanMID);
	EnableItem(fileM, closeCommand);
	DisableItem(fileM, saveAsCommand);
	DisableItem(fileM, pageSetupCommand);
	DisableItem(fileM, printCommand);
	DisableItem(fileM, printOneCommand);
	DisableItem(editM, 0);
	if (Scanning) {
		DisableItem(scanM, 0);
	} else {
		EnableItem(scanM, 0);
	};
}
	
/*______________________________________________________________________

	PlayTune - Play the Sousa March.
_____________________________________________________________________*/


static void PlayTune (void)

{
	SndCommand			cmd;				/* sound command */
	static short		duration[6] = {36, 285, 570, 855, 1140, 1710};
												/* table of note durations */

	while (PTune < PTuneEnd) {
		cmd.cmd = *PTune < 0 ? restCmd : noteCmd;
		cmd.param1 = duration[((*PTune)>>8)&0x3F];
		cmd.param2 = 0xFF000000 | (*PTune & 0xFF);
		if (SndDoCommand(MarchChan, &cmd, true)) break;
		PTune++;
	};
}
	
/*______________________________________________________________________

	Periodic - Perform Periodic Tasks.
_____________________________________________________________________*/


static void Periodic (void)

{
	/* Static local variables.  These variables retain their values across
		calls to this routine. */
		
	static short		windRight;		/* right coord of window */
	static short		indVir;			/* index into virus info array */
	static Str255		vName;			/* virus name */
	static short		nameWidth;		/* width of virus name */
	static short		stopAt;			/* coord to stop sliding at */
	static Rect			scrollRect;		/* scrolling rectangle */
	static short		footHeight;		/* height of foot */
	static short		deltaSquish;	/* amount to expand squish rect each time */
	static short		squishStart;	/* coord to start squishing */
	static short		extraSquishStart;	/* coord to start squishing extra pixel */
	static short		numNames;		/* number of names currently scrolled into
													view */
	static Rect			eraseRect;		/* rectangle to be erased */
	static short		namesTop;		/* top coord of NamesMap */
	
	/* Non-static local variables. */
	
	long					ticks;			/* current tick count */
	short					extraWidth;		/* foot width minus virus names width */
	short					squishSteps;	/* number of squishing scrolls */
	short					extraSquish;	/* number of extra squishing pixels */
	SndCommand			cmd;				/* sound command */
	short					noteCount;		/* number of notes in tune */
		
	ticks = TickCount();
	if (ticks < WaitTill) return;
	
	/* Keep the tune going. */
	
	if (MarchChan) {
		PlayTune();
	};
	
	switch (State) {
	
		case start:
			
			indVir = 1;
			numNames = 0;
			GetIndString(vName, abouVNamesID, indVir++);
			windRight = AbouWindow->portRect.right;
			SetPort(&NamePort);
			EraseRect(&NameMap.bounds);
			SetPort(AbouWindow);
			State = startNameLeft;
			WaitTill = ticks + initWait;
			break;
			
		case startNameLeft:
		
			nameWidth = StringWidth(vName);
			scrollRect.left = windRight-deltaNameHScroll;
			scrollRect.right = windRight;
			scrollRect.top = AbouWindow->portRect.bottom-40;
			scrollRect.bottom = scrollRect.top+16;
			stopAt = NamesLeft;
			namesTop = NameMap.bounds.top;
			State = slideNameLeft;
			
			if (!MarchChan && HaveSound && InForeground) {
			
				/* Allocate sound channel. */
				
				SndNewChannel(&MarchChan, sampledSynth, 0, nil);
				
				/* Install the instrument. */
				
				cmd.cmd = soundCmd;
				cmd.param1 = 0;
				cmd.param2 = (long)(*Instrument+0x16);
				SndDoCommand(MarchChan, &cmd, true);
				
				/* Start playing the tune. */
				
				noteCount = **(short**)Tune;	
				PTune = (short*)(*Tune+2);
				PTuneEnd = PTune + noteCount;
				PlayTune();
			};
			
			break;
			
		case slideNameLeft:
		
			SetPort(&NamePort);
			ScrollRect(&scrollRect, -deltaNameHScroll, 0, UpdateRgn);
			SetOrigin(NamesLeft, namesTop);
			if (scrollRect.left + nameWidth + 4 >= windRight) {
				MoveTo(scrollRect.left, scrollRect.bottom-4);
				DrawString(vName);
			};
			SetPort(AbouWindow);
			CopyBits(&NameMap, &AbouWindow->portBits, &scrollRect,
				&scrollRect, srcCopy, nil);
			if (scrollRect.left - deltaNameHScroll < stopAt) {
				numNames++;
				GetIndString(vName, abouVNamesID, indVir++);
				if (*vName) {
					scrollRect.right = scrollRect.left + MaxNameWidth;
					scrollRect.top = scrollRect.bottom - 16*numNames - 
						deltaNameVScroll;
					stopAt = scrollRect.top - 16 + deltaNameVScroll;
					WaitTill = ticks + deltaNameVTicks;
					State = slideNamesUp;
				} else {
					stopAt = AbouWindow->portRect.bottom - 24;
					SquishRect = scrollRect;
					SquishRect.right = SquishRect.left + MaxNameWidth;
					SquishRect.top = SquishRect.bottom - 16*numNames;
					extraWidth = (FootMap.bounds.right - FootMap.bounds.left -
						MaxNameWidth) >> 1;
					squishSteps = (16*numNames-2)/deltaFootVScroll + 1;
					deltaSquish = extraWidth/squishSteps;
					extraSquish = extraWidth - deltaSquish*squishSteps;
					extraSquishStart = stopAt - deltaFootVScroll*extraSquish;
					squishStart = SquishRect.top;
					footHeight = FootMap.bounds.bottom - FootMap.bounds.top;
					FootCopyRect = FootMap.bounds;
					FootCopyRect.top = FootCopyRect.bottom;
					AbouCopyRect = FootCopyRect;
					AbouCopyRect.top = AbouCopyRect.bottom = 0;
					eraseRect = AbouCopyRect;
					NamesRect = SquishRect;
					WaitTill = ticks + footWait;
					State = slideFootDown;
				};
			} else {
				scrollRect.left -= deltaNameHScroll;
				if (scrollRect.left + nameWidth + 4 < windRight) 
					scrollRect.right = scrollRect.left + nameWidth + 4;
				WaitTill = ticks + deltaNameHTicks;
			};
			break;
			
		case slideNamesUp:
		
			SetPort(&NamePort);
			ScrollRect(&scrollRect, 0, -deltaNameVScroll, UpdateRgn);
			SetOrigin(NamesLeft, namesTop);
			SetPort(AbouWindow);
			CopyBits(&NameMap, &AbouWindow->portBits, &scrollRect,
				&scrollRect, srcCopy, nil);
			WaitTill = ticks + deltaNameVTicks;
			scrollRect.top -= deltaNameVScroll;
			scrollRect.bottom -= deltaNameVScroll;
			if (scrollRect.top < stopAt) State = startNameLeft;
			break;
			
		case slideFootDown:
		
			AbouCopyRect.bottom += deltaFootVScroll;
			FootCopyRect.top -= deltaFootVScroll;
			if (AbouCopyRect.bottom > footHeight) {
				AbouCopyRect.top = AbouCopyRect.bottom - footHeight;
				FootCopyRect.top = 0;
				eraseRect.bottom = AbouCopyRect.top;
				eraseRect.top = eraseRect.bottom - deltaFootVScroll;
				EraseRect(&eraseRect);
			};
			CopyBits(&FootMap, &AbouWindow->portBits,
				&FootCopyRect, &AbouCopyRect, srcCopy, nil);
			if (AbouCopyRect.bottom < stopAt) {
				if (AbouCopyRect.bottom > squishStart) {
					if (MarchChan) {
						SndDisposeChannel(MarchChan, true);
						MarchChan = nil;
						if (!SplatChan) {
							SndNewChannel(&SplatChan, 0, 0, nil);
							SndPlay(SplatChan, Splat, true);
						};
					};
					SquishRect.top = AbouCopyRect.bottom;
					CopyBits(&NameMap, &AbouWindow->portBits, 
						&NamesRect, &SquishRect, srcCopy, nil);
					SquishRect.left -= deltaSquish;
					SquishRect.right += deltaSquish;
					if (AbouCopyRect.bottom > extraSquishStart) {
						SquishRect.left -= 1;
						SquishRect.right += 1;
					};
					WaitTill = ticks + deltaSquishVTicks; 
				} else {
					WaitTill = ticks + deltaFootVTicks;
				};
				scrollRect.bottom += deltaFootVScroll;
			} else {
				eraseRect = AbouCopyRect;
				eraseRect.top = eraseRect.bottom - deltaFootVScroll;
				State = slideFootUp;
				WaitTill = ticks + deltaFootVTicks;
				if (SplatChan) {
					SndDisposeChannel(SplatChan, true);
					SplatChan = nil;
				};
			};
			break;
			
		case slideFootUp:
		
			OffsetRect(&AbouCopyRect, 0, -deltaFootVScroll);
			if (AbouCopyRect.bottom < footHeight) {
				AbouCopyRect.top = 0;
				FootCopyRect.top = footHeight - AbouCopyRect.bottom;
			};
			CopyBits(&FootMap, &AbouWindow->portBits,
				&FootCopyRect, &AbouCopyRect, srcCopy, nil);
			EraseRect(&eraseRect);
			OffsetRect(&eraseRect, 0, -deltaFootVScroll);
			if (AbouCopyRect.bottom > 0) {
				WaitTill = ticks + deltaFootVTicks;
			} else {
				WaitTill = ticks;
				State = start;
			};
			break;
			
	};
			
}
	
/*______________________________________________________________________

	abou_Open - Open Abou Window.
_____________________________________________________________________*/


void abou_Open (void)

{
	GrafPort			footPort;			/* grafPort for drawing foot offscreen */
	PicHandle		footPict;			/* handle to foot picture */
	short				indVir;				/* loop index */
	Str255			vName;				/* virus name */
	short				nameWidth;			/* width of virus name */
	short				numVNames;			/* number of virus names */
	
	/* If the window is already open, activate it. */
	
	if (AbouWindow) {
		SelectWindow(AbouWindow);
		return;
	};
	
	/* Get the about window and restore its state. */
	
	AbouWindow = wstm_Restore(false, abouWindID, (Ptr)&AbouWRecord,
		&Prefs.abouState);
	SetPort(AbouWindow);
	
	/* Initialize the window object. */
	
	((WindowPeek)AbouWindow)->refCon = (long)&AbouWindObject;
	AbouWindObject.windKind = abouWind;
	AbouWindObject.moved = Prefs.abouState.moved;
	AbouWindObject.update = Update;
	AbouWindObject.activate = nil;
	AbouWindObject.deactivate = nil;
	AbouWindObject.resume = nil;
	AbouWindObject.suspend = Suspend;
	AbouWindObject.click = nil;
	AbouWindObject.help = Help;
	AbouWindObject.grow = nil;
	AbouWindObject.zoom = nil;
	AbouWindObject.key = nil;
	AbouWindObject.close = Close;
	AbouWindObject.disk = nil;
	AbouWindObject.save = nil;
	AbouWindObject.pageSetup = nil;
	AbouWindObject.print = nil;
	AbouWindObject.edit = nil;
	AbouWindObject.adjust = Adjust;
	AbouWindObject.periodic = Periodic;
	AbouWindObject.dialogPre = nil;
	AbouWindObject.dialogPost = nil;
	
	/* Create the update region for the ScrollRect calls. */
	
	UpdateRgn = NewRgn();
	
	/* Allocate and initialize the offscreen foot bitmap. */
	
	FootMap.bounds = RectList[footRect];
	FootMap.rowBytes = (((FootMap.bounds.right-FootMap.bounds.left+7)>>3) + 1) 
		& 0xfffe;
	FootMap.baseAddr = NewPtr(FootMap.rowBytes*(FootMap.bounds.bottom-
		FootMap.bounds.top));
	
	/* Draw the foot picture in the offscreen bitmap. */
	
	OpenPort(&footPort);
	SetPortBits(&FootMap);
	footPort.portRect = RectList[footRect];
	ClipRect(&footPort.portRect);
	EraseRect(&footPort.portRect);
	footPict = GetPicture(footPictID);
	HLock((Handle)footPict);
	DrawPicture(footPict, &footPort.portRect);
	HUnlock((Handle)footPict);
	ClosePort(&footPort);
	SetPort(AbouWindow);
	
	/* Allocate the offscreen virus names bitmap and grafport. */
	
	MaxNameWidth = 0;
	TextFont(systemFont);
	TextSize(0);
	numVNames = 0;
	for (indVir = 1; ; indVir++) {
		GetIndString(vName, abouVNamesID, indVir);
		if (!*vName) break;
		numVNames++;
		nameWidth = StringWidth(vName);
		if (nameWidth > MaxNameWidth) MaxNameWidth = nameWidth;
	};
	NamesLeft = (RectList[footRect].right + RectList[footRect].left 
		- MaxNameWidth) >> 1;
	SetRect(&NameMap.bounds, NamesLeft, 
		AbouWindow->portRect.bottom - 24 - 16*numVNames,
		AbouWindow->portRect.right,
		AbouWindow->portRect.bottom - 24);
	NameMap.rowBytes = (((NameMap.bounds.right-NameMap.bounds.left+7)>>3) + 1) & 
		0xfffe;
	NameMap.baseAddr = NewPtr(NameMap.rowBytes * 16 * numVNames);
	OpenPort(&NamePort);
	SetPortBits(&NameMap);
	NamePort.portRect = NameMap.bounds;
	ClipRect(&NamePort.portRect);
	SetPort(AbouWindow);
	
	/* Get and lock the 'snd ' and 'TUNE' resources. */
	
	HaveSound = utl_HaveSound();
	MarchChan = nil;
	SplatChan = nil;
	if (HaveSound) {
		Tune = GetResource('TUNE', abouNotes);
		MoveHHi(Tune);
		HLock(Tune);
		Instrument = GetResource('snd ', abouInstrument);
		MoveHHi(Instrument);
		HLock(Instrument);
		Splat = GetResource('snd ', abouSplat);
		MoveHHi(Splat);
		HLock(Splat);
	};
	
	/* Initialize the WaitTill and State variables. */
	
	WaitTill = 0;
	State = start;	
	
	/* Show the about window. */
	
	ShowWindow(AbouWindow);
}