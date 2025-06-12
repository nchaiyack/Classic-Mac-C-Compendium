/*
	File:		unshar.c

	Contains:	unshar for MPW that's good enough for
				comp.sources.unix archives

	Written by:	sw
				Network Analysis Ltd
				178 Wainbody Ave South
				Coventry CV3 6BX
				UK
	Phone:		+44 203 419996
	E-mail:		sw@network-analysis-ltd.co.uk
	
	Copyright:	Public domain

	Change History (most recent first):

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
#include <Files.h>
#include <CursorCtl.h>
#include <StdLib.h>
#endif	MPW
#include <errno.h>
#include <stdio.h>
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
/*	for Think C
*/

/* if using new Think C headers, uncomment the following line, otherwise
   use the line after that. Only one of the following pair of lines should be used
*/
#define	LOWMEM_LONG(lowMemVar)	(*((long *)lowMemVar))		/* use this if new-style TC headers */
/*
#define LOWMEM_LONG(lowMemVar)	(lowMemVar)					/* use this if old-style TC headers */

#define DIRECTORY(pb)	(((pb).dirInfo.ioFlAttrib & 0x10) == 0x10)
#define	getDir	11			/* buttons in the dialogue box */
#define	getCurDir	12
#define	GD_PROMPT	13

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
short		dirVRefNum = 0;
char		*progname;
short		appFileCount,
			whatToDo;
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
/*	Prototypes for Think C standalone version */
void SetDText(DialogPtr dlog, int item, Str255 str);
void GetDText(DialogPtr dlog, int item, StringPtr str);
void GetCreator(void);
void DoFileMenu(int item);
void DoEditMenu(int item);
void DoOptMenu(int item);
void DoAbout(void);
pascal short DirSelHook(int item, DialogPtr theDialog);
pascal Boolean DirFilterProc(CInfoPBPtr pb);
long GetDir(char *text);
void GetNextFile(SFReply *fInfoPtr);
void Extract(void);


void SetDText(DialogPtr dlog, int item, Str255 str)
{
Handle	itemHandle;
int		itemType;
Rect	itemRect;

	GetDItem (dlog, item, &itemType, &itemHandle, &itemRect);
	SetIText (itemHandle, str);
}

/* Dialog handler */

void GetDText(DialogPtr dlog, int item, StringPtr str)
{
Handle	itemHandle;
int		itemType;
Rect	itemRect;

	GetDItem (dlog, item, &itemType, &itemHandle, &itemRect);
	GetIText (itemHandle, str);
}


void GetCreator(void)
{
	int		itemHit;
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
	File menu handler
*/

void DoFileMenu(int item)
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
			SkelWhoa ();
			break;
	}
}


void DoEditMenu(int item)
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

void DoOptMenu(item)
int		item;
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
	Handle selection of AboutÉ item from Apple menu
*/

void DoAbout(void)
{
	(void) Alert (aboutAlrtRes, NIL);
}

Boolean		useCurDir;		/* Set if current dir to be used */

/* Filter procs & dialogue hooks to select directories only in SFGet file */
pascal short DirSelHook(int item, DialogPtr theDialog)
{
	if (item == getDir|| item == getCurDir) {
		/* folder selected */
		useCurDir = item == getCurDir;
		item = getOpen;
	}
	return(item);
}

pascal Boolean DirFilterProc(CInfoPBPtr pb)
{
	return(!DIRECTORY(*pb));	/* a directory if bit 4 is set */
}
/*
 * GetDir - manage the directory selection dialog
 */
long GetDir(char *text)
{
	char		*routine = "\pGetDir";
	Point		where;
	short		ht, wd;
	SFReply		reply = {0};
	CInfoPBRec	pb;
	DialogPtr	dlgP;
	long		dirDirID=0;		/* Selected directory DirID */

	if ((dlgP = GetNewDialog(gdDlgRes, NIL,(WindowPtr) -1)) == NIL) {
		return(TRUE);
	}
	wd = (dlgP->portRect.right)-(dlgP->portRect.left);
	ht = (dlgP->portRect.bottom)-(dlgP->portRect.top);
	/* centre the dialogue box on the screen
	   (but how do I know which screen?)
	*/
	where.h = (screenBits.bounds.right-screenBits.bounds.left-wd) / 2;
	where.v = (screenBits.bounds.bottom-screenBits.bounds.top-ht) / 2;
	ParamText((StringPtr)text, (StringPtr)"\p", (StringPtr)"\p", (StringPtr)"\p");
	SFPGetFile (where,
				(StringPtr)text,
				(FileFilterProcPtr) DirFilterProc,
				-1, NIL,
				(DlgHookProcPtr) DirSelHook,
				&reply,
				gdDlgRes, NIL);
	if (reply.good) {
		dirVRefNum = reply.vRefNum;
		if (useCurDir) {
			dirDirID = *((long *)CurDirStore);
		}
		else {
			dirDirID = (long)(reply.fType);
		}
	}
	return(dirDirID);
}

void GetNextFile(SFReply *fInfoPtr)
{	AppFile	nextFile;
	static	short	idx=1;

	GetAppFiles(idx++, &nextFile);
	fInfoPtr->vRefNum = nextFile.vRefNum;
	fInfoPtr->fType = nextFile.fType;
	fInfoPtr->version = nextFile.versNum;
	BlockMove(nextFile.fName, fInfoPtr->fName, (int)(nextFile.fName[0]+1));
	appFileCount--;
}

void Extract(void)
{
	Point		where;
	SFReply		reply;

	while (1){
		if (appFileCount > 0) GetNextFile(&reply);
		else {
			/*
			 * Use the standard file dialog to select the archive.
			 */
			where.h = where.v = 75;
			SFGetFile(where, (StringPtr)"\pSelect shar file", NIL, -1, NIL, NIL, &reply);
			if (!reply.good)
				return;
		}
	
		/*
		 * Remember the VRefNum and Name for OpenArchive.
		 * Find out where to put the extracted files.
		 */
		(void) SetVol(NIL, reply.vRefNum);
		PtoCstr((char *)reply.fName);
		unshar((char *)reply.fName);
		(void) SetVol(NIL, reply.vRefNum);
	}
}

main(int argc, char *argv[])
{

	Handle		fTypeH;
	DialogTHndl	dlgH;
	Point		where;
	short		ht, wd;
		
	SkelInit (3, NIL);
	SkelApple ("\pAbout UnsharÉ", DoAbout);

	fileM = NewMenu (1000, (StringPtr)"\pFile");
	AppendMenu (fileM, (StringPtr)"\pExtract/O;Close/K;Quit/Q");
	SkelMenu (fileM, DoFileMenu, NIL, FALSE);

	editM = NewMenu (1001, (StringPtr)"\pEdit");
	AppendMenu (editM, (StringPtr)"\p(Undo/Z;(-;Cut/X;Copy/C;Paste/V;Clear");
	SkelMenu (editM, DoEditMenu, NIL, FALSE);

	optM = NewMenu (1002, (StringPtr)"\pOptions");
	AppendMenu (optM, (StringPtr)"\pOverwrite existing files;File type...");
	SkelMenu (optM, DoOptMenu, NIL, TRUE);
	
	CountAppFiles (&whatToDo, &appFileCount);
	if ((fTypeH = (char **)GetResource('ftyp', 0)) == NIL) {
		ExitToShell();
	}
	BlockMove(*fTypeH, &fdCreator, 4);
	BlockMove((*fTypeH)+4, &fdType, 4);
	ReleaseResource(fTypeH);
	
	CouldDialog(typeDlgRes);
	CouldDialog(infoDlgRes);
	dlgH = (DialogTHndl)GetResource('DLOG', typeDlgRes);
	wd = ((* dlgH)->boundsRect.right)-((* dlgH)->boundsRect.left);
	/* centre the dialogue box on the screen
	   (but how do I know which screen?)
	*/
	where.h = (screenBits.bounds.right-screenBits.bounds.left-wd) / 2;
	(* dlgH)->boundsRect.right += where.h;
	(* dlgH)->boundsRect.left = where.h;
	
	dlgH = (DialogTHndl)GetResource('DLOG', infoDlgRes);
	wd = ((* dlgH)->boundsRect.right)-((* dlgH)->boundsRect.left);
	/* centre the dialogue box on the screen
	   (but how do I know which screen?)
	*/
	where.h = (screenBits.bounds.right-screenBits.bounds.left-wd) / 2;
	(* dlgH)->boundsRect.right += where.h;
	(* dlgH)->boundsRect.left = where.h;

	/* if launched with a set of files, extract them first before asking
	   the user for more
	*/
	if (appFileCount > 0)
		Extract();
	SkelMain ();
	SkelClobber ();
	FreeDialog(typeDlgRes);
	FreeDialog(infoDlgRes);
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
	fprintf (stderr, "### %s %s\n", p1+1, p2);
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
		'foo' -> foo
		"foo" -> foo
		`foo` -> foo
		./foo -> foo
		foo/baz -> :foo:baz
		.foo -> _foo
		foo/./baz -> :foo:baz
		/foo/baz -> :foo:baz
		foo:baz -> foo/baz
		
		rules apply recursively
	*/
	
	(void) strcpy(macfilename, ":");			/* initialize with a leading colon */
	for (cp = buf, tp = unixfilename; *tp; tp++) {
		switch (*tp) {
			case '\'':
			case '"':
			case '`':	break;					/* delete these chars */
			case '.':	if (*(tp+1) == '/') {
							tp++;
						}
						else if (cp == buf)
							*cp++ = '_';		/* replace leading dot with _ */
						else
							*cp++ = '.';
						break;					/* delete any occurence of ./ */
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

}

void unshar(char *s)
{
  char		buffer[BUFSIZ];
  char		*cp;
  FILE		*infp, *outfp;
  char		unixfilename[256], mpwfilename[256];
  char		*tp, *ts, delim;
  char		terminator[30];
  int		tlen;
  int		line;
  long		dirID, dID;
  short		v;
  FInfo		fileInfo;
#ifndef	MPW
  WDPBRec	wdpb;
  char		lineNo[10];
#endif

#ifdef	MPW
  if (strcmp(s, "-") == 0) {
  	infp = stdin;
	ErrMsg("\pProcessing std input:","");
  }
  else {
	  infp = fopen(s, "r");
	  if (!infp) {
		ErrMsg("\pCould not open file", s);
		exit(-1);
	  }
	  else ErrMsg("\pProcessing", s);
  }
#else	/* Think C */
  infp = fopen(s, "r");
  CtoPstr(s);
  if (!infp) {
    ErrMsg("\pCould not open file", s);
    return;
  }
#endif

  /* skip over news header lines etc. */
  for (line = 1; cp = fgets(buffer, sizeof(buffer), infp); line++)
    if (buffer[0] == '#' || buffer[0] == ':') break;
	
  if (!cp) {
    ErrMsg("\pCould not locate start of archive in file", s);
    exit(-1);
  }

#ifndef	MPW
  /* Think C version needs to ask user to locate target folder */

  /*
   * Open the target directory as a base to put everything.
   */
  if ((dirID = GetDir("\pin which to put extracted files")) == 0L)
	  return;
  
  dID = dirID;			/* dID may be changed if subfolders are created */
  (void)HSetVol(NIL, dirVRefNum, dirID);
  
  /* now we should be at the start of the shar archive itself */
  infoDlgPtr = NIL;
  infoDlgPtr = GetNewDialog (infoDlgRes, NIL, (WindowPtr)-1L);
#endif

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

		  outfp = fopen(mpwfilename, "r");
		  if (outfp && !force) {
			fclose(outfp);
#ifdef	MPW
			ErrMsg("\pWill not overwrite existing file", mpwfilename);
#else
         	CtoPstr(mpwfilename);
            ParamText((StringPtr)"\pWill not overwrite existing file",
                	  (StringPtr)mpwfilename, (StringPtr)"\p", (StringPtr)"\p");
            if (NoteAlert(dupFNAlertRes, NIL) == quitB) break;
#endif
			while (strncmp(buffer, ts, tlen) != 0) {  /* skip to terminating string */
			  fgets(buffer, sizeof(buffer), infp);
			  line++;
			}
		  } else {

			if (outfp) fclose(outfp);
#ifdef MPW
			fprintf(stderr, "  Open \"%s\"\n", mpwfilename);
#else
            CtoPstr(mpwfilename);
            Create((StringPtr)mpwfilename, dID, fdCreator, fdType);
            ParamText((StringPtr)mpwfilename,
            		  (StringPtr)"\p", (StringPtr)"\p", (StringPtr)"\p");
            SelectWindow(infoDlgPtr);
            ShowWindow(infoDlgPtr);
            DrawDialog(infoDlgPtr);         
            PtoCstr(mpwfilename);
#endif
			outfp = fopen(mpwfilename, "w");
			
			if (strncmp(buffer, "sed", 3) == 0) {
				fgets(buffer, sizeof(buffer), infp);
				do {
				  fputs(buffer+1, outfp);
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
					fputs(buffer, outfp);
					fgets(buffer, sizeof(buffer), infp);
					line++;
#ifdef	MPW
					SpinCursor(1);
#endif	MPW
				}
			}
			fclose(outfp);
#ifndef	MPW
			HideWindow((WindowPtr)infoDlgPtr);
#endif
			/* set file type and creator */
			CPSTR(mpwfilename);
			GetFInfo((StringPtr)mpwfilename,0,&fileInfo);
			fileInfo.fdType = fdType;
			fileInfo.fdCreator = fdCreator;
			SetFInfo((StringPtr)mpwfilename, 0, &fileInfo);
			PCSTR(mpwfilename);
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
		  CPSTR(mpwfilename);
		  HGetVol((StringPtr)unixfilename, &v, &dirID);
		  DirCreate(v, dirID, (StringPtr)mpwfilename, &dID);
		}
	}
  }
  fclose(infp);
}
