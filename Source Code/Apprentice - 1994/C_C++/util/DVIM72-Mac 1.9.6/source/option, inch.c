/**********************************************************************/
/******************************* option *******************************/
/**********************************************************************/
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include "dvihead.h"
#include "gendefs.h"
#include "gblprocs.h"
#include "m72.h"
#include "egblvars.h"
#include "mac-specific.h"

#if OS_THINKC
#define FATAL Abort_dvi
void Abort_dvi( char *msg );
#else
#define FATAL fatal
#endif

void
option(optstr)			/* process command-line option */
char *optstr;			/* option string (e.g. "-m1500") */
{
    register UNSIGN32 k;	/* magnification */
    register int value;		/* converted digit string value */
    register int m;		/* loop index */
    char *p;			/* pointer into optstr */
    int p1,p2,p3;		/* temp values for sscanf() */
    double fltmag;		/* floating-point mag value */

    typedef struct
    {
	char* envname;
	char* envvar;
    } envstruct;

    static envstruct envlist[] =
    {	/* names and addresses of environment vars (alphabetical order) */
	"DVIHELP",	helpcmd,
	"FONTLIST",	fontlist,
	"TEXFONTS",	fontpath,
	"TEXINPUTS",	subpath,
    };

    if (*optstr != '-')
	return;			/* return if this is not an option */

    switch (*(optstr+1))
    {

    case 'a':	/* A selects virtual font caching */
    case 'A':

#if    VIRTUAL_FONTS
	virt_font = TRUE;
#endif
	break;

    case 'b':	/* b selects backwards printing order */
    case 'B':
	backwards = TRUE;
	break;


    case 'c':	/* c selects number of copies */
    case 'C':
	copies = (UNSIGN16) atoi(optstr+2);
	copies = MAX(1,MIN(copies,256)); /* enforce reasonable limits */
	break;


    case 'd':	/* d selects debug output */
    case 'D':
	debug_code |= (BYTE)atoi(optstr+2);
	break;


    case 'e':	/* e specifies ``environment'' variable definition */
    case 'E':	/* We ignore letter case since these come from */
		/* the command line */
	if (!(p = strrchr(optstr,'=')))
	{
	    /* (void)usage(stderr); */
	    (void)sprintf(message,
	        "option():  Bad switch [%s]--expected -eENVNAME=value",
		optstr);
	    (void)FATAL(message);
	}
	*p = '\0';		/* clobber "=" by string terminator */
	for (m = 0; m < sizeof(envlist)/sizeof(envlist[0]); ++m)
	{
	    if (strcm2(optstr+2,envlist[m].envname) == 0)
	    {
		(void)strcpy(envlist[m].envvar,p+1);
		return;
	    }
	}
	/* (void)usage(stderr); */

	(void)sprintf(message,"option():  Bad switch [%s]--expected one of:",
	    optstr);
	for (m = 0; m < sizeof(envlist)/sizeof(envlist[0]); ++m)
	{
	    (void)strcat(message," ");
	    (void)strcat(message,envlist[m].envname);
	}
	(void)FATAL(message);
	break;


    case 'f':	/* f specifies font substitution file */
    case 'F':
	(void)strcpy(subfile,optstr+2);
	break;


#if    CANON_A2

#ifdef CANON_TEST
    case 'g':
    case 'G':
		stor_fonts = (UNSIGN32)atoi(optstr + 2 );
		break;
#endif /* CANON_TEST */

#endif

    case 'l':	/* l prohibits logging of errors */
    case 'L':
	g_dolog = FALSE;
	break;


    case 'm':	/* m selects alternate magnification */
    case 'M':
	/* Collect 2*value initially.  Then if value is small, assume
	user specified magstep value; magstep k means 1.2**k, where k
	is integral, or half-integral.  Otherwise, assume user has
	specified pxl file magnification (1000 == 200dpi, 1500 ==
	300dpi, etc.).  */


	if (strchr(optstr+2,'.') != (char *)NULL)
	    fltmag = (double)(2.0 * atof(optstr+2));
	else
	    fltmag = (double)(2 * atoi(optstr+2));
	if ((0.0 != fltmag) && 
	    (-30.0 <= fltmag) && (fltmag <= 30.0)) /* magstep 15 is limit */
	{
	    if (fltmag < 0.0)
	        runmag = (UNSIGN32)(0.5 + 
		    (1.0/pow((double)sqrt(1.2),-fltmag))*(double)STDMAG);
	    else
	        runmag = (UNSIGN32)(0.5 + pow((double)sqrt(1.2),fltmag)*
	            (double)STDMAG);
	}
	else
	    runmag = (UNSIGN32)(0.5 + fltmag/2.0);

	k = MAGSIZE(actfact_res(runmag));  /* rounded magnification */
	if (k != runmag)
	{
	    (void)sprintf(message,
		"option():  Requested magnification %d not available.",
		(int)runmag);
	    (void)warning(message);
	    runmag = k;
	    k = (UNSIGN32) MAX(1,MIN(mag_index,(MAGTABSIZE-1)));
	    (void)sprintf(message,
		"  Magnification reset to nearest legal value %d in \
family ..%d..%d..%d..",
		(int)runmag,(int)MAGSIZE(mag_table[k-1]),
		(int)MAGSIZE(mag_table[k]),
		(int)MAGSIZE(mag_table[k+1]));
	    (void)warning(message);
	}
	break;


    case 'o':
    case 'O':	/* o selects output page range (-o# or -o#:# or -o#:#:#) */
        p1 = p2 = p3 = 0;
	value = (int)sscanf(optstr+2,"%d:%d:%d",&p1,&p2,&p3);
	page_begin[npage] = p1;
	page_end[npage] = p2;
	page_step[npage] = p3;
	switch (value)
	{
	case 1:
	    optstr++;			/* point past "-" */

	    do				/* skip over digit string */
	    {
		optstr++;
	    }
	    while (isdigit(*optstr) || (*optstr == '-') || (*optstr == '+'));

	    if (*optstr)		/* trash follows number */
	    {
	        /* (void)usage(stderr); */
		(void)sprintf(message,
		    "option():  %s is not a legal page number switch",optstr);
		(void)FATAL(message);
	    }
	    else			/* had expected end-of-string */
	        page_end[npage] = (INT16)page_begin[npage];
	    page_step[npage] = 1;
	    break;

	case 2:		/* supply default step */
	    page_step[npage] = 1;
	    /* FALL THROUGH to case 3 for order check */

	case 3:		/* check for positive step */
	    page_step[npage] = ABS(page_step[npage]);
	    if (page_step[npage] == 0)
		page_step[npage] = 1;
	    break;

	default:	/* illegal value */
	    /* (void)usage(stderr); */
	    (void)sprintf(message,
		"option():  %s is not a legal page number switch",optstr);
	    (void)FATAL(message);
	}
	npage++;
	break;


    case 'p':	/* p prohibits pre-font loading */
    case 'P':
#if (OS_THINKC|HPLASERJET|HPJETPLUS|GOLDENDAWNGL100|POSTSCRIPT|IMPRESS|CANON_A2)
	preload = TRUE;	/* preloading required for Canon engines */
			/* because of reverse printing order */
#else
	preload = FALSE;
#endif /*NOT(HPLASERJET|HPJETPLUS|GOLDENDAWNGL100|POSTSCRIPT|IMPRESS|CANON_A2)*/
	break;


    case 'q':	/* q inhibits status display */
    case 'Q':
	quiet = TRUE;
	break;


#if    HPLASERJET
    case 'r':	/* r selects output resolution */
    case 'R':
	hpres = (INT16)atoi(optstr+2);
	if ((hpres != 75) && (hpres != 100) && (hpres != 150) &&
	    (hpres != 300))
	    {
	    (void)sprintf(message,
		"option():  Output resolution not in [75,100,150,300] dpi; \
reset to %d",
		DEFAULT_RESOLUTION);
	    (void)warning(message);
	    hpres = DEFAULT_RESOLUTION;
	    }
	break;
#endif /* HPLASERJET */


#if    (APPLEIMAGEWRITER | EPSON | GOLDENDAWNGL100 | TOSHIBAP1351)
    case 'r':	/* r selects run-length coding of output */
    case 'R':
	runlengthcode = TRUE;
	break;
#endif /* (APPLEIMAGEWRITER | EPSON | GOLDENDAWNGL100 | TOSHIBAP1351) */


#if    CANON_A2

#ifdef CANON_TEST
    case 's':	/* pixel size limit to force character reloading */
    case 'S':
	size_limit = (UNSIGN16)atoi(optstr+2);
	break;
#endif /* CANON_TEST */

#endif /* CANON_A2 */

#if    POSTSCRIPT
    case 's':	/* pixel size limit to force character reloading */
    case 'S':
	size_limit = (UNSIGN16)atoi(optstr+2);
	break;
#endif

#if    EPSON
    case 't':
    case 'T':

#if    HIRES
	twice_a_line = TRUE;
#endif /* HIRES */

	break;
#endif /* EPSON */


#if    CANON_A2

#ifdef CANON_TEST
    case 'u':
    case 'U':
	merit_fact = (UNSIGN16)atoi(optstr+2);
	break;

    case 'v':
    case 'V':
	stor_limit = (UNSIGN32)atoi(optstr+2);
	break;
#endif /* CANON_TEST */

#endif /* CANON_A2 */

#if    POSTSCRIPT
    case 'v':	/* vmbug -- reload fonts on each page */
    case 'V':
	ps_vmbug = TRUE;
	break;
#endif /* POSTSCRIPT */


    case 'x':
    case 'X':
	leftmargin = inch(optstr+2);
	break;


    case 'y':
    case 'Y':
	topmargin = inch(optstr+2);
	break;

#if    (OS_TOPS20 | BSD42)
    case 'z':
    case 'Z':
	spool_output = TRUE;	/* offer to send output to spooler */
	break;
#endif /* (OS_TOPS20 | BSD42) */

    default:
	/* (void)usage(stderr); */
	(void)sprintf(message,"option():  %s is not a legal switch", optstr);
	(void)FATAL(message);
    }
}

/**********************************************************************/
/******************************** inch ********************************/
/**********************************************************************/
float
inch(s)
char *s;

/***********************************************************************
Convert a value field in s[] of the form

	------		------------------------------
	value			units implied
	------		------------------------------
	#.##bp		big point (1in = 72bp)
	#.##cc		cicero (1cc = 12dd)
	#.##cm		centimeter
	#.##dd		didot point (1157dd = 1238pt)
	#.##in		inch
	#.##mm		millimeter (10mm = 1cm)
	#.##pc		pica (1pc = 12pt)
	#.##pt		point (72.27pt = 1in)
	#.##sp		scaled point (65536sp = 1pt)
	------		------------------------------

to inches, returning it as the function value.  A leading plus or  minus
sign is optional.  The letter case of the dimension name is ignored.  No
space is permitted between the number and the dimension.
***********************************************************************/

{
    BOOLEAN neg;
    register char *tc;
    register double flnum, pten;

    flnum = 0.0;
    tc = s;
    neg = FALSE;

    if (*tc == '-')
    {
	++tc;
	neg = TRUE;
    }
    else if (*tc == '+')
    {
	++tc;
	neg = FALSE;
    }

    /* we do not use sprintf() here because we want to be able to easily
    find the dimension string */

    for (; isdigit(*tc); ++tc)	/* collect integer part */
	flnum = flnum*10.0 + (double)((*tc) - '0');

    if (*tc == '.')			/* collect fractional part */
	for ((pten = 10.0, ++tc); isdigit(*tc); (pten *= 10.0,++tc))
	    flnum += ((double)((*tc) - '0'))/pten;

    if (strcm2(tc,"in") == 0)		/* inches */
	;
    else if (strcm2(tc,"cm") == 0)	/* centimeters */
	flnum /= 2.54;
    else if (strcm2(tc,"pt") == 0)	/* points */
	flnum /= 72.27;
    else if (strcm2(tc,"bp") == 0)	/* big points */
	flnum /= 72.0;
    else if (strcm2(tc,"cc") == 0)	/* cicero */
	flnum *= 12.0 * (1238.0 / 1157.0) / 72.27;
    else if (strcm2(tc,"dd") == 0)	/* didot points */
	flnum *= (1238.0 / 1157.0) / 72.27;
    else if (strcm2(tc,"mm") == 0)	/* millimeters */
	flnum /= 25.4;
    else if (strcm2(tc,"pc") == 0)	/* pica */
	flnum *= 12.0 / 72.27;
    else if (strcm2(tc,"sp") == 0)	/* scaled points */
	flnum /= (65536.0 * 72.27);
    else				/* unknown units -- error */

#if    PXLID                            /* must be in DVI driver */
    {
	(void)sprintf(message,
	    "inch():  Unrecognized dimension string [%s];\nexpected one of \
bp, cc, cm, dd, in, mm, pc, pt or sp\n",s);
        (void)FATAL(message);
    }
#else                                   /* some other program then */
    {
        (void)printf("inch():  Unrecognized dimension string [%s];",s);
	printf("\n");
	(void)printf(
	    "expected one of bp, cc, cm, dd, in, mm, pc, pt or sp");
	printf("\n");
        EXIT(1);
    }
#endif

    if (neg)
	flnum = -flnum;
    return((float)flnum);
}

/* ---------------------------- Abort_dvi ---------------------------- */
void
Abort_dvi( char *msg )
{
	g_abort_dvi = TRUE;
	Show_error( msg );
}
