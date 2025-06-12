/*
*  Sconf is set up with an array of character string pointers representing
*  keywords which may be found in the input.
*  Each keyword is assigned a state.  When that keyword is found, everything
*  following that keyword up to the next separator is lumped into one string
*  and then processed in the case statement in Sconfile.  Each element of
*  the case is free to do what it needs to do depending on the keyword's
*  parameters.
*
*  For example, in NCSA DataScope, the "vimage" keyword looks like this:
*  vimage=101,400,300
*  When case KVIMAGE is hit, the parameter string contains:  "101,400,300"
*  and sscanf is used to get the values.
*
*/

#include "df.h"

char *index();
#define strchr(A,B) index(A,B)

static unsigned char
			*DSspace;

static int
		lineno,					/* line number in hosts file */
		position,				/* position for scanning string */
		constate,				/* state for config file parser */
		inquote;				/* flag, inside quotes now */

/*
*  information about dataset which we are reading.
*  When told to read and process the info field, these fields are set to 
*  indicate what the results were.
*/
struct {
	int
		sref,					/* return flag for sdg */
		bref,					/* ref #'s for each of these */
		iref,
		pref,
		bxx,bxy,				/* expansion factors for block image */
		ixx,ixy,				/* interpolated image factors */
		pxx,					/* expansion for polar image */
		ashift,
		dleft,
		dtop,					/* viewport for polar image */
		dright,
		dbottom,
		dxsize,					/* sizes for interp image windows */
		dysize;
		
} dsinfo;


/*
*   States for config file reading state machine.
*   One for each type of keyword and some for controlling.
*/

#define	KSDG	101
#define KBLOCK	102
#define KINTERP	103
#define KPOLAR	104
#define KNOTES 	105
#define KPVIEW  106
#define KVIMAGE 107

static char *Skeyw[] = {
		"",	
		"sdg",							/* ? */
		"block",						/* block image */
		"interp",						/* interpolated image */
		"polar",						/* polar image */
		"notes",						/* notebook data  ==5== */
		"pview",						/* viewport for polar */
		"vimage",						/* image size parameters */
		""
	};



/************************************************************************/
/*  Sconf
*   read in the hosts file into our in-memory data structure.
*/
#define MAXSTUFF 512
#define EOF -1

Sconf(cp,touse)
	char *cp;
	int touse;				/*  number of characters which are left to process */
	{
	int c,
		retval;

	*(cp+touse) = EOF;		/* set end of input */

/*
*  Set the default values for the info structure.  They will be
*  changed if there are settings in the file.
*/
	dsinfo.sref = -1;
	dsinfo.bref = -1;
	dsinfo.iref = -1;
	dsinfo.pref = -1;
	dsinfo.dleft = 1;
	dsinfo.dright = -1;					/* illegal settings, so we know if changed */
	dsinfo.dxsize = -1;
	dsinfo.dysize = -1;

	DSspace = (char *) NewPtr(256);				/* get room for gathering stuff */
	if (DSspace == NULL) {
		return(-1);
	}
	position = constate = inquote = lineno = 0;   /* state vars */	

	retval = 0;
	while (!retval) {
		c = *cp++;
		if (c == '#' && !inquote) {
			while (c != EOF && c != '\n' && c != '\r')		/* skip to EOL */
				c = *cp++;
		}
		if (c == '\n' || c == '\r')
			lineno++;
		retval = DScontoken(c);		/* add character to token */
	}

	DisposPtr(DSspace);

	if (retval == EOF)				/* EOF is normal end */
		return(0);
	else
		return(retval);

}


/************************************************************************/
/*  ncstrcmp
*   No case string compare.
*   Only returns 0=match, 1=no match, does not compare greater or less
*   There is a tiny bit of overlap with the | 32 trick, but shouldn't be
*   a problem.  It causes some different symbols to match.
*/
ncstrcmp(sa,sb)
	char *sa,*sb;
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

/************************************************************************/
/* Scontoken
*  tokenize the strings which get passed to Sconfile.
*  Handles quotes and uses separators:  <33, ;:=
*/
DScontoken(c)
	int c;
	{
	int retval;

	if (c == EOF) {
		DSspace[position++] = '\0';
		Sconfile(DSspace);
		return(-1);
	}

	if (!position && DSissep(c))		/* skip over junk before token */
		return(0);

	if (inquote || !DSissep(c)) {

		if (position > 200) {
			return(1);
		}
/*
*  check for quotes, a little mixed up here, could be reorganized
*/
		if (c == '"' ) {
			if (!inquote) {				/* beginning of quotes */
				inquote = 1;
				return(0);
			}
			else
				inquote = 0;			/* turn off flag and drop through */

		}
		else {						
			if (c == '\n') {			/* check for EOL inside quotes */
				return(1);
			}
			DSspace[position++] = c;    /* include in current string */
			return(0);
		}
				
	}

	DSspace[position++] = '\0';

	retval = DSconfile(DSspace);			/* pass the token along */

	position = 0;
	inquote = 0;
	DSspace[0] = '\0';

	return(retval);
}

/************************************************************************/
/*  Sconfile
*   take the characters read from the file and parse them for keywords
*   which require configuration action.
*/
DSconfile(s)
	char *s;
	{
	int i,laststate;
	
	laststate = constate;
	constate = 0;
	
	switch (laststate) {
		case 0:								/* lookup keyword */
			if (!(*s))						/* empty token */
				return(0);


			for (i=1; *Skeyw[i] && ncstrcmp(Skeyw[i],s); i++)		/* search list */
					;
			if (!(*Skeyw[i])) {			/* not in list */
				return(0);				/* don't die - helps backward compatibility */
			}
			constate = 100+i;			/* change to state for keyword */
			break;


		case KSDG:						/* ref for SDG */
			sscanf(DSspace,"%d",&dsinfo.sref);
			break;
			
		case KBLOCK:
			sscanf(DSspace,"%d,%d,%d",&dsinfo.bref,&dsinfo.bxx,&dsinfo.bxy);
			break;
			
		case KINTERP:
			sscanf(DSspace,"%d,%d,%d",&dsinfo.iref,&dsinfo.ixx,&dsinfo.ixy);
			break;
			
		case KPOLAR:
			sscanf(DSspace,"%d,%d",&dsinfo.pref,&dsinfo.pxx);
			break;
			
		case KPVIEW:
			sscanf(DSspace,"%d,%d,%d,%d,%d",
				&dsinfo.ashift,&dsinfo.dleft,&dsinfo.dtop,&dsinfo.dright,&dsinfo.dbottom);
			break;
			
		case KVIMAGE:
			sscanf(DSspace,"%d,%d,%d",&dsinfo.iref,&dsinfo.dxsize,&dsinfo.dysize);
			break;
			
		case KNOTES:
			sscanf(DSspace,"%d",&i);
			break;

		default:
			constate = 0;
			break;
	}


	return(0);
}

/*************************************************************************/
DSissep(c)
	char c;
	{
	if (c == ' ' || c == '=' || c == '\t' || c == '\n' || c == '\r')
		return(1);
		
	return(0);
	
}