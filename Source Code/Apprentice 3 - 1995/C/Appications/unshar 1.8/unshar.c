/*
	File:		unshar.c

	Contains:	unshar for MPW that's good enough for
				comp.sources.unix archives

	Written by:	sw
				Network Analysis Ltd
				178 Wainbody Ave South
				Coventry CV3 6BX
				UK
	Phone:		+44 1203 419996
	E-mail:		sw@network-analysis-ltd.co.uk
	
	Copyright:	Public domain

	Change History (most recent first):

	<10>		30/06/95	sw		Changed to work with TransSkel 3.x, univ headers,
									Metrowerks and native PPC.
	<9>			18/11/91	sw		Changes to compile with TC 5.0.
	<8>			18/11/91	sw		Deal with embedded "cd"s
	<7>			 19/5/91	sw		Remove tests for "if"s and make less sensitive to
									variations in shar file formats. Also stop
									looking for #! /bin/sh; any comment line will do.
									Merge MPW and Think C vers into 1 src file.
	<6>			 16/5/91	sw		Handle shar files in sumex archives.
	<5>			  3/5/91	sw		Previous mod didn't get the last char of the
									filename.
	<4>			  3/5/91	sw		Cope with shar files that do not quote the
									filename in the "if test -f" line.
	<3>			  3/2/91	sw		Pick up terminating string from "sed" or "cat"
									command line.
	<2>			  5/7/90  	sw		Reconvert to MPW tool
	<1>			  ??????	aw		Original by Amanda Walker, Intercon

	To Do:
*/
#ifdef	MPW
#include <Types.h>
#include <StdDef.h>
#include <CursorCtl.h>
#endif	MPW
#include <Files.h>
#include <Errors.h>
#include <Script.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <String.h>
#ifndef NIL
#define NIL	(0L)
#endif

/* Strings used in error messages */
#ifdef	MPW
#define	CPSTR		c2pstr
#define PCSTR		p2cstr
#else
#define CPSTR		CtoPstr
#define PCSTR		PtoCstr
#endif

#ifndef EOF
#define EOF	(-1L)
#endif

#ifndef	MPW
/*	for Think C/Metrowerks
*/
#include <StandardFile.h>
#include "TransSkel.h"

#define DIRECTORY(pb)	(((pb).dirInfo.ioFlAttrib & 0x10) == 0x10)
#define	getDir		14			/* buttons in the dialogue box */
#define	getCurDir	15
#define	GD_PROMPT	16

typedef enum
{
	infoDlgRes = 1000,
	aboutAlrtRes,			/* About... alert resource number */
	abortRes,
	gdDlgRes,
	typeDlgRes,
	errorAlertRes,
	dupFNAlertRes
};


typedef enum				/* File menu item numbers */
{
	extract = 1,
	close,
	quit
};


typedef enum 				/* Edit menu item numbers */
{
	undo = 1,
	/* --- */
	cut = 3,
	copy,
	paste,
	clear
};

typedef enum				/* Option menu item numbers */
{
	forceOpt = 1,
	setCr
};

typedef enum				/* dialog item numbers */
{
	okB = 1,
	crText,
	cancelB
};

typedef enum
{
	continueB = 1,
	quitB,
	messageItem
};

MenuHandle	fileM, editM, optM;
DialogPtr	infoDlgPtr, typeDlgPtr;

#endif								/* Think C declarations */

Boolean force = 	false;  		/* force overwriting existing files */
OSType	fdCreator =	'MPS ';			/* Finder Creator */
OSType	fdType =	'TEXT';			/* Finder Type */

void ErrMsg (char *p1, char *p2);
									/* for display errors */
void ConvertFName (char *unixfilename, char *mpwfilename);
									/* for converting Unix->Mac filenames */
void unshar(char *s);				/* the guts of the program */

#ifndef	MPW
/*	Prototypes for standalone version */
void SetDText(DialogPtr dlog, short item, Str255 str);
void GetDText(DialogPtr dlog, short item, StringPtr str);
void GetCreator(void);
void DoFileMenu(short item);
void DoEditMenu(short item);
void DoOptMenu(short item);
void DoAbout(void);
pascal short DirSelHook(short item, DialogPtr theDialog, Ptr dataP);
pascal Boolean DirFilterProc(CInfoPBPtr pb, Ptr dataP);
Boolean GetDir(char *text, FSSpec* dirSpec);
void Extract(void);

// AE handlers
pascal OSErr MyAEQuitApp (const AppleEvent *theAEvt, AppleEvent *replyEvt, long refCon);
pascal OSErr MyAEOpenDoc (const AppleEvent *theAEvt, AppleEvent *replyEvt, long refCon);
pascal void	 MyAEProc (EventRecord *theEvent);			// called by TransSkel
OSErr MyGotRequiredParams(const AppleEvent* Event);

DlgHookYDUPP				dlgHookUPP		= NULL;
FileFilterYDUPP				dlgFilterUPP	= NULL;
static AEEventHandlerUPP	aeQuitProc, aeOpenDocProc;

void SetDText(DialogPtr dlog, short item, Str255 str)
{
Handle	itemHandle;
short	itemType;
Rect	itemRect;

	GetDItem (dlog, item, &itemType, &itemHandle, &itemRect);
	SetIText (itemHandle, str);
}

/* Dialog handler */

void GetDText(DialogPtr dlog, short item, StringPtr str)
{
Handle	itemHandle;
short	itemType;
Rect	itemRect;

	GetDItem (dlog, item, &itemType, &itemHandle, &itemRect);
	GetIText (itemHandle, str);
}


void GetCreator(void)
{
	short	itemHit;
	char	creator[5];

	typeDlgPtr = GetNewDialog (typeDlgRes, NIL, (WindowPtr) -1L);
	BlockMove(&fdCreator, creator+1, 4);
	creator[0] = '\004';
	SetDText(typeDlgPtr, crText, (StringPtr)creator);
	SelectWindow(typeDlgPtr);
	ShowWindow(typeDlgPtr);
	DrawDialog(typeDlgPtr);
	ModalDialog(NIL, &itemHit);
	if (itemHit == okB) {
		GetDText (typeDlgPtr, crText,(StringPtr) &creator);
		BlockMove(creator+1, &fdCreator, 4);
	}
	DisposDialog(typeDlgPtr);
}
/*
	AE handler called by TransSkel
*/
pascal void MyAEProc (EventRecord *theEvent)
{
	// we only care about AEs for which we have handlers
	AEProcessAppleEvent (theEvent);
}


static pascal OSErr MyAEQuitApp (const AppleEvent *theAEvt, AppleEvent *replyEvt, long refCon)
{
	OSErr					OSError;
	
	OSError = MyGotRequiredParams(theAEvt);
	if (OSError == noErr)
		SkelStopEventLoop();	// stop TransSkel
	
	return (OSError);
}


static pascal OSErr MyAEOpenDoc (const AppleEvent *theAEvt, AppleEvent *replyEvt, long refCon)
{
	OSErr		err = noErr;
	AEDesc		argv;
	short		vRefNum;
	long		dirID;

	HGetVol(NIL, &vRefNum, &dirID);
	
	// pick up each arg and extract it
	err = AEGetParamDesc(theAEvt, keyDirectObject, typeAEList, &argv);
	if (err == noErr) {
		long					argc;
		
		err = AECountItems(&argv, &argc);
		if (err == noErr)
		{
			long		bytesNeeded = 0;
			long		index = 0;\
			AEKeyword	keyword;
			DescType	dataType;
			long		actualSize;
			FSSpec		theFSSpec;

			for (index = 1; index <= argc && err == noErr; index++)
			{
				err = AEGetNthPtr(&argv, index, typeFSS,
								  &keyword, &dataType, &theFSSpec,
								  sizeof(theFSSpec), &actualSize);
				if (err == noErr)
				{
					// get current dir
					(void) HSetVol(NIL, theFSSpec.vRefNum, theFSSpec.parID);
					PtoCstr(theFSSpec.name);
					unshar((char *)theFSSpec.name);
				}
			}
		}
		
		(void)AEDisposeDesc(&argv);
		(void) HSetVol(NIL, vRefNum, dirID);
	}
	return (err);
}

static OSErr MyGotRequiredParams(const AppleEvent* Event)
{
	OSErr		err;
	DescType	ReturnedType;
	Size		ActualSize;
	
	err = AEGetAttributePtr(Event, keyMissedKeywordAttr, typeWildCard, &ReturnedType, NULL, 0, &ActualSize);
	if (err == errAEDescNotFound) {
		
		err = noErr;
		
	} else if (err == noErr) {
		
		err = errAEEventNotHandled;
	}
	
	return (err);
}



/*
	File menu handler
*/

void DoFileMenu(short item)
{
WindowPeek	wPeek;

	switch (item)
	{
		case extract:
			Extract();
			break;
		case close:
			if ((wPeek = (WindowPeek) FrontWindow ()) != NIL)
			{
				if (wPeek->windowKind < 0)
					CloseDeskAcc (wPeek->windowKind);
			}
			break;
		case quit:
			SkelStopEventLoop ();
			break;
	}
}


void DoEditMenu(short item)
{
DialogPtr	theDialog;

	theDialog = (DialogPtr) FrontWindow ();
	if (((WindowPeek) theDialog)->windowKind != dialogKind)
		return;

	switch (item)
	{
		case cut:
		{
			DlgCut (theDialog);
			(void) ZeroScrap ();
			(void) TEToScrap ();
			break;
		}

		case copy:
		{
			DlgCopy (theDialog);
			(void) ZeroScrap ();
			(void) TEToScrap ();
			break;
		}

		case paste:
		{
			(void) TEFromScrap ();
			DlgPaste (theDialog);
			break;
		}

		case clear:
		{
			DlgDelete (theDialog);
			break;
		}
	}
}

void DoOptMenu(short item)
{
	switch (item)
	{
		case forceOpt:	force = !force;
						CheckItem(optM, forceOpt, force);
						break;
	 	case setCr:		GetCreator();
	 					break;
	 }
}

/*
	Handle selection of About� item from Apple menu
*/

void DoAbout(void)
{
	(void) Alert (aboutAlrtRes, NIL);
}

Boolean		useCurDir;		/* Set if current dir to be used */

/* Filter procs & dialogue hooks to select directories only in SFGet file */
pascal short DirSelHook(short item, DialogPtr theDialog, Ptr dataP)
{
	if (GetWRefCon((WindowPtr)theDialog) != sfMainDialogRefCon)
		return item;
	if (item == getDir|| item == getCurDir) {
		/* folder selected */
		*(Boolean*)dataP = (item == getCurDir);
		item = sfItemOpenButton;
	}
	return(item);
}

pascal Boolean DirFilterProc(CInfoPBPtr pb, Ptr dataP)
{
	return(!DIRECTORY(*pb));	/* a directory if bit 4 is set */
}
/*
 * GetDir - manage the directory selection dialog
 */
Boolean GetDir(char *text, FSSpec* dirSpec)
{
	Point				where = {-1, -1};
	StandardFileReply	reply = {0};
	CInfoPBRec			pb = {0};
	DialogPtr			dlgP;
	Boolean				useCurDir	= false;
	short				dirVRefNum;
	

	ParamText((StringPtr)text, (StringPtr)"\p", (StringPtr)"\p", (StringPtr)"\p");
	CustomGetFile(dlgFilterUPP,
				  -1,
				  NIL,
				  &reply,
				  gdDlgRes,
				  where,
				  dlgHookUPP,
				  NIL,
				  NIL,
				  NIL,
				  (Ptr)&useCurDir);
	if (reply.sfGood)
	{
		*dirSpec = reply.sfFile;
		if (useCurDir) {
			dirSpec->parID = LMGetCurDirStore();
		}
		else {
			// the reply contains vref, dir ID and name of selected folder
			pb.dirInfo.ioNamePtr	= (StringPtr)&reply.sfFile.name;
			pb.dirInfo.ioVRefNum	= reply.sfFile.vRefNum;
			pb.dirInfo.ioDrDirID	= reply.sfFile.parID;
			PBGetCatInfo(&pb, false);
			dirSpec->parID			= pb.dirInfo.ioDrDirID;
		}
	}
	return reply.sfGood;
}

void Extract(void)
{
	StandardFileReply	reply;
	short				curVRef;
	long				curDir;

	/*
	 * Use the standard file dialog to select the archive.
	 */
	StandardGetFile(NIL, -1, NIL, &reply);
	if (!reply.sfGood)
		return;
	HGetVol(NIL, &curVRef, &curDir);
	HSetVol(NIL, reply.sfFile.vRefNum, reply.sfFile.parID);
	PCSTR(reply.sfFile.name);
	unshar((char*)reply.sfFile.name);
	HSetVol(NIL, curVRef, curDir);
}

main(int argc, char *argv[])
{

	Handle		fTypeH;
	DialogTHndl	dlgH;
	Point		where;
	short		ht, wd;
		
	SkelInit (NIL);
	SkelApple ("\pAbout Unshar�", (SkelMenuSelectProcPtr)DoAbout);

	// init UPPs
	dlgHookUPP		= NewDlgHookYDProc(DirSelHook);
	dlgFilterUPP	= NewFileFilterYDProc(DirFilterProc);
	aeQuitProc		= NewAEEventHandlerProc(MyAEQuitApp);
	aeOpenDocProc	= NewAEEventHandlerProc(MyAEOpenDoc);

	fileM = NewMenu (1000, (StringPtr)"\pFile");
	AppendMenu (fileM, (StringPtr)"\pExtract/O;Close/K;Quit/Q");
	SkelMenu (fileM, (SkelMenuSelectProcPtr)DoFileMenu, NIL, FALSE, FALSE);

	editM = NewMenu (1001, (StringPtr)"\pEdit");
	AppendMenu (editM, (StringPtr)"\p(Undo/Z;(-;Cut/X;Copy/C;Paste/V;Clear");
	SkelMenu (editM, (SkelMenuSelectProcPtr)DoEditMenu, NIL, FALSE, FALSE);

	optM = NewMenu (1002, (StringPtr)"\pOptions");
	AppendMenu (optM, (StringPtr)"\pOverwrite existing files;File type...");
	SkelMenu (optM, (SkelMenuSelectProcPtr)DoOptMenu, NIL, FALSE, TRUE);
	
	// install AEs
	if (SkelQuery (skelQHasAppleEvents))
	{
		/* Apple Events are available */
		if (AEInstallEventHandler (kCoreEventClass, kAEQuitApplication,
								   aeQuitProc, 0L, false) != noErr)
		{
			// croak and die
			ExitToShell();
		}
		if (AEInstallEventHandler (kCoreEventClass, kAEOpenDocuments,
								   aeOpenDocProc, 0L, false) != noErr)
		{
			// croak and die
			ExitToShell();
		}
		SkelSetAEHandler (MyAEProc);		// register with TransSkel
	}
	else
		/* Apple Events are not available */
	{
		ExitToShell();						// die
	}

	// pick up default type and creator
	if ((fTypeH = (char **)GetResource('ftyp', 0)) == NIL) {
		ExitToShell();
	}
	BlockMove(*fTypeH, &fdCreator, 4);
	BlockMove((*fTypeH)+4, &fdType, 4);
	ReleaseResource(fTypeH);
	
	dlgH = (DialogTHndl)GetResource('DLOG', typeDlgRes);
	wd = ((* dlgH)->boundsRect.right)-((* dlgH)->boundsRect.left);
	/* centre the dialogue box on the screen
	   (but how do I know which screen?)
	*/
	where.h = (qd.screenBits.bounds.right-qd.screenBits.bounds.left-wd) / 2;
	(* dlgH)->boundsRect.right += where.h;
	(* dlgH)->boundsRect.left = where.h;
	
	dlgH = (DialogTHndl)GetResource('DLOG', infoDlgRes);
	wd = ((* dlgH)->boundsRect.right)-((* dlgH)->boundsRect.left);
	/* centre the dialogue box on the screen
	   (but how do I know which screen?)
	*/
	where.h = (qd.screenBits.bounds.right-qd.screenBits.bounds.left-wd) / 2;
	(* dlgH)->boundsRect.right += where.h;
	(* dlgH)->boundsRect.left = where.h;

	SkelEventLoop ();
	SkelCleanup ();
}
#else								/* MPW main support routines */

main(int argc, char *argv[])
{
	char	**filelist;				/* list of files to process */
	int		fileCount = 0;			/* no of files to process */	

#ifdef	MPW
	InitCursorCtl(nil);
#endif	MPW
	argc--; argv++;
	if ((filelist = (char **)calloc((size_t)argc, sizeof(Ptr))) == NULL) {
		fprintf(stderr, "### Not enough memory\n");
		exit(-1);
	}
	while (argc) {
		if (argv[0][0] == '-') {
			switch (argv[0][1]) {
			case '\0':
				filelist[fileCount++] = "-";
				break;
			case 'c':
				/* creator is in next arg */
				argc--;
				argv++;
				strncpy((char *)&fdCreator, argv[0], (size_t)4);
				break;
			case 'f':
				force = true;
				break;
			case 't':
				/* type is in next arg */
				argc--;
				argv++;
				strncpy((char *)&fdType, argv[0], (size_t)4);
				break;
			default:
				fprintf(stderr, "### Usage: unshar -f -c creator -t type [files|-]\n");
				exit(-1);
			}
		}
		else
			filelist[fileCount++] = argv[0];
		argc--; argv++;
	}
	/* at this point all files to process are in filelist */
	for (argc = 0; argc < fileCount; argc++) {
		unshar(filelist[argc]);
	}
}
#endif

/*	This part (mostly) common to both MPW and Think C */

void ErrMsg (char *p1, char *p2)
{
#ifdef	MPW
	fprintf (stderr, "### %P %P\n", p1, p2);
#else
    ParamText((StringPtr)p1, (StringPtr)p2, (StringPtr)"\p", (StringPtr)"\p");
    (void)StopAlert(errorAlertRes, NIL);
#endif
}

void ConvertFName (char *unixfilename, char *macfilename)
{
	char	*cp, *tp;
	char	buf[256];
	Boolean	slashSeen = false;
	
	/* make a Mac relative pathname */
	/*	rules of the game:
		'foo' -> :foo
		"foo" -> :foo
		`foo` -> :foo
		./foo -> :foo
		foo/baz -> :foo:baz
		.foo -> :_foo
		.. -> ::
		foo/./baz -> :foo:baz
		/foo/baz -> :foo:baz       because we don't really want to create files at top vol level
		foo:baz -> :foo/baz
		rules apply recursively
	*/
	
	(void) strcpy(macfilename, ":");			/* initialize with a leading colon */
	for (cp = buf, tp = unixfilename; *tp; tp++) {
		switch (*tp) {
			case '\'':
			case '"':
			case '`':	break;					/* delete these chars */
			case '.':	if (*(tp+1) == '/') {	/* delete any occurence of ./ */
							tp++;
						}
						else if (*(tp+1) == '.') {
							*cp++ = ':';
							*cp++ = ':';		/* convert .. to :: */
							tp++;
						}
						else if (cp == buf)
							*cp++ = '_';		/* replace leading dot with _ */
						else
							*cp++ = '.';
						break;					
			case '/':	if (cp != buf)
							*cp++ = ':';		/* replace / with : */
						slashSeen = true;
						break;
			case ':':	*cp++ = '/';			/* replace : with / */
						break;
			default:	*cp++ = *tp;
						break;
		}
	}
	*cp = '\0';
	
	if (slashSeen)
		(void) strcat(macfilename, buf);
	else
		(void) strcpy(macfilename, buf);
	CPSTR(macfilename);
}

void unshar(char* s)
{
  char		buffer[BUFSIZ];
  char		*cp;
  FILE		*infp;
  short		outfp = -1;
  char		unixfilename[256], mpwfilename[256];
  char		*tp, *ts, delim;
  char		terminator[30];
  int		tlen;
  int		line;
  long		dirID, subDirID;
  short		volRef;
  FInfo		fileInfo;
  OSErr		err;
  FSSpec	extractedFSp;
  FSSpec	defDir;

#ifdef	MPW
  if (strcmp(s, "-") == 0) {
  	infp = stdin;
	ErrMsg("\pProcessing std input:","");
  }
  else {
	  infp = fopen(s, "r");
	  CPSTR(s);
	  if (!infp) {
		ErrMsg("\pCould not open file", s);
		exit(-1);
	  }
	  else ErrMsg("\pProcessing", s);
  }
  HGetVol(NIL, &volRef, &dirID);		/* get current dir, vol */
  FSMakeFSSpec(volRef, dirID, "\p", &defDir);
#else	/* Think C */
  infp = fopen(s, "r");
  CPSTR(s);
  if (!infp) {
    ErrMsg((char*)"\pCould not open file", s);
    return;
  }
#endif

  /* skip over news header lines etc. */
  for (line = 1; cp = fgets(buffer, sizeof(buffer), infp); line++)
    if (buffer[0] == '#' || buffer[0] == ':') break;
	
  if (!cp) {
    ErrMsg((char*)"\pCould not locate start of archive in file", s);
    exit(-1);
  }

#ifndef	MPW
  /* Think C version needs to ask user to locate target folder */

  if (!GetDir((char*)"\pin which to put extracted files", &defDir))
	  return;
  
  dirID	= defDir.parID;


  /*
   * Set the default directory as a base to put everything.
   */
  (void)HSetVol(NIL, defDir.vRefNum, dirID);
  
  /* now we should be at the start of the shar archive itself */
  infoDlgPtr = NIL;
  infoDlgPtr = GetNewDialog (infoDlgRes, NIL, (WindowPtr)-1L);
#endif

  //HGetVol((StringPtr)buffer, &volRef, &dID);		/* get current dir, vol */
  while (cp = fgets(buffer, sizeof(buffer), infp)) {
    line++;
#ifdef	MPW
	SpinCursor(-1);
#endif	MPW

    if (buffer[0] == '#' || buffer[0] == ':') continue;    /* comment line */
    if (strncmp(buffer, "exit", 4) == 0) { break; }  /* exit */

	/*	there are 2 types of shar files:
		a) the ones that use "cat"
		b) the ones that use "sed"
	*/
	if (strncmp(buffer, "sed", 3) == 0 ||
		strncmp(buffer, "cat", 3) == 0) {
		  sscanf(buffer, "%*[^>]>%s", unixfilename);
		  /* make Mac relative pathname */
		  ConvertFName(unixfilename, mpwfilename);

		  /* work out the terminating string */
		  sscanf(buffer, "%*[^<]<<%s", terminator);
		  tp = terminator;
		  tlen = strlen(terminator);
		  
		  while (*tp == ' ') tp++;		/* skip whitespace */
		  switch (*tp) {
			  case '\\':	ts = tp + 1;	/* start of term string */
							delim = ' ';
							break;
			  case '"':		ts = tp + 1;
							delim = *tp;
							break;
			  case '\'':	ts = tp + 1;
							delim = *tp;
							break;
			  default:		ts = tp;
							delim = ' ';
							break;				
		  }
		  do {
			  tp++;
		  } while (*tp != '\0' && *tp != delim);
		  *tp = '\0';
		  tlen = tp - ts;
		  FSMakeFSSpec(0, dirID, (StringPtr)mpwfilename, &extractedFSp);
		  err = FSpOpenDF(&extractedFSp, fsWrPerm, &outfp);
 		  if (err == noErr && !force) {
			FSClose(outfp);
#ifdef	MPW
			ErrMsg("\pWill not overwrite existing file", mpwfilename);
#else
            ParamText((StringPtr)"\pWill not overwrite existing file",
                	  (StringPtr)mpwfilename, (StringPtr)"\p", (StringPtr)"\p");
            if (NoteAlert(dupFNAlertRes, NIL) == quitB) break;
#endif
			while (strncmp(buffer, ts, tlen) != 0) {  /* skip to terminating string */
			  fgets(buffer, sizeof(buffer), infp);
			  line++;
			}
		  } else {
			if (outfp != -1) FSClose(outfp);
            if ((err = FSpCreate(&extractedFSp, fdCreator, fdType, smSystemScript)) != noErr)
            {
				if (err != dupFNErr || !force)
				{
					ErrMsg((char*)"\pCouldn't create", mpwfilename);
					exit(-1);
				}
			}
#ifdef MPW
			fprintf(stderr, "  Open \"%P\"\n", mpwfilename);
#else
            ParamText((StringPtr)mpwfilename,
            		  (StringPtr)"\p", (StringPtr)"\p", (StringPtr)"\p");
            SelectWindow(infoDlgPtr);
            ShowWindow(infoDlgPtr);
            DrawDialog(infoDlgPtr);         
#endif
			if (FSpOpenDF(&extractedFSp, fsWrPerm, &outfp) != noErr)
			{
				ErrMsg((char*)"\pCouldn't open", mpwfilename);
				exit(-1);
			}
			
			if (strncmp(buffer, "sed", 3) == 0) {
				fgets(buffer, sizeof(buffer), infp);
				do {
				  long	count;
				  count	= strlen(buffer+1);
#ifdef	THINK_C
				  if (buffer[count] == '\n')
				  	buffer[count] = '\r';
#endif			
				  FSWrite(outfp, &count, (Ptr)buffer+1);
				  fgets(buffer, sizeof(buffer), infp);
				  line++;
#ifdef	MPW
				  SpinCursor(1);
#endif	MPW
				} while (strncmp(buffer, ts, tlen) != 0);
			}
			else {
				/* copy everything up to terminating string to output file */
				fgets(buffer, sizeof(buffer), infp);
				while (strncmp(buffer, ts, tlen) != 0) {
					long	count;
					count	= strlen(buffer);
#ifdef	THINK_C
				    if (count > 0 && buffer[count-1] == '\n')
				  	  buffer[count-1] = '\r';
#endif			
					FSWrite(outfp, &count, (Ptr)buffer);
					fgets(buffer, sizeof(buffer), infp);
					line++;
#ifdef	MPW
					SpinCursor(1);
#endif	MPW
				}
			}
			FSClose(outfp);
#ifndef	MPW
			HideWindow((WindowPtr)infoDlgPtr);
#endif
			/* set file type and creator */
			GetFInfo((StringPtr)mpwfilename,0,&fileInfo);
			fileInfo.fdType = fdType;
			fileInfo.fdCreator = fdCreator;
			SetFInfo((StringPtr)mpwfilename, 0, &fileInfo);
		  }
      } else if (strncmp(buffer, "if test ! -d", 12) == 0 ||
      			 strncmp(buffer, "if `test ! -d", 13) == 0) {
        /* testing to see if a directory is there */
        if (sscanf(buffer, "if test ! -d '%s'", unixfilename) == 1 ||
			sscanf(buffer, "if test ! -d %s", unixfilename) == 1 ||
        	sscanf(buffer, "if `test ! -d %s`", unixfilename) == 1) {
          /* make Mac relative pathname */
		  ConvertFName(unixfilename, mpwfilename);
			
		  /* I wish MPW C had mkdir(), but at least we don't have to do parameter blocks */
		  CPSTR(unixfilename);
		  HGetVol((StringPtr)unixfilename, &volRef, &dirID);
		  err = DirCreate(volRef, dirID, (StringPtr)mpwfilename, &subDirID);
		}
	} else if (strncmp(buffer, "cd ", 3) == 0) {
		if (sscanf(buffer, "cd '%s'", unixfilename) == 1 ||
			sscanf(buffer, "cd %s", unixfilename) == 1) 	{
			/* change the default directory */
			ConvertFName(unixfilename, mpwfilename);
#ifdef MPW
			fprintf(stderr, "###  Change directory to \"%P\"\n", mpwfilename);
#endif
			HSetVol((StringPtr)mpwfilename, 0, 0L);
		    HGetVol((StringPtr)unixfilename, &volRef, &dirID);
		}
	}
  }
  fclose(infp);
}
