/*
*   compiles and links under MPW C 2.0.2
*   The following two lines are in my stdio.h:
*  #define malloc(A) NewPtr(A)
*  #define free(A) DisposPtr(A)
*/

#include <stdio.h>
#include <memory.h>
#include "vdevice.h"

/*
*  Window types are:
*    Notebook with a text representation of the user's notes and eqn's
*    Text window displaying the numbers as a spreadsheet-type array.
*    Simple image window with integral pixel expansion.
*    Bi-linear interpolation window - an image of the data.
*    Polar data generated into a Cartesian window.
*
*	fdatawin holds the binary dataset.
*/

struct fnotewin {
	TEHandle
		trec;				/* TextEdit record which holds notes */
	Rect
		vis;				/* Visible portion of the notebook */
};

struct ftextwin {
	int
		synchon,			/* synchronized window operation */
		offt,offl,			/* offset from 0,0 into dataset for scroll bars */
		forcedraw;			/* forces complete redraw of window */
	Point 
		selanchor,			/* selection anchor point in cell coords */
		ccount,				/* how many cells are there visible in each direction? */
		csize;				/* how big is each cell in window */
	Rect
		disp,				/* pixel rect which the array's text will go in */
		sel;				/* selection region, UL, LR in cell coords */
	ControlHandle	
		vbar,hbar;			/* scroll bars */
	char 
		*font;				/* what font to use */
	int fsize;				/* size of that font */
};

struct fimgwin {			/* Image window */
	Rect
		viewport,			/* visible port in imaging window */
		xr;					/* xor rectangle marks current selection */
	int
		xsize,ysize,		/* Size of the window as displayed */
		exx,exy;			/* expansion factors, when displayed as an image */
};

struct fbiwin {				/* Bilinear interp Image window */
	Rect
		viewport,			/* visible port in imaging window */
		xr;					/* xor rectangle marks current selection */
	int
		xsize,ysize,		/* Size of the window as displayed */
		exx,exy;			/* expansion factor, when displayed as an image */
};

struct fpolwin {			/* Polar data structure */
	Rect
		viewport,			/* visible port in imaging window */
		xr;					/* xor rectangle marks current selection */
	int
		angleshift,			/* angle shift for this image */
		xsize,ysize,		/* Size of the window as displayed */
		exx,exy;			/* expansion factor, when displayed as an image */
};

struct fdatawin {
	float 
		valmax,				/* maximum value in the set */
		valmin,				/* minimum value in the set */
		*xvals,				/* values of the x independent var along axis */
		*yvals,				/* increments in the independent vars */
		*vals;				/* floating point data set */
	int 
		top, bottom,		/* selection region in dataset */
		left, right,
		angleshift,			/* zero axis shift in pi/2 increments */
		xsize,ysize,		/* how big to make image (independent of integer expansion) */
		cmin,cmax,			/* color range to use when color scaling the data */
		exx,exy,			/* expansion factors for generating the image */
		refcount,			/* keep track of references for freeing later */
		contentlen,			/* length of content field */
		xdim,ydim;			/* x and y dimensions of data set */
	char 
		needsave,			/* have changes been made which require saving? */
		dvar[20],			/* dependent var */
		xvar[20],			/* x axis label */
		yvar[20],			/* y axis label */
		fname[100],			/* filename that data came from */
		labfmt[50],			/* row/col label format for use in sprintf */
		fmt[50];			/* printing format to use in sprintf */

	Rect
		viewport;			/* visible port in imaging window */
		
	CharsHandle
		content;			/* handle to contents of notebook */
		
	struct Mwin				/* which image windows were generated from this data? */
		*notes,				/* notebook window which goes with this data */
		*text,				/* text display of the floating point numbers */
		*image,				/* simple image scaling */
		*interp,			/* interpolated image, using bilinear interpolation */
		*polar;				/* polar transformation */
};
	
struct Mwin {
	WindowPtr win;			/* reference to window */
	int wintype;			/* window type for this record */
							/* the window pointers below could be a union, but why bother? */
	VDevice 
		vdev;				/* Virtual drawing device with its own colors */
	Rect
		pref;				/* preferred display rectangle, used for odd-width datasets */
		
	struct fnotewin *nw;	/* notebook window */
	struct ftextwin *cw;	/* text window */
	struct fimgwin *iw;		/* image window */
	struct fbiwin *bw;		/* bilinear interp window */
	struct fpolwin *pw;		/* polar data window */
	struct fdatawin *dat;	/* actual floating point data structure */
	struct Mwin *next;		/* linked list */
};

struct Mwin *findm();		/* finding an mwindow in the list */
struct fdatawin *newdatawin();

/******************************************************************************/
/*  The window type defines.
*/
#define FTEXT 1
#define FIMG 2
#define FBI 3
#define FPOL 4
#define FNOTES 5

