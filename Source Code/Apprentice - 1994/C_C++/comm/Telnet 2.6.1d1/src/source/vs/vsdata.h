/*
 *      Virtual Screen Kernel Data/Structure Definitions
 *                          (vsdata.h)
 *  
 *    National Center for Supercomputing Applications
 *      by Gaige B. Paulsen
 *
 *    This file contains the control and interface calls for the NCSA
 *  Virtual Screen Kernel.
 *
 *      Version Date    Notes
 *      ------- ------  ---------------------------------------------------
 *      0.01    861102  Initial coding -GBP
 *		2.1		871130	NCSA Telnet 2.1 -GBP
 *		2.2 	880715	NCSA Telnet 2.2 -GBP
 *
 */

#ifndef	__VSDATA__
#define __VSDATA__

#define MAXWID 132      /* The absolute maximum number of chars/line */

struct VSline
  {
	struct VSline
	  /* doubly-linked list of lines */
		*next,          /* Pointer to next line */
		*prev;          /* Pointer to previous line */
	char  		        /* Text for the line -- may be part of a block */
		*text;          /* of memory containing more than one line */
	short                /* flag for memory allocation coordination */
		mem;            /* nonzero for first line in memory block */
  };

typedef struct VSline
	VSline;
typedef VSline
	*VSlinePtr;
typedef VSlinePtr
	*VSlineArray;

#define maxparms 16

struct VSscrn {
	OSType	id;				// VSCR
    VSlinePtr
         scrntop,           /* topmost line of the current screen (= linest[0]) */
         buftop,            /* top (i e oldest line) of scrollback buffer 	*/
         vistop;            /* topmost line within visible region (may be in screen or scrollback area) */
	VSlineArray
        attrst,             /* pointer to array of screen attribute lines   */
        linest;             /* pointer to array of screen text lines        */
    short lines,				/* How many lines are in the screen arrays      */
		 maxlines,          /* maximum number of lines to save off top      */
         numlines,          /* number of lines currently saved off top      */
         allwidth,          /* allocated width of screen lines              */
         maxwidth,          /* current screen width setting (<= allwidth)   */
         savelines,         /* save lines off top? 0=no                     */
 		 forcesave,			/* NCSA 2.5: always force lines to be saved off top */
         ESscroll,			/* Scroll screen when ES received				*/
         attrib,            /* current character writing attributes         */
         x,y,               /* current cursor positon                       */
         Px,Py,Pattrib,     /* saved cursor position and writing attributes */
         VSIDC,             /* Insert/delete character mode 0=draw line     */
         DECAWM,            /* Auto Wrap Mode 0=off 						*/
         DECCKM,            /* Cursor Key Mode      						*/
         DECPAM,            /* keyPad Application Mode						*/
		 DECORG,			/* origin mode                                  */
		 G0,G1,				/* Character set identifiers 					*/
		 charset,			/* Character set mode 							*/
         IRM,               /* Insert/Replace Mode  						*/
         escflg,            /* Current Escape level							*/
         top, bottom,       /* Vertical bounds of scrolling region 			*/
         Rtop,Rbottom,		/* Vertical bounds of visible region 			*/
         Rleft,Rright,		/* Horizontal bounds of visible region 			*/
         parmptr,           /* LU - index of current parm 					*/
		 prredirect;		/* LU - printer redirection or not				*/
	long prbuf;				/* LU - last four chars							*/
   short refNum;			/* LU - temporary file for printer redirection	*/
	char fname[40];			/* LU - file name for temporary file			*/
    short parms[maxparms];   /* Ansi Parameters 							*/
    char *tabs;             /* pointer to array for tab settings 			*/
                            /* contains 'x' at each tab position, blanks elsewhere */
    };

typedef struct VSscrn VSscrn;

struct VSscrndata {
    VSscrn *loc;            /* Location of the Screen record for this scrn */
    short
    	captureRN,	/* capture file's RefNum                       */	/* BYU 2.4.18 */
    	stat;		/* status of this screen (0=Uninitialized,     */	/* BYU 2.4.18 */
					/*                        1=In Use             */	/* BYU 2.4.18 */
					/*                        2=Inited, but not IU */	/* BYU 2.4.18 */
    };

typedef struct VSscrndata VSscrndata;

#ifdef VSMASTER
VSscrn  *VSIw;
short      VSIwn;
#else
extern VSscrn   *VSIw;
extern short       VSIwn;
#endif

#define	VSPBOTTOM	(VSIw->lines)

#endif	// __VSDATA__