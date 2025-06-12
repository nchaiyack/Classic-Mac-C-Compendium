/* -*-C-*- gblvars.h */
/*-->gblvars*/
#if OS_THINKC
#include "PrintTraps.h"
#endif
/**********************************************************************/
/****************************** egblvars.h *******************************/
/**********************************************************************/

/**********************************************************************/
/*********************  General Global Variables  *********************/
/**********************************************************************/

extern INT32 hxxxx;			/* current horizontal position		    */
extern char curpath[MAXFNAME];			/* current file area */
extern char curext[MAXFNAME];			/* current file extension */
extern char curname[MAXFNAME];			/* current file name */

extern UNSIGN16 debug_code;			/* 0 for no debug output */

extern char dviname[MAXFNAME];			/* DVI filespec */
extern char dvoname[MAXFNAME];			/* device output filespec */

extern char fontlist[MAXSTR];			/* FONTLIST environment string */
extern char fontpath[MAXFNAME];		/* font directory path */

extern char helpcmd[MAXSTR];			/* where to look for help */

extern char message[MAXMSG];			/* for formatting error messages */

extern int basemag;					/* takes the place of preprocessor STDMAG */
/***********************************************************************
Magnification table for 144dpi, 200dpi, and 300dpi devices, computed
to 20 figures and sorted by magnitude.

    Column 1	     Column 2	     Column 3
0.72*sqrt(1.2)**i  sqrt(1.2)**I  1.5*sqrt(1.2)**I	(I = -16,16)

***********************************************************************/
extern float mag_table[33];

extern INT16 mag_index;		/* set by actfact */

#define MAGTABSIZE 33 /* (sizeof(mag_table) / sizeof(float)) */

extern char g_logname[MAXSTR];		/* name of log file, if created		  */
extern BOOLEAN g_dolog;		/* allow log file creation		  */
extern FILE *g_logfp;	/* log file pointer (for errors)	  */
extern char g_progname[MAXSTR];	/* program name				  */

extern FILE *plotfp;	/* plot file pointer			  */

extern struct char_entry
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


};

extern struct font_entry
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


    BYTE font_type;		/* GF, PK, or PXL font file		   */
    BYTE a;			/* length of font area in n[]              */
    BYTE l;			/* length of font name in n[]              */
    char n[MAXSTR];		/* font area and name                      */
    char name[MAXSTR];		/* full name of PXL file		   */
    struct char_entry ch[NPXLCHARS];/* character information		   */
};

extern struct font_list
{
    FILE *font_id;		/* file identifier			   */
    INT16 use_count;		/* count of "opens"			   */
};

extern INT32 cache_size;		/* record of how much character raster	    */
				/* is actually used			    */
extern float conv;			/* converts DVI units to pixels		    */
extern UNSIGN16 copies;		/* number of copies to print of each page   */
extern INT16 cur_page_number;		/* sequential output page number in 1..N    */
extern INT16 cur_index;		/* current index in page_ptr[]              */

extern COORDINATE xcp,ycp;		/* current position			    */
extern UNSIGN32 den;			/* denominator specified in preamble	    */
extern FILE *dvifp;	/* DVI file pointer			    */
extern struct font_entry *fontptr;	/* font_entry pointer			    */
extern struct font_entry *hfontptr;
				/* head font_entry pointer		    */


extern INT16 gf_index, pk_index, pxl_index;
				/* indexes into filelist[] in fontfile();   */
				/* they define the search order, and are    */
				/* in initglob().			    */
extern UNSIGN32 gpower[33];		/* gpower[k] = 2**k-1 (k = 0..32)	    */
extern COORDINATE hh;			/* current horizontal position in pixels    */

extern UNSIGN32 img_mask[32];		/* initialized at run-time so that bit k    */
				/* (counting from high end) is one	    */
extern UNSIGN32 img_row[(MAX_M - MIN_M + 1 + 31) >> 5];
				/* current character image row of bits	    */
extern INT16 max_m, min_m, max_n, min_n;
				/* current character matrix extents	    */
extern UNSIGN16 img_words;		/* number of words in use in img_row[]	    */
extern float leftmargin;		/* left margin in inches		    */
extern COORDINATE lmargin;		/* left margin offset in pixels		    */
extern INT16 nopen;			/* number of open PXL files		    */
extern INT16 page_count;		/* number of entries in page_ptr[]	    */


extern long page_ptr[MAXPAGE+1];	/* byte pointers to pages (reverse order)   */


extern INT16 page_begin[MAXREQUEST+1],
    page_end[MAXREQUEST+1],
    page_step[MAXREQUEST+1];	/* explicit page range requests		    */
extern INT16 npage;			/* number of explicit page range requests   */
extern struct font_list font_files[MAXOPEN+1];
				/* list of open PXL file identifiers	    */

extern UNSIGN32 power[32];		/* power[k] = 1 << k			    */

#if    POSTSCRIPT
BOOLEAN ps_vmbug;		/* reload fonts on each page when TRUE	    */
#endif /* POSTSCRIPT */

extern UNSIGN32 rightones[HOST_WORD_SIZE];/* bit masks */

#if    (APPLEIMAGEWRITER | EPSON | DECLA75 | DECLN03PLUS)
extern BOOLEAN runlengthcode;		/* this is runtime option '-r' */
#endif /* (APPLEIMAGEWRITER | EPSON | DECLA75 | DECLN03PLUS) */

#if    (GOLDENDAWNGL100 | TOSHIBAP1351)
BOOLEAN runlengthcode = FALSE;		/* this is runtime option '-r' */
#endif /* (GOLDENDAWNGL100 | TOSHIBAP1351) */

extern UNSIGN32 runmag;		/* runtime magnification		    */
extern UNSIGN32 mag;			/* magnification specified in preamble	    */
extern UNSIGN32 num;			/* numerator specified in preamble	    */
extern struct font_entry *pfontptr;
				/* previous font_entry pointer		    */
extern BOOLEAN preload;		/* preload the font descriptions?	    */
extern FILE *fontfp;	/* font file pointer			    */

extern BOOLEAN quiet;		/* suppress status display when TRUE	    */

extern BOOLEAN backwards;	/* print in backwards order		    */


extern char subpath[MAXFNAME];		/* font substitution file path		    */
extern char subname[MAXFNAME];		/* font substitution file name field	    */
extern char subext[MAXFNAME];		/* font substitution file extension field   */
extern char subfile[MAXFNAME];		/* font substitution filename		    */

extern INT32 tex_counter[10];		/* TeX c0..c9 counters on current page      */
extern float topmargin;		/* top margin in inches			    */
extern COORDINATE tmargin;		/* top margin offset in pixels		    */

extern INT32 vxxxx;			/* current vertical position		    */

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

extern COORDINATE vv;			/* current vertical position in pixels	    */


#if OS_THINKC
extern TPPrPort	g_print_port_p;
#endif
