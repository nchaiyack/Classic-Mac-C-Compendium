/* cvnetcnv.c - CVNet master member list format converter.
   This program converts CVNet master member list into other data formats
	suitable for importing into a variety of database programs.
	By defining the desired output format in file "cvnetcnv.fmt"
	in the current directory, the program can be customized for
	nearly all kinds of format conversion needs.  Format specification
	files for some common data formats are included in the package.

   The program has been tested on the following machines and operating systems.
	BSD Unix 4.3
	Sun 4 (SunOS Release 4.1)
	NeXT (NeXT OS Release 2.1)
	IBM PC/Clones (MSDOS 5.0)
		Microsoft C (v5.1), Turbo C (v2.0), Turbo C++ (v1.0)
	Macintosh (System 7)
		THINK C

   This program is not copyrighted.  You are free to do whatever
	you want with it.

   Send comments, bug fixes, support for other machines/OS, and additonal
	format specification files to: izumi@pinoko.berkeley.edu.

   Up-to-date version of this program and additional format specification files
	will be available via anonymous FTP from our machine
	"pinoko.berkeley.edu" in directory: /pub/CVNet.

   This program is in no way guaranteed to do anything useful for you.
   More likely, it will distract your attention from your research.

   ---------------------------------------------------------------------------
   Usage (Unix, PC):
	cvnetcnv inputfile outputfile [formatfile]

   "Inputfile" is a file containing the CVNet master member list as obtained
   from Listserv@vm1.yorkU.ca by sending it a one-line message that says:
   GET MEMBER FILE
   You do not have to edit out the e-mail header and little notes at the
   beginning and end of the e-mail text.

   "Outputfile" will contain the result of conversion performed per
   specification in file "cvnetcnv.fmt".

   "Formatfile" may be given as an option when you want to override the
   default use of "cvnetcnv.fmt".

	The Macintosh version (i.e. if MAC is defined) presents standard file
	dialogs, asking you to open your CVNet master file, and how you want to
	save it. You are asked to open your own format file only if it can't find
	"cvnetcnv.fmt".
   ---------------------------------------------------------------------------
   To Compile:
   NOTE -- Please check if the appropriate macro is defined for
	UNIX, MSDOS, MAC or whatever.  These macros should be mutually
	exclusive.  If you define MAC, comment out the others.

   UNIX
	cc -o cvnetcnv cvnetcnv.c

   MSDOS (Uncomment #define MSDOS below. Same for Turbo C++)
	cl cvnetcnv.c (MSC)
	tcc cvnetcnv.c (Turbo C)

   Macintosh THINK C
   	 Uncomment #define MAC below. Use the supplied project file cvnetcnv.�. It
   	 will create an application called CVNetConvert, which is a stand-alone
   	 Macintosh application, suitable for double clicking. CVNetConvert
   	 should work fine on all Macs and Systems 6 and 7, but was only tested on 
   	 Mac IIci with System 7. The only portability problem I can conceive of is
   	 the need for ALRT resource -16405, which is present in Systems 6.07 and 7.
   ---------------------------------------------------------------------------
   To Prepare Conversion Format Specification File:

   The first line of the format file specifies the record format. You may use 
   the following variable names. All other characters not listed below will 
   be written out to the output as is.

	    $L -- last name
	    $F -- first name & initial
	    $I -- institutional affiliation
	    $A -- address (e-mail)
	    $T -- TAB character
	    $N -- NEW LINE character (LF for UNIX, CRLF for MSDOS)
	    $Cnn -- advance to column nn if possible (if not past it already)

    For example, this format line
	    $L$C12$F$C22($I)$C43<$A>,
    produces a file suitable for UNIX "mail/sendmail":
	    Abramov    Israel    (Brooklyn College)   <ixrbc@cunyvm.bitnet>,

    [#### Warning.  Don't send E-mails to everyone on CVNet using the alias file
     you can obtain this way.  That is ABUSE of the database.]

    On the second line, a line beginning with $M may appear optionally. If present,
    it specifies the output file's Macintosh file type and creator (ignored unless 
    the program is running on a Mac).  If not present, the file type/creator
    will be set to that of MacWrite text file.
    
    After the $M line, or on the second line if $M line does not appear,
    a line beginning with $P may appear optionally (which may continue for
    multiple lines).  If present, it will specify the prolog, text placed
    at the beginning of the output file before all address records.
    After this, a line beginning with $E may appear optionally (which
    may continue for multiple lines).  If present, it will specify the
    epilog, text placed at the end of the output file after all
    address records.  
    
    $M, $P, $E must appear in that order if all three are present.
    Not all of them have to be present, e.g., if no prolog is needed
    but epilog is required, $M, $E lines should be entered in that order.
    
    There must not be any blank or extraneous lines between the first line
    and $M.  Similarly, no blank or extraneous lines should be present
    between $M, $P, $E specifications.  However, blank lines are allowed
    within the quoted text for the prolog and epilog. 
    
    $M, $P, $E must appear at column 1 without any leading spaces.

    
	$Mttttcccc -- specify the output file's four-character Macintosh type 
		(e.g. TEXT) and creator (e.g. MACA). The rest of the line is ignored.
	$P^prolog text^ -- Place the text between quote chars at the beginning of
		the output file.
	$E^epilog text^ -- Place the text between quote chars at the end of the
		output file.
    
    The prolog and epilog may extend over many lines. The character that 
    immediately follows $P and $E is used as the "quote" character to 
    mark the range of text for prolog and epilog.  The quote char may be any 
    printable character that is not used in the quoted text for the 
    prolog and epilog. The rest of the line after the terminating quote is 
    ignored. After the optional $M, $P, and $E the rest of the file is ignored.
    
    The QuickDEX.fmt format file produces a Macintosh file
    suitable for use with the Rolodex-like program called QuickDEX:
	    *$NN$F $L$N$I$N$A
	    $MTEXTQDEx
	    $P^
	    *
	    ^
	    $E^*
	    *
	    ^
    The first line specifies the record format, which produces records like this:
	    *
	    NIsrael Abramov
	    Brooklyn College
	    ixrbc@cunyvm.bitnet
    The line beginning with $M specifies the Macintosh file type (TEXT) and creator
    (QDEx) so that QuickDEX will recognize it. The lines beginning with $P and $E 
    specify a prolog and epilog, adding some extra *'s and linefeeds to please QuickDEX.

    Here are some more examples:
    
    Revert to the old format of CVNet master list (but without continuation lines):
		$L$C12$F$C22$I$C43$A
	produces
		Abramov    Israel    Brooklyn College     ixrbc@cunyvm.bitnet

	Tab-separated ASCII format for importing into NeXT SpeedDeX application:
		$L, $F$T$A$T$I
	produces
		Abramov, Israel	ixrbc@cunyvm.bitnet	Brooklyn College
		               ^                   ^
		              TAB                 TAB

	A basic format might demarcate field by quotes and commas:
		"$L", "$F", "$I", "$A"
	produces
		"Abramov", "Israel", "Brooklyn College", "ixrbc@cunyvm.bitnet"

   ---------------------------------------------------------------------------
   Revision history:
   V1.00, 92-06-07, Izumi Ohzawa, izumi@pinoko.berkeley.edu
   V1.01, 92-06-15, Denis_Pelli@isr.syr.edu added support for Macintosh.
   V1.02, 92-06-16, Izumi Ohzawa, added support for Prolog, Epilog.
   V1.03, 92-06-16, Denis Pelli, set type and creator of Mac files
   V1.04, 92-06-17, Izumi & Denis, minor fixups for portability
   V1.05, 92-07-01, Denis fixed bug reported by Larry Thibos, can now open
   					files in any folder.
   v1.06?,4/25/93, Denis changed call to Macintosh SFGetFile() to only list TEXT files.
   					Enhanced fatal error message.
   ---------------------------------------------------------------------------
*/

/* ####### Uncomment the one that is appropriate for your environment ########### */
/* #define UNIX      1 */		/* Sun, NeXT, BSD, and others */
/* #define MSDOS     1 */		/* Microsoft C, Turbo C, C++ for MSDOS */
#define MAC       1 			/* Macintosh THINK C */

#include <stdio.h>
#ifdef UNIX
#include <strings.h>
#endif
#ifdef MSDOS
#include <string.h>
#endif
#ifdef MAC
	#include <string.h>
	#include <StandardFile.h>
	#include <QuickDraw.h>
	#include <Memory.h>
	#include <Fonts.h>
	#include <Dialogs.h>
	#include <Events.h>
	#include <OSEvents.h>
	#include <Files.h>
	#if THINK_C
		#include <pascal.h>		/* c2pstr() */
	#else
		#include <Strings.h>	/* c2pstr() */
	#endif
	EventRecord event;
	void SetFileInfo(char *fileName,OSType fileType,OSType fileCreator);
#endif

#ifdef __STDC__			/* ANSI-C-style prototypes */
#include <stdlib.h>		/* exit() and atoi() */
void usage(void);
void stupid(void);
char *findex(char *str, char ch);
void remove_comma(char *str);
int check_address(char *str);
void outstring(char *str, FILE *fp);
void outchar(int c, FILE *fp);
void write_record(FILE *fp);
#endif

/* Bad Bad Globals */
int linecnt = 0;		/* number of lines including junk for error report */
int recordcnt = 0;		/* number of records processed */
int column = 1;			/* current column position of output */
char *okchar = "!@%.+-_/";	/* non-alphanumerics that are OK in e-mail address */
char *deffile = "cvnetcnv.fmt";	/* default filename for target format specification */
char *fmtfile;			/* either deffile or argv[3] if given */
char fmt[256];			/* output format string read from fmtfile */
char linebuf[256];		/* line buffer for CVNet file */
char fLineBuffer[256];		/* line buffer for format file */
char tmpbuf[256];		/* buffer for processing */
char lastname[128];		/* Storage for last name */
char firstini[128];		/* Storage for first name, initial */
char affiliation[128];		/* Storage for institutional affiliation */
char emaddress[128];		/* Storage for e-mail address */

#ifdef MAC
void SetFileInfo(char *fileName,OSType fileType,OSType fileCreator)
{
	FInfo outFileInfo;
	
	c2pstr(fileName);
	GetFInfo((StringPtr)fileName,0,&outFileInfo);
	outFileInfo.fdType = fileType;
	outFileInfo.fdCreator = fileCreator;
	SetFInfo((StringPtr)fileName,0,&outFileInfo);
	p2cstr((void *)fileName);
}
#endif

/* __STDC__ is a predefined macro for ANSI-standard C.
 * This stuff is needed to increase portability for some obsolete C compilers.
 */

#ifdef __STDC__
void usage(void)
#else
usage()
#endif
{
#ifdef MAC
	    printf("This program asks you to open your CVNet master file,\n");
	    printf("     and how you want to save it.\n");
	    printf("     And, unless there is a format file called \"%s\",\n",deffile);
	    printf("     asks you to open your own format file.\n");
	    fmtfile=deffile;
#else
	printf("Usage: cvnetcnv inputfile outputfile [formatfile]\n");
#endif
	printf("  *This program converts the CVNet master list to another format\n");
	printf("     as specified in file <%s>.\n", fmtfile);
	printf("     Please look at example format specification files with *.fmt\n");
	printf("     extension.\n");
	printf("  *You do not have to edit out the E-mail header or any other\n");
	printf("     text at the beginning and end of input file.\n");
#ifdef MAC
	    printf("Click this window to exit:\n");
	    FlushEvents(everyEvent,0);
	    while(!GetNextEvent(everyEvent,&event));
#endif
	exit(1);
}

#ifdef __STDC__
void stupid(void)
#else
stupid()
#endif
{
printf("##### WARNING !!! WARNING !!! WARNING !!! WARNING !!! #####\n\n");
printf("  Please DO NOT ABUSE the computer readable database you\n");
printf("generate using this program.  Mailing directly from your\n");
printf("computer to all members of CVNet is *ABUSE*!  No matter\n");
printf("how hot a manuscript or how great a product you have, do\n");
printf("not send mails to everyone directly.  If you think you have\n");
printf("some important information for a lot of people, send it\n");
printf("to CVNET@vm1.yorku.ca.  It will not be sent to the members\n");
printf("automatically, however.  The coordinator of CVNet will\n");
printf("review your submission according to the rules developed\n");
printf("over the years as a result of feedback from the members.\n");
printf("  Please also do not give out the database to non-members.\n");
printf("  I wrote this program to give us convenience in finding\n");
printf("the e-mail address of our colleagues.  Abuse of the\n");
printf("database will lead to your expulsion from CVNet,\n");
printf("not to mention that everyone will hate you for it.\n");
printf("###########################################################\n\n");
#ifndef __STDC__
return(0);
#endif
}


/* Sigh. Not all C libraries include index(char *str, char ch) function */
#ifdef __STDC__
char *findex(char *str, char ch)
#else
char *findex(str, ch)
char *str;
char ch;
#endif
{
char *p, *end;
	p = end = str;
	while(*end != '\0') end++;	    /* end points to string terminator */
	while(*p != ch && *p != '\0') p++;  /* p points to char ch if present */
	if(p != end) return (p);
	else {
	    fprintf(stderr, "\nFATAL ERROR: Line %d of input file is missing \"%c\" character:\n",
			linecnt, ch);
	    fprintf(stderr, "%s\n",str);	/* dgp */
	    fprintf(stderr,"\nBEWARE: the output file is incomplete.\n");
#ifdef MAC
		printf("Click this window to exit:\n");
		FlushEvents(everyEvent,0);
		while(!GetNextEvent(everyEvent,&event));
#endif
	    exit(4);
	}
	return (NULL);		/* it shouldn't come here */
}


#ifdef __STDC__
void remove_comma(char *str)
#else
remove_comma(str)
char *str;
#endif
{
static char tmp[128];
char *s, *t;
	s = str;
	t = tmp;
	/* copy string to tmp[] while removing ',' */
	while((*t = *s++) != '\0')
	    if(*t != ',') t++;
	strncpy(str, tmp, 80);		/* copy back to original string */
#ifndef __STDC__
return(0);
#endif
}

/* Check for invalid characters in e-mail address */
/* Non-zero if there is a character that is not supposed to be in address */
#ifdef __STDC__
int check_address(char *str)
#else
int check_address(str)
char *str;
#endif
{
char *p, c;
int i, nc, ok, retcode = 0;
	p = str;
	nc = strlen(okchar);
	while((c = *p++) != '\0') {
	    ok = 0;
	    if(c >= 'a' && c <= 'z') continue;
	    if(c >= 'A' && c <= 'Z') continue;
	    if(c >= '0' && c <= '9') continue;
	    for(i=0; i<nc; i++) {
		if(c == okchar[i]) {
		    ok =1;
		    break;	/* out of for() loop */
		}
	    }
	    if(ok) continue;	/* while loop */
	    else {
		retcode = c;
		break;		/* out of while loop */
	    }
	}
	return(retcode);
}


#ifdef __STDC__
void outstring(char *str, FILE *fp)
#else
outstring(str, fp)
char *str;
FILE *fp;
#endif
{
char *p;
int c;
	p = str;
	while((c = *p++) != '\0') {
	    putc(c, fp);
	    column++;
	}
#ifndef __STDC__
return(0);
#endif
}


#ifdef __STDC__
void outchar(int c, FILE *fp)
#else
outchar(c, fp)
int c;
FILE *fp;
#endif
{
	putc(c, fp);
	column++;	/* GLOBAL column counter */
#ifndef __STDC__
return(0);
#endif
}


/* Writes out each E-mail address record to output file */
#ifdef __STDC__
void write_record(FILE *fp)
#else
write_record(fp)
FILE *fp;
#endif
{
int ch, cmd, cto, i, cend;
char *p;
char *n;
static char cnn[16];
int atoi();
	column = 1;	/* GLOBAL, current column position */
	p = fmt;	/* pointer to format string */
	while((ch = *p++) != '\0') {
	    switch(ch) {
		/* Formatting code for variables. Case-sensitive
		     $L -- last name
		     $F -- first name & initial
		     $I -- institutional affilication
		     $A -- address (e-mail)
		     $T -- TAB character
		     $N -- NEW LINE character (LF for UNIX, CRLF for MSDOS)
		     $Cnn -- advance to column nn if possible (if not past it already)
		 */
		case '$':     /* special formatting code */
			if((cmd = *p) == '\0') break;
			p++;		/* advance ptr only if not at '\0' already */
			switch(cmd) {
			    case 'L':    /* last name */
				outstring(lastname, fp); break;
			    case 'F':    /* first name & initial */
				outstring(firstini, fp); break;
			    case 'I':    /* institutional affiliation */
				outstring(affiliation, fp); break;
			    case 'A':    /* email address */
				outstring(emaddress, fp); break;
			    case 'T':    /* TAB character insert */
				outchar('\t', fp); break;
			    case 'N':    /* NEW LINE character insert */
				outchar('\n', fp); break;
			    case 'C':    /* column advance */
				n = cnn;	/* number buffer for target column pos */
				while((*n = *p) >='0' && *n <='9') {
				    n++; p++;
				}
				*n = '\0';		/* string terminator */
				cto = atoi(cnn);	/* move to cto */
				if(cto < 1 ) cto = 1;
				if((cend = cto-column) > 0) {
				    for(i=0; i<cend; i++)
					outchar(' ', fp);
				}
				else    outchar(' ', fp);
				break;
			    default:  /* This shouldn't happen */
				fprintf(stderr, "\nIllegal variable \"$%c\" in \"%s\".\n",
						cmd, fmtfile);
#ifdef MAC
				    printf("Click this window to exit:\n");
				    FlushEvents(everyEvent,0);
				    while(!GetNextEvent(everyEvent,&event));
#endif
				exit(5);
			} /* end switch(cmd) */
			break;

		default: /* regular ascii */
			outchar(ch, fp);
			break;
	    } /* end switch(ch) */
	} /* end while((ch = *p) ... */

/*
	fprintf(fp, "%s %s (%s) <%s>\n",
			lastname, firstini, affiliation, emaddress);
*/
#ifndef __STDC__
return(0);
#endif
}


#ifdef __STDC__
void main(int argc, char *argv[])
#else
main(argc, argv)
int argc;
char *argv[];
#endif
{
FILE *fpi, *fpf, *fpo;
char *bptr, *eptr;
int c;
int charcnt;
int quote;	/* prolog, epilog quote char */
#ifdef MAC
    Point where={200,100};
    SFReply	reply;
    char string[256];
    /* The convention of storing four characters in a long, and of specifying
    them with single quotes is not part of standard C, but is a standard
    Macintosh extension to C. */
    OSType type='TEXT',creator='MACA';	/* default is MacWrite text file */
    short outputVRefNum,vRefNum;
#endif
		
#ifdef MAC
	    InitGraf(&qd.thePort);
	    InitFonts();
	    InitWindows();
	    InitCursor();
	    argc=3;
#endif
	if(argc != 3 && argc != 4) usage();
	stupid();

	/* Input file -- Original CVNet member list file */
#ifdef MAC
	    printf("Where is the CVNet master list?\n");
	    printf("Click this window to continue:\n");
	    FlushEvents(everyEvent,0);
	    while(!GetNextEvent(everyEvent,&event));
	    SFGetFile(where,NULL,NULL,1,&type,NULL,&reply);
	    if(!reply.good)usage();
	    p2cstr(reply.fName);
	    argv[1]=malloc(1+strlen((char *)reply.fName));
	    strcpy(argv[1],(char *)reply.fName);
	    c2pstr((char *)reply.fName);
		SetVol(NULL,reply.vRefNum);	/* look in that folder */
#endif
	if((fpi = fopen(argv[1], "r")) == NULL) {
	    fprintf(stderr, "Can't open file \"%s\".\n", argv[1]);
#ifdef MAC
		printf("Click this window to exit:\n");
		FlushEvents(everyEvent,0);
		while(!GetNextEvent(everyEvent,&event));
#endif
	    exit(2);
	}

	/* Output file. Check that we are not wiping out a *.fmt file. */
#ifdef MAC
	    sprintf(string,"%s.out",argv[1]);
	    SFPutFile(where,"\pSave new CVNet list as:",c2pstr(string),NULL,&reply);
	    if(!reply.good)usage();
	    p2cstr(reply.fName);
	    argv[2]=malloc(1+strlen((char *)reply.fName));
	    strcpy(argv[2],(char *)reply.fName);
	    c2pstr((char *)reply.fName);
		SetVol(NULL,reply.vRefNum);	/* look in that folder */
	    outputVRefNum=reply.vRefNum;
#endif
	bptr = eptr = argv[2];
	while(*eptr != '\0') eptr++;
	while(*bptr != '.' && *bptr != '\0') bptr++;
	if(bptr != eptr) {
	    if(strncmp(bptr, ".fmt", 4) == 0) {
		fprintf(stderr, "You shouldn't be writing to \"%s\".\n", argv[2]);
#ifdef MAC
		    printf("Click this window to exit:\n");
		    FlushEvents(everyEvent,0);
		    while(!GetNextEvent(everyEvent,&event));
#endif
		exit(1);
	    }
	}
	if((fpo = fopen(argv[2], "w")) == NULL) {
	    fprintf(stderr, "Can't open output file \"%s\".\n", argv[2]);
#ifdef MAC
		printf("Click this window to exit:\n");
		FlushEvents(everyEvent,0);
		while(!GetNextEvent(everyEvent,&event));
#endif
	    exit(3);
	}

	/* Format file.
	 * Read the first few lines.
	 * The rest of the file will be ignored and may be used to describe
	 * the format specified in the file.
	 */
	if(argc == 4) fmtfile = argv[3];	/* optional third argument given for fmt */
	else          fmtfile = deffile;	/* default */
	fpf = fopen(fmtfile, "r");
#ifdef MAC
	    if(fpf==NULL){
		sprintf(string,"Couldn't find \"%s\". Please choose a format file.",fmtfile);
		ParamText(c2pstr(string),NULL,NULL,NULL);
		Alert(-16405,NULL);
	    SFGetFile(where,NULL,NULL,1,&type,NULL,&reply);
		if(!reply.good)usage();
		p2cstr(reply.fName);
		fmtfile=malloc(1+strlen((char *)reply.fName));
		strcpy(fmtfile,(char *)reply.fName);
		c2pstr((char *)reply.fName);
		SetVol(NULL,reply.vRefNum);	/* look in that folder */
		fpf = fopen(fmtfile, "r");
	    }
#endif
	if(fpf != NULL) {

	    /* Read record-format string */
	    if((fgets(fmt, 250, fpf)) == NULL) {
		fprintf(stderr, "Error reading format specification file \"%s\".\n", fmtfile);
#ifdef MAC
		    printf("Click this window to exit:\n");
		    FlushEvents(everyEvent,0);
		    while(!GetNextEvent(everyEvent,&event));
#endif
		exit(1);
	    }
	    fLineBuffer[0]=0;
	    fgets(fLineBuffer, 250, fpf);		/* read next line */
		
	    /* Read optional file type specification */
	    if(strncmp(fLineBuffer, "$M", 2) == 0) {
#ifdef MAC
		    strncpy((char *)&type,&fLineBuffer[2],4);
		    strncpy((char *)&creator,&fLineBuffer[6],4);
#endif
	    	fLineBuffer[0]=0;
	    	fgets(fLineBuffer, 250, fpf);	/* read next line */
	    }
#ifdef MAC
		/* Set the output file's Macintosh type and creator. */
		GetVol(NULL,&vRefNum);
		SetVol(NULL,outputVRefNum);	/* look in that folder */
		SetFileInfo(argv[2],type,creator);
		SetVol(NULL,vRefNum);
#endif

	    /* Read optional prolog into output file */
	    if(strncmp(fLineBuffer, "$P", 2) == 0) {
		quote = fLineBuffer[2];		/* set quote char for prolog */
		bptr = &fLineBuffer[3];
		while(1){
			    while(((c = *bptr++) != quote) && c != '\0')
				putc(c, fpo);
			    if(c == quote) break;
			    if(fgets(fLineBuffer, 250, fpf) == NULL) break;
			    bptr = fLineBuffer;
		}
		fLineBuffer[0]=0;
		fgets(fLineBuffer, 250, fpf);	/* read next line */
	    }
	    /* Prolog done, but keep FILE *fpf open for epilog at the end */
	} else {
	    fprintf(stderr, "Can't open conversion format specification file \"%s\".\n",
				fmtfile);
#ifdef MAC
		printf("Click this window to exit:\n");
		FlushEvents(everyEvent,0);
		while(!GetNextEvent(everyEvent,&event));
#endif
	    exit(1);
	}


	/* The program relies on the presence of "------" to indicate the
	 * start of records.  This could be a bad thing to rely on.
	 */
	while( ( fgets(linebuf, 250, fpi) ) != NULL ) {
	    linecnt++;
	    if(strncmp(linebuf, "-----", 5) == 0)
		break;		/* start of records detected */
	}

	/* ---- Read a line and separate components into proper places ------ */
	while((fgets(linebuf, 250, fpi)) != NULL) {
	    linecnt++;
	    charcnt = strlen(linebuf);
	    if(linebuf[0] == '#' || linebuf[0] == ';' || charcnt == 0)
		continue;	/* blank or comment lines */
	    if(strncmp(linebuf, "-----", 5) == 0)
		break;		/* end of records detected */
	    if(charcnt < 5) {
		fprintf(stderr,"%c", linebuf[0]); fflush(stderr);
		continue;	/* must be AAA, BBB, ... */
	    }

	    /* Clear all storage arrays. */
	    lastname[0] = firstini[0] = affiliation[0] = emaddress[0] = '\0';

	    recordcnt++;	/* only count real address record */
	    /* We've got a reasonable line.  First, get the lastname */
	    findex(linebuf, ',');		/* check if a comma is present */
	    sscanf(linebuf, "%[^,]", lastname);	/* read to before comma */

	    /* Get the firstname plus initial */
	    strncpy(tmpbuf, linebuf, 250);
	    bptr = findex(tmpbuf, ',');
	    while((c = *++bptr) == ' ' || c == '\t');	/* non-space after comma */
	    eptr = findex(tmpbuf, '(');
	    while((c = *--eptr) == ' ' || c == '\t');	/* non-space before '(' */
	    if(eptr >= bptr) {
		eptr++; *eptr = '\0';		/* make sure there is firstname part */
		strncpy(firstini, bptr, 60);
	    } else
		strcpy(firstini, "");		/* no firstname/initial */

	    /* Get institutional affilication enclosed in bracket */
	    strncpy(tmpbuf, linebuf, 250);
	    bptr = findex(tmpbuf, '(');
	    eptr = findex(tmpbuf, ')');
	    bptr++;		/* char after '(' */
	    *eptr = '\0';	/* terminate string at ')' */
	    strncpy(affiliation, bptr, 60);

	    /* Get (1-st part of) E-mail address.  It should also work
	       even if the e-mail address is entirely on the continuation
	       line and there is nothing after the closing bracket.
	     */
	    eptr++;	/* eptr pointing to tmpbuf is still good */
	    sscanf(eptr, "%s", emaddress);	/* read email address to EOL */

	    /* Check for continuation of email address.
	     * We just look for a space character in the first column.
	     */
	    if((c = getc(fpi)) != ' ')
		ungetc(c, fpi);			/* not a continuation line */
	    else {
		ungetc(c, fpi);			/* looks like a continuation line */
		if((fgets(linebuf, 250, fpi)) == NULL) break;	/* get cont. line */
		linecnt++;
		bptr = findex(linebuf, '"');	/* check continuation marker <"> */
		while((c = *++bptr) == ' ' || c == '\t');
		sscanf(bptr, "%s", tmpbuf);	/* 2-nd part of address in tmpbuf */
		strncat(emaddress, tmpbuf, 80); /* Join 2-nd part to 1-st part */
	    } /* end if((c= getc(fpi)..) else{} */

	    /* Remove comma, if present, from Firstname-initial, and
	     * Institutional affiliation
	     */
	    remove_comma(firstini);
	    remove_comma(affiliation);

	    /* Check if e-mail address contains any bad character. */
	    if((c = check_address(emaddress))) {
		fprintf(stderr, "\nLine %d - Warning: strange character \"%c\" in e-mail address:\n",
				linecnt, c);
		fprintf(stderr, "  %s, %s (%s) %s\n", lastname, firstini, affiliation, emaddress);
	    }

	    /* OK, we must now have everything collected for a record.
	     * Output the record in the format desired.
	     */
	    write_record(fpo);

	} /* end while((fgets(linebuf, .... ) */


	/* Read optional epilog from format file into output file */
	if(strncmp(fLineBuffer, "$E", 2) == 0) {
	    quote = fLineBuffer[2];		/* set quote char for epilog */
	    bptr = &fLineBuffer[3];
	    while(1){
			while(((c = *bptr++) != quote) && c != '\0')
			    putc(c, fpo);
			if(c == quote) break;
			if(fgets(fLineBuffer, 250, fpf) == NULL) break;
			bptr = fLineBuffer;
	    }
    	    fLineBuffer[0]=0;
    	    fgets(fLineBuffer, 250, fpf);	/* read next line */
	} 

	fprintf(stderr, "\n%d records processed. Conversion done.\n", recordcnt);
	fclose(fpf);
	fclose(fpi);
	fclose(fpo);
#ifdef MAC
	    printf("Click this window to exit:\n");
	    FlushEvents(everyEvent,0);
	    while(!GetNextEvent(everyEvent,&event));
#endif
}
