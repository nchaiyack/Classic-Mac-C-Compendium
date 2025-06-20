/*
*	Sets.c
*	All code relating to loading, parsing, and saving of sets
*
*****************************************************************
*	NCSA Telnet for the Macintosh								*
*																*
*	National Center for Supercomputing Applications				*
*	Software Development Group									*
*	152 Computing Applications Building							*
*	605 E. Springfield Ave.										*
*	Champaign, IL  61820										*
*																*
*	Copyright (c) 1986-1993,									*
*	Board of Trustees of the University of Illinois				*
*****************************************************************
*  Revisions:
*  7/92		Telnet 2.6:	Code moved here from config.c and maclook.c...  Jim Browne
*/

#ifdef MPW
#pragma segment Sets
#endif

#include <stdio.h>			/* So sprintfs have protos... */
#include <string.h>
#include <ctype.h>

#include "TelnetHeader.h"
#include "macros.proto.h"
#include "wind.h"			/* For WindRec definition */
#include "Sets.proto.h"
#include "telneterrors.h"
#include "prefs.proto.h"
#include "debug.h"
#include "Connections.proto.h"
#include "rsmac.proto.h"

#include "vsdata.h"
#include "vsinterf.proto.h"

//#define	DEBUG_SETS
#ifdef	DEBUG_SETS
#define	sets_debug_print(x) putln(x)
#else
#define sets_debug_print(x)
#endif

ConnInitParams	**SetParams;
SessionPrefs	*SetSessionPtr;
TerminalPrefs	*SetTerminalPtr;

char			*Cspace;

extern WindRec
	*screens;		/* Window Records (VS) for :	Current Sessions */

extern SysEnvRec theWorld;						/* System Environment record */

extern MenuHandle
	myMenus[NMENUS];		/* Menu Handles .... */

#define PORTNUM 37			/* NCSA 2.5: the port variable */

void	SETSunload(void)	{}

/* affirmative() checks a token to see if it is a valid Affirmation string.
	We now get the affirmative strings from the resource */
	
Boolean	affirmative( char *s)
{
	short	i;
	Str255	AffWords;		/* Telnet 2.6: get string resources */

	for (i=0; i<TelInfo->position; i++)
		s[i] = tolower( s[i] );

	for (i=1; i<= AFF_WORDS_COUNT ;i++) 
		{
		GetIndString(AffWords,AFF_WORDS_ID,i);
		p2cstr(AffWords);
		if (!ncstrcmp((char *)AffWords,s))				
		break;										
		}
		
	if (i <= AFF_WORDS_COUNT)
		return(TRUE);
	else
		return(FALSE);
}

/**************************************************************************/
/*  Sissep
*   is the character a valid separator for the hosts file?
*   separators are white space, special chars and :;=
*
*/
Boolean Sissep( char c)
{
	if (c < 33)
		return(1);
	if (c == ':' || c == ';' || c == '=')
		return(TRUE);
	return(FALSE);
}

/************************************************************************/
/*  ncstrcmp
*   No case string compare.
*   Only returns 0=match, 1=no match, does not compare greater or less
*   There is a tiny bit of overlap with the | 32 trick, but shouldn't be
*   a problem.  It causes some different symbols to match.
*/
short ncstrcmp(char *sa, char *sb)
{
	while (*sa && *sa < 33)		/* don't compare leading spaces */
		sa++;
	while (*sb && *sb < 33)
		sb++;

	while (*sa && *sb) {
		if ((*sa != *sb) && ((*sa | 32) != (*sb | 32)))
			return(1);
		sa++;sb++;
	}
	if (!*sa && !*sb)		/* if both at end of string */
		return(0);
	else
		return(1);
}

/* confile() now gets all of its keywords from the string resources, for greater
	ease, flexibility, and overall coolness.  */
short confile( char *s)
{
	short		i, port;
	Boolean		success;
	unsigned	int a,b,c,d;
	int			signedint;
	Str255		Ckeyw;
	char		tempCstring[256];
	
	sets_debug_print(s);
	if (!(*s) )
		return(0);

	switch( TelInfo->CONFstate) {
		case 0:				/* No keyword yet! */
			for (i=0; i<TelInfo->position; i++)
				s[i] = tolower( s[i] );

			for (i=1; i<= SAVE_SET_STRINGS_COUNT ;i++) 
				{
				GetIndString(Ckeyw,SAVE_SET_STRINGS_ID,i);
				p2cstr(Ckeyw);
				if (!ncstrcmp((char *)Ckeyw,s))				
				break;										
				}

			if ( i > SAVE_SET_STRINGS_COUNT ) 
				{
				OperationFailedAlert(BAD_SET_ERR, 0, 0);
				return(1);
				}

			TelInfo->CONFstate=i;

			if (TelInfo->CONFstate==5) {
				SetSessionPtr->bksp = 0;
				TelInfo->CONFstate=0;
				}
			if (TelInfo->CONFstate==6) {
				SetSessionPtr->bksp = 1;
				TelInfo->CONFstate=0;
				}
			break;

		case 1:				/* NAME */
			{ char *p;
			if (NULL == (p = NewPtr(40000))) 		/* is there enough memory? */
				{					/* NOT enough memory for the set! */
				DoError(107 | MEMORY_ERRORCLASS, LEVEL2, NULL);	/* register the error */
				return(-1);
				}
			else
				DisposPtr(p);
			}
			if (TelInfo->CONFactive) {
				success = CreateConnectionFromParams(SetParams);
				SetParams = NULL;
				if (!success) {
					sets_debug_print("ERROR IN OPENING!! ");
					return(42);
					}
				}

			if (SetParams == NULL) {
				SetParams = ReturnDefaultConnInitParams();
				HLockHi((Handle)SetParams);
				HLockHi((Handle)(**SetParams).session);
				SetSessionPtr = *(**SetParams).session;
				HLockHi((Handle)(**SetParams).terminal);
				SetTerminalPtr = *(**SetParams).terminal;
				}
				
			strncpy(tempCstring, s, 255);			/* Move name in */
			CtoPstr(tempCstring);
			BlockMove(tempCstring, (**SetParams).WindowName, tempCstring[0]+1);
			TelInfo->CONFstate=0;
			TelInfo->CONFactive=1;
			break;

		case 2:				/* HOST */
			strncpy(tempCstring, s, 63);			/* Move name in */
			CtoPstr(tempCstring);

			//	Process the hosname string.
			if (ProcessHostnameString((StringPtr)tempCstring, &port))
				SetSessionPtr->port = port;
			
			BlockMove(tempCstring, SetSessionPtr->hostname, tempCstring[0]+1);

			TelInfo->CONFstate=0;
			break;

		case 3:				/* SIZE */
			if ( 4 != sscanf(s, "{%d,%d,%d,%d}", &a, &b, &c,&d) ) {
				sets_debug_print("Error in window size");
				return(2);
				}
			
			(**SetParams).WindowLocation.top=a;
			(**SetParams).WindowLocation.left=b;
			(**SetParams).WindowLocation.bottom=c;
			(**SetParams).WindowLocation.right=d;
			TelInfo->CONFstate=0;
			break;

		case 4:
			if ( 1 != sscanf(s,"%d", &a) ) {
				sets_debug_print("Scrollback needs parameter");
				return(1);
				}
			SetTerminalPtr->numbkscroll = a;
			TelInfo->CONFstate=0;
			break;

		case 5:
			SetSessionPtr->bksp = 0;			
			TelInfo->CONFstate=0;
			break;

		case 6:
			SetSessionPtr->bksp = 1;
			TelInfo->CONFstate=0;
			break;

		case 7:
			setmacro( 0, (unsigned char *) s);
			TelInfo->CONFstate=0;
			break;
		case 8:
			setmacro( 1, (unsigned char *) s);
			TelInfo->CONFstate=0;
			break;
		case 9:
			setmacro( 2, (unsigned char *) s);
			TelInfo->CONFstate=0;
			break;
		case 10:
			setmacro( 3, (unsigned char *) s);
			TelInfo->CONFstate=0;
			break;
		case 11:
			setmacro( 4, (unsigned char *) s);
			TelInfo->CONFstate=0;
			break;
		case 12:
			setmacro( 5, (unsigned char *) s);
			TelInfo->CONFstate=0;
			break;
		case 13:
			setmacro( 6, (unsigned char *) s);
			TelInfo->CONFstate=0;
			break;
		case 14:
			setmacro( 7, (unsigned char *) s);
			TelInfo->CONFstate=0;
			break;
		case 15:
			setmacro( 8, (unsigned char *) s);
			TelInfo->CONFstate=0;
			break;
		case 16:
			setmacro( 9, (unsigned char *) s);
			TelInfo->CONFstate=0;
			break;
		case 17:
			TelInfo->CONFstate=0;		// Now ignored (was commandkeys)
			break;
		case 18:
			if (!strcmp(s,"backspace") )
				SetSessionPtr->bksp = 0;
			else
				SetSessionPtr->bksp = 1;
			TelInfo->CONFstate=0;
			break;
		case 19:
		case 21:
			if ( 1 == sscanf(s,"%d", &a) ) 
				SetTerminalPtr->vtwidth = a;
			TelInfo->CONFstate=0;
			break;
		case 20:
			if (affirmative(s))
				SetSessionPtr->tekclear = 1;
			else
				SetSessionPtr->tekclear = 0;
			TelInfo->CONFstate = 0;
			break;
		case 22:
			if ( 3 != sscanf(s, "{%u,%u,%u}", &a, &b, &c)) {	/* BYU LSC - "%d" changed to "%u" */
				sets_debug_print("Bad Parms to rgb");
				return(2);
				}
			SetTerminalPtr->nfcolor.red = a;
			SetTerminalPtr->nfcolor.green = b;
			SetTerminalPtr->nfcolor.blue = c;
			TelInfo->CONFstate = 0;
			break;			
		case 23:
			if ( 3 != sscanf(s, "{%u,%u,%u}", &a, &b, &c)) {	/* BYU LSC - "%d" changed to "%u" */
				sets_debug_print("Bad Parms to rgb");
				return(2);
				}
			SetTerminalPtr->nbcolor.red = a;
			SetTerminalPtr->nbcolor.green = b;
			SetTerminalPtr->nbcolor.blue = c;
			TelInfo->CONFstate = 0;
			break;			
		case 24:
			if ( 3 != sscanf(s, "{%u,%u,%u}", &a, &b, &c)) {	/* BYU LSC - "%d" changed to "%u" */
				sets_debug_print("Bad Parms to rgb");
				return(2);
				}
			SetTerminalPtr->bfcolor.red = a;
			SetTerminalPtr->bfcolor.green = b;
			SetTerminalPtr->bfcolor.blue = c;
			TelInfo->CONFstate = 0;
			break;			
		case 25:
			if ( 3 != sscanf(s, "{%u,%u,%u}", &a, &b, &c)) {	/* BYU LSC - "%d" changed to "%u" */
				sets_debug_print("Bad Parms to rgb");
				return(2);
				}
			SetTerminalPtr->bbcolor.red = a;
			SetTerminalPtr->bbcolor.green = b;
			SetTerminalPtr->bbcolor.blue = c;
			TelInfo->CONFstate = 0;
			break;
		case 26:		/* Font Name */
			strncpy(tempCstring, s, 63);			/* Move name in */
			CtoPstr(tempCstring);
			BlockMove(tempCstring, &(SetTerminalPtr->DisplayFont[0]), tempCstring[0]+1);
			TelInfo->CONFstate = 0;
			break;
		case 27:		/* Font Size */
			if (1 == sscanf( s, "%d", &a))
				SetTerminalPtr->fontsize = a;
			TelInfo->CONFstate = 0;
			break;
		case 28:		/* number of lines to use for window's editable region */
			if (1 == sscanf( s, "%d", &a))
				SetTerminalPtr->vtheight = a;
			TelInfo->CONFstate = 0;
			break;
		case 29:		/* keystop, XOFF key */
			if (1 == sscanf( s, "%d", &a))
				SetSessionPtr->skey = a;
			TelInfo->CONFstate = 0;
			break;
		case 30:		/* keygo, XON key */
			if (1 == sscanf( s, "%d", &a))
				SetSessionPtr->qkey = a;
			TelInfo->CONFstate = 0;
			break;
		case 31:		/* keyip, kill key */
			if (1 == sscanf( s, "%d", &a))
				SetSessionPtr->ckey = a;
			TelInfo->CONFstate = 0;
			break;
		case 32:		/* cr-map */
			if ((1 == sscanf( s, "%d", &a)) && (a !=0))
				SetSessionPtr->crmap = TRUE;
			else
				SetSessionPtr->crmap = FALSE;
			TelInfo->CONFstate = 0;
			break;
		case 33:					/* BYU 2.4.9 */
			if (affirmative(s))		/* BYU 2.4.9 */
				SetSessionPtr->linemode = TRUE;	/* BYU 2.4.9 */
			else					/* BYU 2.4.9 */
				SetSessionPtr->linemode = FALSE;	/* BYU 2.4.9 */
			TelInfo->CONFstate=0;			/* BYU 2.4.9 */
			break;					/* BYU 2.4.9 */
		case 34:					/* BYU 2.4.9 */
			if (affirmative(s))		/* BYU 2.4.9 */
				SetTerminalPtr->eightbit = TRUE;	/* BYU 2.4.9 */
			else					/* BYU 2.4.9 */
				SetTerminalPtr->eightbit = FALSE;	/* BYU 2.4.9 */
			TelInfo->CONFstate=0;			/* BYU 2.4.9 */
			break;					/* BYU 2.4.9 */
		case 35:					/* BYU */
			(**SetParams).ftpstate = 1;		/* BYU */
			TelInfo->CONFstate=0;			/* BYU */
			break;					/* BYU */
		case 36:	// ignored
			TelInfo->CONFstate=0;
			break;
		case PORTNUM:							/* NCSA 2.5: get the real port # */
			if (1 == sscanf( s, "%d", &a))		/* NCSA */
				SetSessionPtr->port = a;				/* NCSA */
			TelInfo->CONFstate = 0;				/* NCSA */
			break;								/* NCSA */
		case 38:	// translation
			strncpy((char *) SetSessionPtr->TranslationTable, s, 32);
			CtoPstr((char *) SetSessionPtr->TranslationTable);
			TelInfo->CONFstate=0;
			break;
		case 39:	// tekem
			if (1 == sscanf(s, "%d", &signedint))
				SetSessionPtr->tektype = signedint;
			TelInfo->CONFstate=0;
			break;
		case 40:	// answerback
			strncpy((char *) SetTerminalPtr->AnswerBackMessage, s, 32);
			CtoPstr((char *) SetTerminalPtr->AnswerBackMessage);
			TelInfo->CONFstate=0;
			break;
		default:
			TelInfo->CONFstate=0;
		}
	return(0);
  } /* confile */

/************************************************************************/
/* contoken
*  tokenize the strings which get passed to confile.
*  Handles quotes and uses separators:  <33, ;:=
*/ 
short contoken( char c)
{
	short		retval;
	Boolean		success;

	if (c == EOF) {
		Cspace[TelInfo->position++] = '\0';
		sets_debug_print("Eof handler called");
		confile(Cspace);
		if (TelInfo->CONFactive) {
				success = CreateConnectionFromParams(SetParams);
				if (!success) {
					sets_debug_print("ERROR IN OPENING!! ");
					return(42);
					}
				}
		return(-1);
	}
	
	if (!TelInfo->position && !TelInfo->inquote && Sissep(c))
	/*if (!TelInfo->position && Sissep(c))	*/	/* old_skip over junk before keyword */
		return(0);

	if (TelInfo->inquote || !Sissep(c)) {

		if (TelInfo->position > 200) {
			sets_debug_print("Out of bounds error!");
			return(1);
		}
/*
*  check for quotes, a little mixed up here, could be reorganized
*/
		if (c == '"' ) {
			if (!TelInfo->inquote) {			/* beginning of quotes */
				TelInfo->inquote = 1;
				return(0);
			}
			 Cspace[TelInfo->position++] =c;
			return(0);
		}
		else 
			{						/* include in current string */
			if (c != '\012' && c != '\015')		/* BYU 2.4.18 - changed \n to \015 and added \012 */
				{
				Cspace[TelInfo->position++] = c;
				return(0);
				}
			}
				
		}

	if (Cspace[TelInfo->position-1] == '"') TelInfo->position--;
	Cspace[TelInfo->position++] = '\0';

	retval = confile(Cspace);			/* pass the token along */

	TelInfo->position = 0;
	TelInfo->inquote = 0;
	Cspace[0] = '\0';

	return(retval);
}

/************************************************************************/
/*  readconfig
*   read the saved set file into our in-memory data structure.
*   Handle everything by keyword (stored in resources).
*/
void readconfig(FSSpec theSet)
{
	short c,retval;
	short fn;
	OSErr err;

	Cspace = NewPtr(256);				/* BYU LSC - get room for gathering stuff */

	SetParams = ReturnDefaultConnInitParams();
	HLockHi((Handle)SetParams);
	HLockHi((Handle)(**SetParams).session);
	SetSessionPtr = *(**SetParams).session;
	HLockHi((Handle)(**SetParams).terminal);
	SetTerminalPtr = *(**SetParams).terminal;

	if (NULL == Cspace) 				/* no memory left for the set to load in */
		{								/* we're out of memory */
		DoError(107 | MEMORY_ERRORCLASS, LEVEL2, NULL);	
		return;
		}
	
	TelInfo->position = TelInfo->CONFstate = TelInfo->CONFactive = TelInfo->inquote = TelInfo->lineno = 0;   /* state vars */	

	err = HOpen(theSet.vRefNum, theSet.parID, theSet.name, fsRdPerm, &fn);

	retval = 0;
	while (!retval) {
		c = Myfgetc(fn);
		if (c == '#' && !TelInfo->inquote) {
			while (c != EOF && c != '\012' && c != '\015')		/* skip to EOL */	/* BYU 2.4.18 - changed \n to \015 and added \012*/
				c = Myfgetc(fn);
		}
		if (c == '\012' || c == '\015')			/* BYU 2.4.18 - changed \n to \015 and added \012 */
			TelInfo->lineno++;
		retval = contoken(c);
	}

	FSClose(fn);
	DisposPtr((Ptr) Cspace);

	if (retval == EOF) {				/* EOF is normal end */
		sets_debug_print("EOF termination");
		}
	else {
		sets_debug_print("NON-EOF termination");
		}
		
	return;
}

void LoadSet( void)
{
	SFReply		sfr;
	long		junk;
	SFTypeList	typesok = {'CONF'};
	Point		where;
	FSSpec		set;

	where.h=100;where.v=100;
#ifndef MPW
	SFGetFile( where, "\pSet to load:", 0L, 1, typesok, 0L, &sfr);
#else
	SFGetFile( where, "Set to load:", 0L, 1, typesok,
					0L, &sfr);
#endif MPW

	if (! sfr.good) return;

	BlockMove(&sfr.fName, set.name, (*sfr.fName)+1); // pstring copy sfr.fName -> set.name
	GetWDInfo(sfr.vRefNum, &set.vRefNum, &set.parID, &junk);
	readconfig(set);
}

char Myfgetc(short myfile)
{
	OSErr err;
	long count;
	unsigned char buffer;
	
	count = 1;
	if ((err = FSRead(myfile, &count, &buffer)) == eofErr)
		buffer = EOF;
	
	return (buffer);
}

void CStringToFile(short myfile, unsigned char *mystring) 
{	
	long mycount;										/* BYU LSC */
	short fstatus;										/* BYU LSC */
  
  mycount = strlen((char *) mystring);				/* BYU LSC */
  fstatus = FSWrite(myfile,&mycount,mystring);		/* BYU LSC */
}


void SaveSet( void)
{
	SFReply		sfr;
	short		fn, truncate;
	WindowPeek	wpeek;
	Rect		rect;
	Point		where;
	long		junk;
	char		temp[256], temp2[256];			/* BYU LSC */
	unsigned short	red, green, blue;
	short			fnum,fsiz;
	short			i;
	FSSpec		set;
	OSErr		err;
	Str255		scratchPstring;
	
	where.h = 100; where.v = 100;
#ifndef MPW
	SFPutFile( where, "\pSave Set to:", "\pTelnet Set", 0L, &sfr);	/* BYU LSC */
#else
	SFPutFile( where, "Save Set to:", "Telnet Set", 0L, &sfr);
#endif

	if (! sfr.good)
		return;

	BlockMove(&sfr.fName, set.name, (*sfr.fName)+1); // pstring copy sfr.fName -> set.name
	GetWDInfo(sfr.vRefNum, &set.vRefNum, &set.parID, &junk);

	if (err = HCreate(set.vRefNum, set.parID, set.name, creator, filetype) == dupFNErr)
		truncate = 1;
		
	err = HOpen(set.vRefNum, set.parID, set.name, fsWrPerm, &fn);

	if (truncate) 
		SetEOF(fn, 0L);

	if (gApplicationPrefs->CommandKeys)
		CStringToFile(fn,(unsigned char *) "commandkeys = yes\015");	/* BYU LSC */
	else
		CStringToFile(fn,(unsigned char *) "commandkeys = no\015");		/* BYU LSC */

	for (i = 0; i < 10; i++)
	  {
		getmacro(i, (unsigned char *) temp);			/* BYU LSC */
		if (*temp) {									/* BYU LSC */
			sprintf(temp2, "key%d = \"", i);			/* BYU 2.4.16 */
			strcat(temp2,temp);							/* BYU LSC */
			strcat(temp2,"\"\015");						/* BYU LSC */
			CStringToFile(fn,(unsigned char *) temp2);	/* BYU LSC */
		}												/* BYU LSC */
	  } /* for */

#if 0													/* BYU LSC */
	for (i = 0; i < TelInfo->numwindows; i++)
	  {
		short j;
		j = RSgetfont(screens[i].vs, &fnum, &fsiz);
	  } /* for */
#endif													/* BYU LSC */

	for (i = 0; i < TelInfo->numwindows; i++)
	  {
	  	GetWTitle(screens[i].wind, scratchPstring);
	  	PtoCstr(scratchPstring);
		sprintf(temp2, "name= \"%s\"\015", scratchPstring);
		CStringToFile(fn,(unsigned char *) temp2);				/* BYU LSC */

		BlockMove((Ptr)screens[i].machine, (Ptr)scratchPstring, Length(screens[i].machine)+1);
		PtoCstr(scratchPstring);
		sprintf(temp2, "host= \"%s\"\015", scratchPstring);
		CStringToFile(fn,(unsigned char *) temp2);					/* BYU LSC */

		if (screens[i].ftpstate != 0)							/* BYU 2.4.15 */
			CStringToFile(fn,(unsigned char *)  "ftp= yes\015");	/* BYU mod */

		sprintf (temp2,"port= %d\015",screens[i].portNum);	/* NCSA: save port # */
		CStringToFile(fn,(unsigned char *) temp2);						/* BYU LSC */

		sprintf(temp2, "scrollback= %d\015", (screens[i].maxscroll));	/* BYU LSC */
		CStringToFile(fn,(unsigned char *) temp2);						/* BYU LSC */

		if (screens[i].bsdel)
			CStringToFile(fn,(unsigned char *)  "erase = delete\015");		/* BYU LSC */
		else
			CStringToFile(fn,(unsigned char *)  "erase = backspace\015");	/* BYU LSC */

		wpeek = (WindowPeek) screens[i].wind;
		rect = (*wpeek->contRgn)->rgnBBox;

		sprintf(temp2, "size = {%d,%d,%d,%d}\015", rect.top, rect.left,	/* BYU LSC */
					rect.bottom, rect.right);
		CStringToFile(fn,(unsigned char *) temp2);						/* BYU LSC */

		sprintf(temp2, "vtwidth = %d\015", screens[i].width);			/* BYU LSC */
		CStringToFile(fn,(unsigned char *) temp2);						/* BYU LSC */

		if (screens[i].tekclear)
			CStringToFile(fn,(unsigned char *) "tekclear = yes\015");	/* BYU LSC */
		else
			CStringToFile(fn,(unsigned char *) "tekclear = no\015");	/* BYU LSC */

		if (theWorld.hasColorQD)
		  {
			short j;
			for (j = 0; j < 4; j++)
			  {
				RSgetcolor( screens[i].vs, j, &red, &green, &blue);
				sprintf(temp2, "rgb%d = {%u,%u,%u}\015",
					j, red, green, blue);
				CStringToFile(fn,(unsigned char *) temp2);
			  } /* for j */
		  } /* if */
		RSgetfont( screens[i].vs, &fnum, &fsiz);
		GetFontName( fnum, (StringPtr)temp);									/* BYU LSC */
#ifndef MPW
		p2cstr((unsigned char *) temp);								/* BYU LSC */
#endif

		sprintf( temp2, "font = \"%s\"\015", temp);					/* BYU LSC */
		CStringToFile(fn,(unsigned char *) temp2);					/* BYU LSC */
		sprintf( temp2, "fsize= %d\015", fsiz);						/* BYU LSC */
		CStringToFile(fn,(unsigned char *) temp2);					/* BYU LSC */
		
		sprintf( temp2, "nlines= %d\015", VSgetlines(screens[i].vs));/* BYU LSC */
		CStringToFile(fn,(unsigned char *) temp2);					/* BYU LSC */
		sprintf( temp2, "keystop= %d\015", screens[i].TELstop);		/* BYU LSC */
		CStringToFile(fn,(unsigned char *) temp2);					/* BYU LSC */
		sprintf( temp2, "keygo= %d\015", screens[i].TELgo);			/* BYU LSC */
		CStringToFile(fn,(unsigned char *) temp2);					/* BYU LSC */
		sprintf( temp2, "keyip= %d\015", screens[i].TELip);			/* BYU LSC */
		CStringToFile(fn,(unsigned char *) temp2);					/* BYU LSC */
		sprintf( temp2, "crmap= %d\015", screens[i].crmap);		/* BYU LSC */
		CStringToFile(fn,(unsigned char *) temp2);					/* BYU LSC */
		sprintf( temp2, "tekem= %d\015", screens[i].tektype);
		CStringToFile(fn,(unsigned char *) temp2);
		if (screens[i].national) {						// Don't do this if using default translation table
			GetItem(myMenus[National], screens[i].national+1, scratchPstring);
			PtoCstr(scratchPstring);
			sprintf(temp2, "translation= \"%s\"\015", scratchPstring);
			CStringToFile(fn, (unsigned char *)temp2);
			}
		BlockMove(screens[i].answerback, scratchPstring, *(screens[i].answerback)+1);
		PtoCstr(scratchPstring);
		sprintf(temp2, "answerback= \"%s\"\015", scratchPstring);
		CStringToFile(fn, (unsigned char *)temp2);
	  } /* for i */

	FSClose(fn);						/* BYU LSC */
}

