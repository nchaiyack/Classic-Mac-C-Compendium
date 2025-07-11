/**********************************************************************/
/**************************** dviinit.h ********************************/
/**********************************************************************/
/* -*-C-*- dviinit.h */
/*-->dviinit*/
/**********************************************************************/
/****************************** dviinit *******************************/
/**********************************************************************/
#include "dvihead.h"
#include "commands.h"
#include "gendefs.h"
#include "gblprocs.h"
#include "egblvars.h"
#include "m72.h"
#include "mac-specific.h"


void
dviinit(filestr)		/* initialize DVI file processing */
char *filestr;			/* command line filename string  */
{
    int i;			/* temporary */
    INT16 k;			/* index in curpath[] and curname[] */
    char* tcp;			/* temporary string pointers */
    char* tcp1;			/* temporary string pointers */

#if    OS_UNIX
/***********************************************************************
Unix allows binary I/O on stdin and stdout; most other operating systems
do not.  In order  to support this,  we use the  convention that a  null
filestr (NOT  a (char*)NULL)  means the  dvifile is  on stdin,  and  the
translated output is on stdout.  Error logging will be to dvixxx.err  or
dvixxx.dvi-err.  Because random access is required of the dvifile, stdin
may NOT be a pipe; it must be a disk file.
***********************************************************************/
#define NO_FILENAME (filestr[0] == '\0')
#endif


	basemag = g_dpi * 5;
    runmag = basemag;

    for (k = 0; k < 10; ++k)
	tex_counter[k] = (INT32)0L;

    fontptr = (struct font_entry *)NULL;
    hfontptr = (struct font_entry *)NULL;
    pfontptr = (struct font_entry *)NULL;
    fontfp = (FILE *)NULL;
    cache_size = 0;
    nopen = 0;


    /***********************************************************
    Set up file names and open dvi and plot files.  Simple
    filename parsing assumes forms like:

    Unix:	name
		name.ext
		/dir/subdir/.../subdir/name
		/dir/subdir/.../subdir/name.ext
    TOPS-20:	any Unix style (supported by PCC-20)
		<directory>name
		<directory>name.ext
		<directory>name.ext.gen
		device:<directory>name
		device:<directory>name.ext
		device:<directory>name.ext.gen
		[directory]name
		[directory]name.ext
		[directory]name.ext.gen
		device:[directory]name
		device:[directory]name.ext
		device:[directory]name.ext.gen
		logname:name
		logname:name.ext
		logname:name.ext.gen

    The Unix style should work  under IBM PC DOS (backslash  can
    usually be  replaced for  forward  slash), and  the  TOPS-20
    style contains VAX VMS as  a subset.  Fancier TOPS-20  names
    are possible (control-V quoting of arbitrary characters, and
    attributes), but they are rare enough that we do not support
    them.  For  TOPS-20  and  VAX VMS,  generation  numbers  are
    recognized as a digit following  the last dot, and they  are
    preserved for the DVI file only.  For the output files,  the
    highest generation  will always  be  assumed.  This  is  for
    convenience with the  TOPS-20 TeX  implementation, which  on
    completion types  in a  command "TeXspool:  foo.dvi.13"  and
    waits for the user to type a carriage return.

    We only  need  to  extract the  directory  path,  name,  and
    extension, so the parsing is simple-minded.

    ***********************************************************/

    tcp = strrchr(filestr,'/'); /* find end of Unix file path */

#if    (OS_ATARI | OS_PCDOS)
    if (tcp == (char*)NULL)	/* no Unix-style file path */
	tcp = strrchr(filestr, '\\');	/* try \dos\path\ */
#endif

#if OS_THINKC
	if (tcp == (char *)NULL)
		tcp = strrchr(filestr, ':');
#endif


    if (tcp == (char*)NULL)	/* no file path */
    {
		curpath[0] = '\0';	/* empty path */
		tcp = filestr;	/* point to start of name */
    }
    else			/* save path for later use */
    {
		k = (INT16)(tcp-filestr+1);
		(void)strncpy(curpath, filestr, (size_t)k);
		curpath[k] = '\0';
		tcp++;			/* point to start of name */
    }

    tcp1 = strrchr(tcp, '.');	/* find last dot in filename */


#if    (OS_TOPS20 | OS_VAXVMS)
    if ((tcp1 != (char*)NULL) && isdigit(*(tcp1+1)))
    {				/* then assume generation number */
	tcp2 = tcp1;		/* remember dot position */
	*tcp1 = '\0';		/* discard generation number */
	tcp1 = strrchr(tcp,'.');/* find last dot in filename */
	*tcp2 = '.';		/* restore dot */
    }
#endif

    if (tcp1 == (char*)NULL)
    {				/* no dot, so name has no extension */
		(void)strcpy(curname, tcp);	/* save name */
		tcp1 = strchr(tcp, '\0');	/* set empty extension */
    }
    else			/* save name part */
    {
		k = (INT16)(tcp1-tcp);
		(void)strncpy(curname, tcp, (size_t)k);
		curname[k] = '\0';
    }

    (void)strcpy(curext, tcp1);	/* save extension */


    /* DVI file must always have extension DVIEXT; supply one if
    necessary (e.g /u/jones/foo.dvi) */

    (void)strcpy(dviname, curpath);
    (void)strcat(dviname, curname);
    if (curext[0] == '\0')
	(void)strcat(dviname, DVIEXT);
    else
	(void)strcat(dviname, curext);


    /* Font substitution file is PATH NAME SUBEXT (e.g.
       /u/jones/foo.sub).  We do not tell user (via stderr)
       about it until it is needed. */

    if (subfile[0] == '\0')	/* no -fsubfile; make default */
    {
	(void)strcpy(subfile, curpath);
	(void)strcat(subfile, curname);
	(void)strcat(subfile, subext);
    }



    dvifp = FOPEN(dviname,RB_OPEN);
    DEBUG_OPEN(dvifp,dviname,RB_OPEN);

    if (dvifp == (FILE*)NULL)
    {
		(void)sprintf(message,"dviinit(): %s: can't open [%s]",
	    g_progname, dviname);
		Kill_dvi(message);
		return;
    }

    /* We try both
	PATH NAME . DVIPREFIX OUTFILE_EXT and
	     NAME . DVIPREFIX OUTFILE_EXT,
        in case the user does not have write access to the directory PATH */

    if (!quiet)
    {
		(void)printf("[Input from DVI file %s]",dviname);
		printf("\n");
    }

    lmargin = (COORDINATE)(leftmargin*((float)XDPI));
    tmargin = (COORDINATE)(topmargin*((float)YDPI));

#if JDEB
	printf("[dviinit calling nosignex(dvi,1)]");
#endif
    if ((BYTE)nosignex(dvifp,(BYTE)1) != PRE)
    {
    	Kill_dvi( "dviinit(): PRE doesn't occur first--are you sure"
    		" this is a DVI file?" );
    	return;
    }

#if JDEB
	printf("[dviinit calling signex(dvi,1)]");
#endif
    i = (int)signex(dvifp,(BYTE)1);
    if (i != DVIFORMAT)
    {
		(void)sprintf(message,
	    "dviinit(): DVI format = %d, can only process DVI format %d files.",
	    i, (int)DVIFORMAT);
		Kill_dvi(message);
    }
}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
