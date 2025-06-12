/* -*-C-*- dvifile.h */
/*-->dvifile*/
#define JDEB 0
#include "dvihead.h"
#include "commands.h"
#include "gendefs.h"
#include "gblprocs.h"
#include "egblvars.h"
#include "m72.h"
#include "mac-specific.h"
#include "sillymalloc.h"

void rollbeachball( void );
/**********************************************************************/
/****************************** dvifile.c *******************************/
/**********************************************************************/

#if OS_THINKC
void
dvifile( filestr )
char* filestr;			/* DVI filename to process */

#else /* not OS_THINKC */
void
dvifile(argc,argv,filestr)
int   argc;			/* argument count */
char* argv[];			/* argument strings */
char* filestr;			/* DVI filename to process */
#endif /* OS_THINKC */
{

#define PAGENUMBER(p) ((p) < 0 ? (page_count + 1 + (p)) : (p))

    register INT16 i,j,m;	/* loop indices */
    register INT16 swap;	/* swap storage */

    INT16 m_begin,m_end,m_step;	/* loop limits and increment */
    INT16 list[3];		/* index list for sort */
    char tempstr[MAXSTR];	/* temporary string storage */


    /*
    Establish the default  font file  search order.  This  is done  here
    instead of  in  initglob()  or  option(),  because  both  could  set
    fontlist[].

    By default, the search list contains  names for the PK, GF, and  PXL
    font  files;  that  order   corresponds  to  increasing  size   (and
    therefore, presumably, increasing  processing cost)  of font  raster
    information.  This search  order may  be overridden at  run time  by
    defining an alternate one in the environment variable FONTLIST;  the
    order of  the  strings  "PK",  "GF",  and  "PXL"  in  that  variable
    determines the search  order, and  letter case  is NOT  significant.
    The  substrings  may  be  separated   by  zero  or  more   arbitrary
    characters, so  the  values  "PK-GF-PXL", "PK  GF  PXL",  "PKGFPXL",
    "PK;GF;PXL", "PK/GF/PXL"  are all  acceptable,  and all  choose  the
    default search  order.   This  flexibility  in  separator  character
    choice is occasioned  by the  requirement on some  systems that  the
    environment variable have the syntax of a file name, or be a  single
    "word".  If  any  substring  is  omitted, then  that  font  will  be
    excluded from consideration.  Thus, "GF" permits the use only of  GF
    font files.

    The indexes gf_index, pk_index, and pxl_index are in -1 .. 2, and at
    least one must be non-negative.  A negative index excludes that font
    file type from the search.
    */


    /* Note that non-negative entries in list[] are UNIQUE. */
    list[0] = gf_index = (INT16)strid2(fontlist,"GF");
    list[1] = pk_index = (INT16)strid2(fontlist,"PK");
    list[2] = pxl_index = (INT16)strid2(fontlist,"PXL");

    for (i = 0; i < 3; ++i)	/* put list in non-decreasing order */
	for (j = i+1; j < 3; ++j)
	    if (list[i] > list[j])
	    {
		swap = list[i];
		list[i] = list[j];
		list[j] = swap;
	    }
    for (i = 0; i < 3; ++i)	/* assign indexes 0,1,2 */
	if (list[i] >= 0)
	{
            if (list[i] == gf_index)
		gf_index = i;
            else if (list[i] == pk_index)
		pk_index = i;
            else if (list[i] == pxl_index)
		pxl_index = i;
	}

    if ((gf_index < 0) && (pk_index < 0) && (pxl_index < 0))
	(void)fatal("dvifile():  FONTLIST does not define at least one of \
GF, PK, or PXL fonts");

    (void)dviinit(filestr);	/* initialize DVI file processing */
	if (g_abort_dvi)
		return;
#if OS_THINKC
#else
    (void)devinit(argc,argv);	/* initialize device output */
#endif /* OS_THINKC */

    (void)readpost();
    if (g_abort_dvi)
    {
	    (void)dviterm();		/* terminate DVI file processing */
	    (void)Close_printer();		/* terminate device output */
    	return;
    }
	rollbeachball(); /* OS_THINKC */
    (void)FSEEK(dvifp, 14L, 0); /* position past preamble */
    (void)getbytes(dvifp, tempstr,
	(BYTE)nosignex(dvifp,(BYTE)1)); /* flush DVI comment */

    cur_page_number = 0;


#if    (HPLASERJET|HPJETPLUS|GOLDENDAWNGL100|POSTSCRIPT|IMPRESS|CANON_A2)
    /* print pages in reverse order because of laser printer */
    /* page stacking */
    if (backwards)
    {
	m_begin = 1;
	m_end = page_count;
	m_step = 1;
    }
    else	/* normal device order */
    {
	m_begin = page_count;
	m_end = 1;
	m_step = -1;
    }

#else
  /* NOT (HPLASERJET|HPJETPLUS|GOLDENDAWNGL100|POSTSCRIPT|IMPRESS|CANON_A2) */
    /* print pages in forward order for most devices */
    if (backwards)
    {
	m_begin = page_count;
	m_end = 1;
	m_step = -1;
    }
    else
    {
	m_begin = 1;
	m_end = page_count;
	m_step = 1;
    }
#endif /* (HPLASERJET|HPJETPLUS|GOLDENDAWNGL100|POSTSCRIPT|IMPRESS|CANON_A2) */

    for (i = 0; i < npage; ++i)		/* make page numbers positive */
    {					/* and order pairs non-decreasing */
	page_begin[i] = PAGENUMBER(page_begin[i]);
	page_end[i] = PAGENUMBER(page_end[i]);
	if (page_begin[i] > page_end[i])
	{
	    swap = page_begin[i];
	    page_begin[i] = page_end[i];
	    page_end[i] = swap;
	}
    }
	rollbeachball(); /* OS_THINKC */

    for (m = m_begin; ; m += m_step)
    {
#if OS_THINKC
		if (g_abort_dvi)
			break;
#endif /* OS_THINKC */
		for (i = 0; i < npage; ++i)	/* search page list */
		    if ( IN(page_begin[i],m,page_end[i]) &&
		        (((m - page_begin[i]) % page_step[i]) == 0) )
		    {
				if (!quiet)		/* start progress report */
				{
			   		(void)printf("[%d", m);
			   		/* (void) fflush(stderr); */
			   	}
				cur_page_number++;/* sequential page number 1..N */
				cur_index = m-1;	/* remember index globally */
				prtpage(page_ptr[cur_index]);
				if (!quiet)		/* finish progress report */
			    	(void)printf("] "); /* (void)fflush(stderr); */
				break;
		    }
		if ( (m == m_end) || g_abort_dvi )	/* exit loop after last page */
		    break;
    }

	rollbeachball(); /* OS_THINKC */

    (void)dviterm();		/* terminate DVI file processing */
    (void)Close_printer();		/* terminate device output */

}

/* -*-C-*- readpost.h */
/*-->readpost*/
/**********************************************************************/
/****************************** readpost.h ******************************/
/**********************************************************************/

void
readpost()

/***********************************************************************
This routine is used  to read in the  postamble values.  It  initializes
the magnification and checks the stack height prior to starting printing
the document.
***********************************************************************/

{
    long lastpageptr;		/* byte pointer to last physical page */
    int the_page_count;		/* page count from DVI file */

    findpost();
    if (g_abort_dvi)
    	return;
    if ((BYTE)nosignex(dvifp,(BYTE)1) != POST)
	{
		(void)Kill_dvi("readpost():  POST missing at head of postamble");
		return;
	}
    lastpageptr = (long)nosignex(dvifp,(BYTE)4);
    num = (UNSIGN32)nosignex(dvifp,(BYTE)4);
    den = (UNSIGN32)nosignex(dvifp,(BYTE)4);
    mag = (UNSIGN32)nosignex(dvifp,(BYTE)4);
    conv = ((float)num/(float)den) *
	   ((float)runmag/(float)STDMAG) *

#if    USEGLOBALMAG
	    actfact(mag) *
#endif

	    ((float)RESOLUTION/254000.0);
    /* denominator/numerator here since will be dividing by the conversion
	   factor */
    (void) nosignex(dvifp,(BYTE)4);	/* height-plus-depth of tallest page */
    (void) nosignex(dvifp,(BYTE)4);	/* width of widest page */
    if ((int)nosignex(dvifp,(BYTE)2) >= STACKSIZE)
	{
		(void)Kill_dvi("readpost():  Stack size is too small");
		return;
	}

    the_page_count = (int)nosignex(dvifp,(BYTE)2);

    if (!quiet)
    {
		(void)printf("[%d pages]\n",the_page_count);
	
		(void)printf("[%d magnification, fonts %ddpi, printer %ddpi]\n",
			(int)runmag, g_dpi, g_printer_dpi);
    }

    if (preload)
        getfntdf();
    if (!g_abort_dvi)
	    getpgtab(lastpageptr);
}

/* -*-C-*- findpost.h */
/*-->findpost*/
/**********************************************************************/
/****************************** findpost.h ******************************/
/**********************************************************************/

void
findpost()

{
    register long       postambleptr;
    register BYTE       i;
    register UNSIGN16 the_char;         /* loop index */

    (void) FSEEK (dvifp, 0L, 2);        /* goto end of file */

    /* VAX VMS binary files are stored with NUL padding to the next
    512 byte multiple.  We therefore search backwards to the last
    non-NULL byte to find the real end-of-file, then move back from
    that.  Even if we are not on a VAX VMS system, the DVI file might
    have passed through one on its way to the current host, so we
    ignore trailing NULs on all machines. */

    while (FSEEK(dvifp,-1L,1) == 0)
    {
        the_char = (UNSIGN16)fgetc(dvifp);
        if (the_char)
          break;                /* exit leaving pointer PAST last non-NUL */
        UNGETC((char)the_char,dvifp);
    }

    postambleptr = FTELL(dvifp) - 4;

#if    OS_VAXVMS
    /* There is a problem with FSEEK() that I cannot fix just now.  A
    request to position to end-of-file cannot be made to work correctly,
    so FSEEK() positions in front of the last byte, instead of past it.
    We therefore modify postambleptr accordingly to account for this. */

    postambleptr++;
#endif

    (void) FSEEK (dvifp, postambleptr, 0);
    while (TRUE)
    {
        (void) FSEEK (dvifp, --(postambleptr), 0);
#if JDEB
	printf("[nosignex(dvi)");
#endif
		rollbeachball(); /* OS_THINKC */
        if (((i = (BYTE)nosignex(dvifp,(BYTE)1)) != 223) &&
            (i != DVIFORMAT))
         {
         	Kill_dvi( "findpost():  Bad end of DVI file" );
         	return;
         }
         if (i == DVIFORMAT)
            break;
    }
    (void) FSEEK (dvifp, postambleptr - 4, 0);
#if JDEB
	printf("[nosignex(dvi)");
#endif
    (void) FSEEK (dvifp, (long)nosignex(dvifp,(BYTE)4), 0);
}



/* -*-C-*- getfntdf.h */
/*-->getfntdf*/
/**********************************************************************/
/****************************** getfntdf.h ******************************/
/**********************************************************************/

void
getfntdf()

/***********************************************************************
Read the font definitions as they are in the postamble of the DVI  file.
Note that the font directory is not yet loaded.
***********************************************************************/

{
    register BYTE    byte;

#if JDEB
	printf("[nosignex(dvi) getfntdf");
#endif
    while (((byte = (BYTE)nosignex(dvifp,(BYTE)1)) >= FNT_DEF1)
		&& (byte <= FNT_DEF4))
	{
		if (User_wants_out())
			return;
		readfont ((INT32)nosignex(dvifp,(BYTE)(byte-FNT_DEF1+1)));
		if (g_abort_dvi)
			return;
	}
    if (byte != POST_POST)
		Kill_dvi ("getfntdf():  POST_POST missing after fontdefs");
}


/* -*-C-*- readfont.h */
/*-->readfont*/ /* called by getfntdef */
/**********************************************************************/
/****************************** readfont.h ******************************/
/**********************************************************************/

void
readfont(font_k)
INT32 font_k;
{
    BYTE a, l;
    UNSIGN32 c;				/* checksum */
    UNSIGN32 d;				/* design size */
    char n[MAXSTR];
    UNSIGN32 s;				/* scale factor */
    struct font_entry *tfontptr;	 /* temporary font_entry pointer */

#if JDEB
	printf("[nosignex(dvi) readfont");
#endif
    c = (UNSIGN32)nosignex(dvifp,(BYTE)4);
    s = (UNSIGN32)nosignex(dvifp,(BYTE)4);
    d = (UNSIGN32)nosignex(dvifp,(BYTE)4);
    a = (BYTE)nosignex(dvifp,(BYTE)1);
    l = (BYTE)nosignex(dvifp,(BYTE)1);
    (void)getbytes(dvifp, n, (BYTE)(a+l));
    n[a+l] = '\0';
    tfontptr = (struct font_entry*)MALLOC((unsigned)sizeof(struct font_entry));
    if (tfontptr == (struct font_entry *)NULL)
    {
    	Kill_dvi("readfont():  No allocable memory space left for font_entry");
    	return;
    }
    tfontptr->next = hfontptr;

    fontptr = hfontptr = tfontptr;
    fontptr->k = font_k;
    fontptr->c = c;
    fontptr->s = s;
    fontptr->d = d;
    fontptr->a = a;
    fontptr->l = l;
    fontptr->font_file_id = NULL; /* JWW fixed subtle bug */
    (void)strcpy(fontptr->n, n);
    fontptr->font_space = (INT32)(s/6);
    (void)reldfont(fontptr);
}

/* -*-C-*- reldfont.h */
/*-->reldfont*/
/**********************************************************************/
/****************************** reldfont.h ******************************/
/**********************************************************************/

void
reldfont(tfontptr)		/* load (or reload) font parameters */
struct font_entry *tfontptr;
{
    register UNSIGN16 the_char;	/* loop index */
    int err_code;
    register struct char_entry *tcharptr;/* temporary char_entry pointer */

    tfontptr->font_mag = (UNSIGN32)((actfact(
        MAGSIZE((float)tfontptr->s/(float)tfontptr->d)) *
	((float)runmag/(float)STDMAG) *

#if    USEGLOBALMAG
	actfact(mag) *
#endif

	(float)RESOLUTION * 5.0) + 0.5);

    tfontptr->designsize = (UNSIGN32)0L;
    tfontptr->hppp = (UNSIGN32)0L;
    tfontptr->vppp = (UNSIGN32)0L;
    tfontptr->min_m = (INT32)0L;
    tfontptr->max_m = (INT32)0L;
    tfontptr->min_n = (INT32)0L;
    tfontptr->max_n = (INT32)0L;

    for (the_char = FIRSTPXLCHAR; the_char <= LASTPXLCHAR; the_char++)
    {
	tcharptr = &(tfontptr->ch[the_char]);
	tcharptr->dx = (INT32)0L;
	tcharptr->dy = (INT32)0L;
	tcharptr->hp = (COORDINATE)0;
	tcharptr->fontrp = -1L;
	tcharptr->pxlw = (UNSIGN16)0;
	tcharptr->rasters = (UNSIGN32*)NULL;
	tcharptr->refcount = 0;
	tcharptr->tfmw = 0L;
	tcharptr->wp = (COORDINATE)0;
	tcharptr->xoffp = (COORDINATE)0;
	tcharptr->yoffp = (COORDINATE)0;
    }

    if (tfontptr != pfontptr)
	(void)openfont(tfontptr->n);

    if (fontfp == (FILE *)NULL)	/* have empty font with zero metrics */
	return;

    for (;;)		/* fake one-trip loop */
    {	/* test for font types PK, GF, and PXL in order of preference */
    	(void)REWIND(fontfp);	/* position to beginning-of-file */
#if JDEB
	printf("[nosignex(font) reldfont");
#endif
    	if ( ((BYTE)nosignex(fontfp,(BYTE)1) == (BYTE)PKPRE) &&
    	    ((BYTE)nosignex(fontfp,(BYTE)1) == (BYTE)PKID) )
    	{
    	    tfontptr->font_type = (BYTE)FT_PK;
	    	tfontptr->charxx = (void(*)())charpk;
    	    err_code = readpk();
			if (g_abort_dvi)
				return;
    	    break;
    	}

        (void)REWIND(fontfp);	/* position to beginning-of-file */
#if JDEB
	printf("[nosignex(font) reldfont");
#endif
#if !OS_THINKC /* GF fonts? We don't need no stinking GF fonts! */
        if ( ((BYTE)nosignex(fontfp,(BYTE)1) == (BYTE)GFPRE) &&
	    ((BYTE)nosignex(fontfp,(BYTE)1) == (BYTE)GFID) )
        {
	    tfontptr->font_type = (BYTE)FT_GF;
	    tfontptr->charxx = (void(*)())chargf;
	    err_code = readgf();
	    break;
        }

	(void)REWIND(fontfp);	/* position to beginning-of-file */
#if JDEB
	printf("[nosignex(font) reldfont");
#endif
	if ((UNSIGN32)nosignex(fontfp,(BYTE)4) == (UNSIGN32)PXLID)
	{
	    tfontptr->font_type = (BYTE)FT_PXL;
	    tfontptr->charxx = (void(*)())charpxl;
	    err_code = readpxl();
	    break;
	}
#endif /* not OS_THINKC */

	err_code = (int)EOF;
	break;
    } /* end one-trip loop */

    if (err_code)
    {
        (void)sprintf(message,
	    "reldfont():  Font file [%s] is not a valid GF, PK, or PXL file",
	    tfontptr->name);
    	Kill_dvi(message);
    }
}


/* -*-C-*- readpk.h */
/*-->readpk*/
/**********************************************************************/
/******************************* readpk *******************************/
/**********************************************************************/

int
readpk()	/* return 0 on success, EOF on failure */
{
    UNSIGN32 checksum;
    long end_of_packet;	/* pointer to byte following character packet */
    /* register */ BYTE flag_byte;
    UNSIGN32 packet_length;
    long start_of_packet;	/* pointer to start of character packet */
    register struct char_entry *tcharptr;/* temporary char_entry pointer */
    register UNSIGN32 the_char;

    /* Read a PK file, extracting character metrics and raster
       locations. */
    /******************************************************************/

    /* Process the preamble parameters */

    (void)REWIND(fontfp);	/* start at beginning of file */
    if ((BYTE)nosignex(fontfp,(BYTE)1) != PKPRE)
    {
	(void)warning("readpk():  PK font file does not start with PRE byte");
	return(EOF);
    }
    if ((BYTE)nosignex(fontfp,(BYTE)1) != PKID)
    {
	(void)warning(
	    "readpk():  PK font file PRE byte not followed by ID byte");
	return(EOF);
    }

#if JDEB
	printf("[nosignex(font) readpk");
#endif
    if (FSEEK(fontfp,(long)nosignex(fontfp,(BYTE)1),1))
    {
	(void)warning("readpk():  PK font file has garbled comment string");
	return(EOF);		/* skip comment string */
    }

#if JDEB
	printf(" (seek rel) [nosignex(font) readpk");
#endif
    fontptr->designsize = (UNSIGN32)nosignex(fontfp,(BYTE)4);

#if JDEB
	printf("[nosignex(font) readpk");
#endif
    checksum = (UNSIGN32)nosignex(fontfp,(BYTE)4);	/* checksum */
    if ((fontptr->c != 0L) && (checksum != 0L) && (fontptr->c != checksum))
    {
	(void)sprintf(message,
	"readpk():  font [%s] has checksum = 10#%010lu [16#%08lx] [8#%011lo] \
different from DVI checksum = 10#%010lu [16#%08lx] [8#%011lo].  \
TeX preloaded .fmt file is probably out-of-date with respect to new fonts.",
	    fontptr->name, fontptr->c, fontptr->c, fontptr->c,
	    checksum, checksum, checksum);
	(void)warning(message);
    }

#if JDEB
	printf("[nosignex(font) readpk");
#endif
    fontptr->hppp = (UNSIGN32)nosignex(fontfp,(BYTE)4);
#if JDEB
	printf("[nosignex(font) readpk");
#endif
    fontptr->vppp = (UNSIGN32)nosignex(fontfp,(BYTE)4);

    fontptr->min_m = 0L;	/* these are unused in PK format */
    fontptr->max_m = 0L;
    fontptr->min_n = 0L;
    fontptr->max_n = 0L;

    fontptr->magnification = (UNSIGN32)( 0.5 + 5.0 * 72.27 *
	(float)(fontptr->hppp) / 65536.0 );	/* from GFtoPXL Section 53 */

    /******************************************************************/
    /* Process the characters until the POST byte is reached */

    (void)skpkspec();	/* skip any PK specials */
	if (g_abort_dvi)
		return;
    start_of_packet = (long)FTELL(fontfp);
#if JDEB
	printf("[nosignex(font) readpk (start packet = %ld)", start_of_packet);
#endif
    while ((flag_byte = (BYTE)nosignex(fontfp,(BYTE)1)) != PKPOST)
    {
    	rollbeachball(); /* OS_THINKC */
	flag_byte &= 0x07;
	switch(flag_byte)	/* flag_byte is in 0..7 */
	{
	case 0:
	case 1:
	case 2:
	case 3:		/* short character preamble */
	    packet_length = (UNSIGN32)flag_byte;
	    packet_length <<= 8;
	    packet_length += (UNSIGN32)nosignex(fontfp,(BYTE)1);
#if JDEB
	printf("\n(case 3 packet_length = %ld)\n", packet_length);
#endif
	    the_char = (UNSIGN32)nosignex(fontfp,(BYTE)1);
	    if (the_char >= NPXLCHARS)
	    {
		(void)warning(
		"readpk():  PK font file character number is too big for me");
	        return(EOF);
	    }
	    tcharptr = &(fontptr->ch[the_char]);
	    end_of_packet = (long)FTELL(fontfp) + (long)packet_length;
#if JDEB
	printf("[nosignex(font) readpk");
#endif
	    tcharptr->tfmw = (UNSIGN32)(((float)nosignex(fontfp,(BYTE)3) *
		(float)fontptr->s) / (float)(1L << 20));
	    tcharptr->dx = (INT32)nosignex(fontfp,(BYTE)1) << 16;
	    tcharptr->dy = 0L;
	    tcharptr->pxlw = (UNSIGN16)PIXROUND(tcharptr->dx, 1.0/65536.0);
	    tcharptr->wp = (COORDINATE)nosignex(fontfp,(BYTE)1);
	    tcharptr->hp = (COORDINATE)nosignex(fontfp,(BYTE)1);
	    tcharptr->xoffp = (COORDINATE)signex(fontfp,(BYTE)1);
	    tcharptr->yoffp = (COORDINATE)signex(fontfp,(BYTE)1);
	    break;

	case 4:
	case 5:
	case 6:		/* extended short character preamble */
	    packet_length = (UNSIGN32)(flag_byte & 0x03);
	    packet_length <<= 16;
#if JDEB
		printf("(pre packet_length = %ld)\n", packet_length);
#endif
	    packet_length += (UNSIGN32)nosignex(fontfp,(BYTE)2);
#if JDEB
	printf("[nosignex(font) readpk (case 6 packet_length = %ld)\n",
		packet_length);
	printf("(flag_byte = %ld)\n", flag_byte);
#endif
	    the_char = (UNSIGN32)nosignex(fontfp,(BYTE)1);
	    if (the_char >= NPXLCHARS)
	    {
		(void)warning(
		"readpk():  PK font file character number is too big for me");
	        return(EOF);
	    }
	    tcharptr = &(fontptr->ch[the_char]);
	    end_of_packet = (long)FTELL(fontfp) + (long)packet_length;
#if JDEB
	printf("[nosignex(font) readpk");
#endif
	    tcharptr->tfmw = (UNSIGN32)(((float)nosignex(fontfp,(BYTE)3) *
		(float)fontptr->s) / (float)(1L << 20));
	    tcharptr->dx = (INT32)nosignex(fontfp,(BYTE)2) << 16;
	    tcharptr->dy = 0L;
	    tcharptr->pxlw = (UNSIGN16)PIXROUND(tcharptr->dx, 1.0/65536.0);
	    tcharptr->wp = (COORDINATE)nosignex(fontfp,(BYTE)2);
	    tcharptr->hp = (COORDINATE)nosignex(fontfp,(BYTE)2);
	    tcharptr->xoffp = (COORDINATE)signex(fontfp,(BYTE)2);
	    tcharptr->yoffp = (COORDINATE)signex(fontfp,(BYTE)2);
	    break;

	case 7:		/* long character preamble */
	    packet_length = (UNSIGN32)nosignex(fontfp,(BYTE)4);
#if JDEB
	printf("[nosignex(font) readpk (case 7 packet length= %ld)\n",
		packet_length);
#endif
#if JDEB
	printf("[nosignex(font) readpk (chars)");
#endif
	    the_char = (UNSIGN32)nosignex(fontfp,(BYTE)4);
	    if (the_char >= NPXLCHARS)
	    {
		(void)warning(
		"readpk():  PK font file character number is too big for me");
	        return(EOF);
	    }
	    tcharptr = &(fontptr->ch[the_char]);
	    end_of_packet = (long)FTELL(fontfp) + (long)packet_length;
	    tcharptr->tfmw = (UNSIGN32)(((float)nosignex(fontfp,(BYTE)4) *
		(float)fontptr->s) / (float)(1L << 20));
	    tcharptr->dx = (INT32)signex(fontfp,(BYTE)4);
	    tcharptr->dy = (INT32)signex(fontfp,(BYTE)4);
	    tcharptr->pxlw = (UNSIGN16)PIXROUND(tcharptr->dx, 1.0/65536.0);
	    tcharptr->wp = (COORDINATE)nosignex(fontfp,(BYTE)4);
	    tcharptr->hp = (COORDINATE)nosignex(fontfp,(BYTE)4);
	    tcharptr->xoffp = (COORDINATE)signex(fontfp,(BYTE)4);
	    tcharptr->yoffp = (COORDINATE)signex(fontfp,(BYTE)4);
	    break;
	} /* end switch */
	tcharptr->fontrp = start_of_packet;
#if JDEB
	printf("(seek abs %ld by %ld)\n", end_of_packet, packet_length);
#endif
	(void)FSEEK(fontfp,end_of_packet,0); /* position to end of packet */
	(void)skpkspec();	/* skip any PK specials */
	if (g_abort_dvi)
		return;
	start_of_packet = (long)FTELL(fontfp);
    } /* end while */

#if    (BBNBITGRAPH | HPJETPLUS | POSTSCRIPT | IMPRESS | CANON_A2)
    (void)newfont();
#endif

    return(0);
}

/* -*-C-*- skpkspec.h */
/*-->skpkspec*/
/**********************************************************************/
/****************************** skpkspec ******************************/
/**********************************************************************/

void
skpkspec()	/* Skip PK font file specials */
{
    BYTE the_byte;

#if JDEB
	printf("\nSkpkspec starting.\n");
#endif

#if JDEB
	printf("[nosignex(font) Skpkspec A");
#endif
    the_byte = (BYTE)nosignex(fontfp,(BYTE)1);
#if JDEB
	printf("(the_byte = %.2X)", the_byte);
#endif
    while ((the_byte >= (BYTE)PKXXX1) && (the_byte != PKPOST))
    {
	switch(the_byte)
	{
	case PKXXX1:
	    (void)FSEEK(fontfp,(long)nosignex(fontfp,(BYTE)1),1);
#if JDEB
		printf("PKXXX1\n");
#endif
	    break;

	case PKXXX2:
	    (void)FSEEK(fontfp,(long)nosignex(fontfp,(BYTE)2),1);
#if JDEB
		printf("PKXXX2\n");
#endif
	    break;

	case PKXXX3:
	    (void)FSEEK(fontfp,(long)nosignex(fontfp,(BYTE)3),1);
#if JDEB
		printf("PKXXX3\n");
#endif
	    break;

	case PKXXX4:
	    (void)FSEEK(fontfp,(long)nosignex(fontfp,(BYTE)4),1);
#if JDEB
		printf("PKXXX4\n");
#endif
	    break;

	case PKYYY:
	    (void)nosignex(fontfp,(BYTE)4);
#if JDEB
		printf("PKYYY\n");
#endif
	    break;

	case PKNOOP:
#if JDEB
		printf("PKNOOP\n");
#endif
	    break;

	default:
	    (void)sprintf(message,"skpkspec():  Bad PK font file [%s] [%hX]",
		fontptr->name, the_byte);
	    Kill_dvi(message);
	    return;
	}
	the_byte = (BYTE)nosignex(fontfp,(BYTE)1);
#if JDEB
	printf("(the_byte = %.2X)", the_byte);
#endif
    }
#if JDEB
	printf("(ungetc)", the_byte);
#endif
    (void)UNGETC((char)the_byte,fontfp);	/* put back lookahead byte */
#if JDEB
	printf("\nSkpkspec ending.\n");
#endif
}

/* -*-C-*- nosignex.h */
/*-->nosignex*/
/**********************************************************************/
/****************************** nosignex ******************************/
/**********************************************************************/

UNSIGN32
nosignex(fp, n)	/* return n byte quantity from file fd */
register FILE *fp;	/* file pointer    */
register BYTE n;	/* number of bytes (1..4) */

{
    register UNSIGN32 number;	/* number being constructed */

    number = 0;
    while (n--)
    {
		number <<= 8;
		number |= getc(fp);
    }
    return(number);
}

/* -*-C-*- signex.h */
/*-->signex*/
/**********************************************************************/
/******************************* signex *******************************/
/**********************************************************************/

INT32
signex(fp, n)     /* return n byte quantity from file fd */
register FILE *fp;    /* file pointer	 */
register BYTE n;      /* number of bytes */

{
    register BYTE n1; /* number of bytes	  */
    register INT32 number; /* number being constructed */

    number = (INT32)getc(fp);/* get first (high-order) byte */

#if    PCC_20
    if (number > 127) /* sign bit is set, pad top of word with sign bit */
	number |= 0xfffffff00;
		      /* this constant could be written for any */
		      /* 2's-complement machine as -1 & ~0xff, but */
		      /* PCC-20 does not collapse constant expressions */
		      /* at compile time, sigh... */
#endif /* PCC_20 */

#if    (OS_ATARI | OS_UNIX)	/* 4.1BSD C compiler uses logical shift too */
    if (number > 127) /* sign bit is set, pad top of word with sign bit */
	number |= 0xffffff00;
#endif /* OS_UNIX */

    n1 = n--;
    while (n--)
    {
	number <<= 8;
	number |= (INT32)getc(fp);
    }

#if    (OS_ATARI | PCC_20 | OS_UNIX)
#else /* NOT (OS_ATARI | PCC_20 | OS_UNIX) */

    /* NOTE: This code assumes that the right-shift is an arithmetic, rather
    than logical, shift which will propagate the sign bit right.   According
    to Kernighan and Ritchie, this is compiler dependent! */

    number<<=HOST_WORD_SIZE-8*n1;

#if    ARITHRSHIFT
    number>>=HOST_WORD_SIZE-8*n1;  /* sign extend */
#else /* NOT ARITHRSHIFT */
    (void)fatal("signex():  no code implemented for logical right shift");
#endif /* ARITHRSHIFT */

#endif /* (OS_ATARI | PCC_20 | OS_UNIX) */
#if JDEB
	printf("[signex(%d) <%ld>]", n, (INT32)number);
#endif
    return((INT32)number);
}


/**********************************************************************/
/****************************** dviterm *******************************/
/**********************************************************************/

void
dviterm()			/* terminate DVI file processing */
{
    register INT16 k;		/* loop index */


    /* Close all files and free dynamically-allocated font space.  Stdin
       is never closed, however, because it might be needed later for
       ioctl(). */

    if ((dvifp != (FILE*)NULL) && (dvifp != stdin))
        (void)fclose(dvifp);

    for (k = 1; k <= (INT16)nopen; ++k) /* k started at 1, said <= (JWW) */
    {
		if (font_files[k].font_id != (FILE *)NULL)
		{
		    (void)fclose(font_files[k].font_id);
		    font_files[k].font_id = (FILE *)NULL;
		}
    }

#if OS_THINKC
	sillyfree();
	g_page_window->grafProcs = NIL;
#else
    fontptr = hfontptr;
    while (fontptr != (struct font_entry *)NULL)
    {
		for (k = 0; k < NPXLCHARS; ++k)
		    if ((fontptr->ch[k].rasters) != (UNSIGN32 *)NULL)
			(void)free((char *)fontptr->ch[k].rasters);
		pfontptr = fontptr;
		fontptr = fontptr->next;
		(void)free((char *)pfontptr);
    }
#endif

    if (!quiet)
    {
    	/* fflush(stdout); */
		(void)printf(" [OK]");
		printf("\n");
    }

}

/**********************************************************************/
/****************************** fontfile ******************************/
/**********************************************************************/

void
fontfile(filelist,font_path,font_name,magnification)
char *filelist[MAXFORMATS];	/* output filename list */
char *font_path;		/* host font file pathname */
char *font_name;		/* TeX font_name */
int magnification;		/* magnification value */
{

    /*
    Given a TeX font_name and a magnification value, construct a list of
    system-dependent  host  filenames,  returning  them  in  the   first
    argument.  The files are not guaranteed to exist.  The font names by
    default contains names for the PK,  GF, and PXL font files, but  the
    selection, and search order, may be changed at run time by  defining
    a value for the environment  variable FONTLIST.  See initglob()  for
    details.
    */

    register int m;			/* index into filelist[] */
    register INT16 k;			/* loop index */
    float fltdpi;			/* dots/inch */
    int dpi;				/* dots/inch for filename */
    char *nameformat;		/* flags for possible font name formats */

    /*
    We need to convert an old-style ROUNDED integer magnification based
    on 1000 units = 200 dpi to an actual rounded dpi value.

    We have
	magnification = round(real_mag) = trunc(real_mag + 0.5)
    which implies
	float(magnification) - 0.5 <= real_mag < float(magnification) + 0.5

    We want
	dpi = round(real_mag/5.0)
    which implies
	float(dpi) - 0.5 <= real_mag/5.0 < float(dpi) + 0.5
    or
	5.0*float(dpi) - 2.5 <= real_mag < 5.0*float(dpi) + 2.5

    We can combine the two to conclude that
	5.0*float(dpi) - 2.5 < float(magnification) + 0.5
    or
	5.0*float(dpi) - 3 < float(magnification)
    which gives the STRICT inequality
	float(dpi) < (float(magnification) + 3.0)/5.0
    */

    fltdpi = (((float)magnification) + 3.0)/5.0;
    dpi = (int)fltdpi;
    if (fltdpi == (float)dpi)
	dpi--;				/* enforce inequality */

#define MAXFTYPES 1

    for (k = 0; k < MAXFORMATS; ++k) /* loop over possible file types */
      *filelist[k] = '\0';	/* Initially, all filenames are empty */
      
    m = 0;				/* index in filelist[] */
    for (k = 0; k < MAXFTYPES; ++k) /* loop over possible file types */
    {
    
	/* Typical results:
	    texfonts:72:cmr10.pk
	    texfonts:72:cmr10
	    texfonts:cmr10.72pk
	    texfonts:72:cmr10.72pk
	*/

	nameformat = *get_str_resource( "NAMEFORMAT" );
	/* if (k == pk_index) */
	if (nameformat[0] == '1')
		(void)sprintf(filelist[m++], "%s%s.%dpk",		/* cmr8.72pk */
			font_path, font_name, dpi );
	if (nameformat[2] == '1')
	    (void)sprintf(filelist[m++], "%s%d:%s",			/* 72:cmr8 */
			font_path, dpi, font_name);
	if (nameformat[3] == '1')
	    (void)sprintf(filelist[m++], "%s%d:%s.pk",		/* 72:cmr8.pk */
			font_path, dpi, font_name);
	if (nameformat[4] == '1')
		(void)sprintf( filelist[m++], "%s%d:%s.%dpk",	/* 72:cmr8.72pk */
			font_path, dpi, font_name, dpi );
	if (nameformat[6] == '1')
	    (void)sprintf(filelist[m++], "%s%d/%s",			/* 72/cmr8 */
			font_path, dpi, font_name);
	if (nameformat[7] == '1')
	    (void)sprintf(filelist[m++], "%s%d/%s.pk",		/* 72/cmr8.pk */
			font_path, dpi, font_name);
	if (nameformat[8] == '1')
		(void)sprintf( filelist[m++], "%s%d/%s.%dpk",	/* 72/cmr8.72pk */
			font_path, dpi, font_name, dpi );
	/* (void)printf("In fontfile: Path = %s\n",filelist[m - 1]); */

    }

}

