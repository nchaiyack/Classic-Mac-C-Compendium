/**********************************************************************/
/************************  Device Definitions  ************************/
/**********************************************************************/

/* All output-device-specific definitions go here.  This section must
be changed when modifying a dvi driver for use on a new device */

#undef APPLEIMAGEWRITER
#define  APPLEIMAGEWRITER  1		/* conditional compilation flag */

#undef HIRES
#define  HIRES		  0		/* this is 72 dpi version */

#define VERSION_NO	"2.10"		/* DVI driver version number */

#define  DEVICE_ID	"Apple ImageWriter dot matrix printer"
					/* this string is printed at runtime */
#define  XDPI		g_dpi		/* horizontal dots/inch */

#define OUTFILE_EXT	"m72"

#define  BYTE_SIZE	  8		/* output file byte size */

#undef STDRES
#define STDRES  0		/* 0 for low-resolution devices */

#define  XWORDS		((XSIZE + HOST_WORD_SIZE - 1)/HOST_WORD_SIZE)
					/* number of words in rows  */
					/* of bitmap array */

#define  YDPI		XDPI		/* vertical dots/inch */

/* The printer bit map. */


#if    (IBM_PC_LATTICE | IBM_PC_MICROSOFT | IBM_PC_WIZARD)
#undef SEGMEM
#define SEGMEM 1 /* ( ((long)XBIT * (long)YBIT) > 65536L ) */
#endif

#undef STDMAG
#define STDMAG basemag
#undef RESOLUTION
#define RESOLUTION g_dpi

#define XSIZE g_paper_width
#define YSIZE g_paper_length