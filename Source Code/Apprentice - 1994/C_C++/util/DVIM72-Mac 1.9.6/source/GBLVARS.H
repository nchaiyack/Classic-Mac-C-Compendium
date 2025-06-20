/* -*-C-*- gblvars.h */
/*-->gblvars*/
/**********************************************************************/
/****************************** gblvars *******************************/
/**********************************************************************/

/**********************************************************************/
/*********************  General Global Variables  *********************/
/**********************************************************************/

char curpath[MAXFNAME];			/* current file area */
char curext[MAXFNAME];			/* current file extension */
char curname[MAXFNAME];			/* current file name */

UNSIGN16 debug_code;			/* 0 for no debug output */

char dviname[MAXFNAME];			/* DVI filespec */
char dvoname[MAXFNAME];			/* device output filespec */

char fontlist[MAXSTR];			/* FONTLIST environment string */
char fontpath[MAXFNAME];		/* font directory path */

char helpcmd[MAXSTR];			/* where to look for help */

char message[MAXMSG];			/* for formatting error messages */

int basemag;					/* takes the place of preprocessor STDMAG */

/***********************************************************************
Magnification table for 144dpi, 200dpi, and 300dpi devices, computed
to 20 figures and sorted by magnitude.

    Column 1	     Column 2	     Column 3
0.72*sqrt(1.2)**i  sqrt(1.2)**I  1.5*sqrt(1.2)**I	(I = -16,16)

***********************************************************************/
float mag_table[33];

INT16 mag_index;		/* set by actfact */

#define MAGTABSIZE (sizeof(mag_table) / sizeof(float))

BOOLEAN g_dolog = TRUE;		/* allow log file creation		  */
char g_progname[MAXSTR];	/* program name				  */

FILE *plotfp = (FILE*)NULL;	/* plot file pointer			  */

struct char_entry
{				/* character entry			  */
   COORDINATE wp, hp;		/* width and height in pixels		  */
   COORDINATE xoffp, yoffp;	/* x offset and y offset in pixels	  */
   long fontrp;			/* font file raster pointer		  */
   UNSIGN32 tfmw;		/* TFM width				  */
   INT32 dx, dy;		/* character escapements		  */
   UNSIGN16 pxlw;		/* pixel width == round(TFM width in	  */
				/* pixels for .PXL files, or		  */
				/* float(char_dx)/65536.0 for .GF and .PK */
				/* files)				  */
   INT16 refcount;		/* reference count for memory management  */
   UNSIGN32 *rasters;		/* raster description (dynamically loaded) */

#if    (HPJETPLUS | POSTSCRIPT | IMPRESS | CANON_A2)
   BOOLEAN isloaded;		/* is the character already downloaded?   */
#endif /* (HPJETPLUS | POSTSCRIPT | IMPRESS | CANON_A2) */

#if    HPJETPLUS
   BOOLEAN istoobig;		/* Too big (or too odd) to be loaded */
#endif

#if    CANON_A2
   BOOLEAN isknown;		/* Character is known */
   BOOLEAN istoobig;		/* Too big to be loaded */
#endif

#if    BBNBITGRAPH
   BOOLEAN istoobig;		/* is the character too big for BitGraph? */
   BOOLEAN isloaded;		/* is the character loaded in the BitGraph?*/
   INT16 bgfont, bgchar;	/* BitGraph font and character		  */
#endif /* BBNBITGRAPH */

};

struct font_entry
{
    struct font_entry *next;	/* pointer to next font entry		   */
    void (*charxx)();		/* pointer to chargf(), charpk(), charpxl()*/
    FILE *font_file_id;		/* file identifier (NULL if none)	   */
    INT32 k;			/* font number                             */
    UNSIGN32 c;			/* checksum                                */
    UNSIGN32 d;			/* design size                             */
    UNSIGN32 s;			/* scale factor                            */
    INT32 font_space;		/* computed from FNT_DEF s parameter	   */
    UNSIGN32 font_mag;		/* computed from FNT_DEF s and d parameters*/
    UNSIGN32 magnification;	/* magnification read from PXL file	   */
    UNSIGN32 designsize;	/* design size read from PXL file	   */
    UNSIGN32 hppp;		/* horizontal pixels/point * 2**16	   */
    UNSIGN32 vppp;		/* vertical pixels/point * 2**16	   */
    INT32 min_m;		/* GF bounding box values		   */
    INT32 max_m;
    INT32 min_n;
    INT32 max_n;

#if    (CANON_A2 | HPJETPLUS | IMPRESS | POSTSCRIPT)
    UNSIGN16 font_number;	/* font number (0..32767) */
#endif /* (CANON_A2 | HPJETPLUS | IMPRESS | POSTSCRIPT) */

#if    CANON_A2
    COORDINATE cell_w;
    COORDINATE cell_h;
    COORDINATE cell_d;
    UNSIGN16 nloaded;
    UNSIGN32 storage;
#endif /* CANON_A2 */

    BYTE font_type;		/* GF, PK, or PXL font file		   */
    BYTE a;			/* length of font area in n[]              */
    BYTE l;			/* length of font name in n[]              */
    char n[MAXSTR];		/* font area and name                      */
    char name[MAXSTR];		/* full name of PXL file		   */
    struct char_entry ch[NPXLCHARS];/* character information		   */
};

struct font_list
{
    FILE *font_id;		/* file identifier			   */
    INT16 use_count;		/* count of "opens"			   */
};

INT32 cache_size;		/* record of how much character raster	    */
				/* is actually used			    */
float conv;			/* converts DVI units to pixels		    */
UNSIGN16 copies;		/* number of copies to print of each page   */
INT16 cur_page_number;		/* sequential output page number in 1..N    */
INT16 cur_index;		/* current index in page_ptr[]              */

COORDINATE xcp,ycp;		/* current position			    */
UNSIGN32 den;			/* denominator specified in preamble	    */
FILE *dvifp = (FILE*)NULL;	/* DVI file pointer			    */
struct font_entry *fontptr;	/* font_entry pointer			    */
struct font_entry *hfontptr = (struct font_entry *)NULL;
				/* head font_entry pointer		    */

#if    (CANON_A2 | HPJETPLUS | IMPRESS | POSTSCRIPT)
UNSIGN16 font_count;		/* used to assign unique font numbers	    */
struct font_entry *font_table[MAXFONTS];
#endif /* (CANON_A2 | HPJETPLUS | IMPRESS | POSTSCRIPT) */

#if    (HPJETPLUS | POSTSCRIPT | IMPRESS | CANON_A2)
BOOLEAN font_switched;		/* current font has changed		    */
#endif /* (HPJETPLUS | POSTSCRIPT | IMPRESS | CANON_A2) */

INT16 gf_index, pk_index, pxl_index;
				/* indexes into filelist[] in fontfile();   */
				/* they define the search order, and are    */
				/* in initglob().			    */
UNSIGN32 gpower[33];		/* gpower[k] = 2**k-1 (k = 0..32)	    */
INT32 hxxxx;			/* current horizontal position		    */
COORDINATE hh;			/* current horizontal position in pixels    */

#if    HPLASERJET
INT16 hpres;			/* output resolution (75, 100, 150, 300 dpi) */
#endif /* HPLASERJET */

UNSIGN32 img_mask[32];		/* initialized at run-time so that bit k    */
				/* (counting from high end) is one	    */
UNSIGN32 img_row[(MAX_M - MIN_M + 1 + 31) >> 5];
				/* current character image row of bits	    */
INT16 max_m, min_m, max_n, min_n;
				/* current character matrix extents	    */
UNSIGN16 img_words;		/* number of words in use in img_row[]	    */
float leftmargin;		/* left margin in inches		    */
COORDINATE lmargin;		/* left margin offset in pixels		    */
INT16 nopen;			/* number of open PXL files		    */
INT16 page_count;		/* number of entries in page_ptr[]	    */

#if    HPJETPLUS
INT16 page_fonts;		/* count of fonts used on current page      */
#endif /* HPJETPLUS */

long page_ptr[MAXPAGE+1];	/* byte pointers to pages (reverse order)   */

#if    POSTSCRIPT
long page_loc[MAXPAGE+1];	/* byte pointers to output pages	    */
INT32 page_tex[MAXPAGE+1];	/* TeX's \count0 page numbers		    */
#endif /* POSTSCRIPT */

INT16 page_begin[MAXREQUEST+1],
    page_end[MAXREQUEST+1],
    page_step[MAXREQUEST+1];	/* explicit page range requests		    */
INT16 npage;			/* number of explicit page range requests   */
struct font_list font_files[MAXOPEN+1];
				/* list of open PXL file identifiers	    */

UNSIGN32 power[32];		/* power[k] = 1 << k			    */

#if    POSTSCRIPT
BOOLEAN ps_vmbug;		/* reload fonts on each page when TRUE	    */
#endif /* POSTSCRIPT */

UNSIGN32 rightones[HOST_WORD_SIZE];/* bit masks */

#if    (APPLEIMAGEWRITER | EPSON | DECLA75 | DECLN03PLUS)
BOOLEAN runlengthcode = FALSE;		/* this is runtime option '-r' */
#endif /* (APPLEIMAGEWRITER | EPSON | DECLA75 | DECLN03PLUS) */

#if    (GOLDENDAWNGL100 | TOSHIBAP1351)
BOOLEAN runlengthcode = FALSE;		/* this is runtime option '-r' */
#endif /* (GOLDENDAWNGL100 | TOSHIBAP1351) */

UNSIGN32 runmag;		/* runtime magnification		    */
UNSIGN32 mag;			/* magnification specified in preamble	    */
UNSIGN32 num;			/* numerator specified in preamble	    */
struct font_entry *pfontptr = (struct font_entry *)NULL;
				/* previous font_entry pointer		    */
BOOLEAN preload = TRUE;		/* preload the font descriptions?	    */
FILE *fontfp = (FILE*)NULL;	/* font file pointer			    */

BOOLEAN quiet = FALSE;		/* suppress status display when TRUE	    */

BOOLEAN backwards = FALSE;	/* print in backwards order		    */

#if    (CANON_A2 | HPJETPLUS | IMPRESS | POSTSCRIPT)
COORDINATE rule_height;		/* parameters of last rule set */
COORDINATE rule_width;
COORDINATE str_ycp;		/* last string ycp value */
UNSIGN16 size_limit;		/* character size limit in pixels -- larger */
				/* characters are downloaded each time they */
				/* are required to avoid PostScript ROM bugs */
#endif /* (CANON_A2 | HPJETPLUS | IMPRESS | POSTSCRIPT) */

#if    (BSD42 | OS_TOPS20)
BOOLEAN spool_output = FALSE;	/* offer to send output to spooler */
#endif /* (BSD42 | OS_TOPS20) */

char subpath[MAXFNAME];		/* font substitution file path		    */
char subname[MAXFNAME];		/* font substitution file name field	    */
char subext[MAXFNAME];		/* font substitution file extension field   */
char subfile[MAXFNAME];		/* font substitution filename		    */

INT32 tex_counter[10];		/* TeX c0..c9 counters on current page      */
float topmargin;		/* top margin in inches			    */
COORDINATE tmargin;		/* top margin offset in pixels		    */

INT32 vxxxx;			/* current vertical position		    */

#if    VIRTUAL_FONTS
BOOLEAN virt_font;		/* virtual font cache flag                  */
struct virt_data
    {
	int	cnt;
	char	*ptr;
	char	*base;
    };
struct virt_data virt_save[_NFILE];/* space for saving old FILE values      */
#endif /* VIRTUAL_FONTS */

COORDINATE vv;			/* current vertical position in pixels	    */

#if    BBNBITGRAPH
struct char_entry *bgcp[NBGFONTS+(NBGFONTS+2)/3][NPXLCHARS];
	/* Pointer to corresponding char_entry for this BitGraph font */
	/* and character.  These are used to set the char_entry's */
	/* status to "not loaded" when we have to reuse the BitGraph */
	/* character.  The array is cleared initially in devinit(). */

INT16 fullfont = 0;		/* full font to load in BitGraph	    */
BOOLEAN g_interactive=TRUE;	/* is the program running interactively   */
				/* (i.e., standard output not redirected)? */
INT16 partchar = FIRSTBGCHAR;	/* partial font character to load in BitGraph*/
INT16 partfont = NBGFONTS;	/* partial font to load in BitGraph	    */
INT16 pbghpos;			/* previous BitGraph horizontal position    */
INT16 pbgvpos;			/* previous BitGraph vertical position	    */
INT16 pbgf = -1;		/* previous BitGraph font		    */
COORDINATE xdiff;		/* x difference				    */
COORDINATE xscreen;		/* x screen adjustment			    */
COORDINATE ydiff;		/* y difference				    */
COORDINATE yscreen;		/* y screen adjustment			    */
long cpagep;			/* pointer to current page in DVI file	    */
long ppagep;			/* pointer to previous page in DVI file     */

#if    OS_TOPS20
#define jfn_plotfp (jfnof(fileno(plotfp)))

int bg_length,bg_width,bg_1ccoc,bg_2ccoc,bg_modeword,bg_sysmsg;
#endif /* OS_TOPS20 */

#endif /* BBNBITGRAPH */

#if OS_THINKC
TPPrPort	g_print_port_p;
#endif