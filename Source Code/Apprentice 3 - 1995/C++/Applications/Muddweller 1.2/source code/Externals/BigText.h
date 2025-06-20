/* BigText - Supports large line-wrapped text buffers                         */

#ifndef __BIGTEXT__
#define __BIGTEXT__

		// � MacApp
#ifndef __UMacApp__
#include "UMacApp.h"
#endif


const int kTBSize = 1000;	/* Size of buffer, should be << 32K */
const short kLSChunk = kTBSize / 20;
const short kMaxTBSize = 32766;
const short kMaxChunk = 320; /* kMaxChunk * max char width must be < 32K! */
const int kClikDelay = 3;	/* Delay in ticks for autoscrolling */
const int kBTRsrv = 10000;	/* Size of memory reserve */

typedef short LineStarts [32767];
typedef LineStarts *LSPtr, **LSHandle;

typedef struct TextBuffer *TBPtr, **TBHandle;

struct TextBuffer {
	TBHandle bNext;			/* Handle to next buffer */
	short bLength;			/* Number of chars in this buffer */
	short bLines;			/* Number of lines in this buffer */
	long bVersion;			/* Format version of this buffer */
	LSHandle bStarts;		/* Line start array */
	unsigned char bBuf [kTBSize]; /* Text stored in this buffer */
};


class TBigText: public TObject {
public:
	TBHandle fText;			/* The first text buffer */
	VRect fBounds;			/* Keeps track of text width and offsets */
	Rect fDisplay;			/* The visible part of the text */
	long fCurVers;			/* Current format version */
	TextStyle fStyle;		/* Our text style */
	long fLineHeight;		/* Height of a line in pixels */
	long fTabWidth;			/* Width of a tab in pixels */
	long fSelStart;			/* Start of the selection */
	long fSelEnd;			/* End of the selection */
	long fLength;			/* Length of the text */
	long fMaxLength;		/* Maximum allowed length of the text */
	long fMaxLowSize;		/* Maximum size during low memory situation */
	Handle fMemReserve;		/* Memory reserve for the view */
	Boolean fActive;		/* Is this text view active? */
	Boolean fAutoWrap;		/* Wrap around text within view bounds? */
	TView *fSuperView;		/* For ClikLoop, better solution ??? */
	
	virtual pascal void BTActivate (Boolean activate, Boolean redraw);
	virtual pascal void BTAppend (unsigned char *buf, long count,
		Boolean redraw);
	virtual pascal void BTClick (Point *pt, short clicks, Boolean extend);
	virtual pascal void BTDrop (long size, Boolean redraw);
	virtual pascal Handle BTGetText (void);
	virtual pascal void BTResize (Rect *display, long bleft, long bwidth,
		Boolean redraw);
	virtual pascal void BTScroll (long h, long v, Boolean redraw);
	virtual pascal void BTSetSelect (long selStart, long selEnd,
		Boolean redraw);
	virtual pascal void BTSetStyle (TextStyle *theStyle, long tabChars,
		Boolean redraw);
	
	virtual pascal TBHandle AppendBuf (TBHandle th, long size);
	virtual pascal long AppendChars (unsigned char *buf, long count);
	virtual pascal long Buf2Char (TBHandle th, long pos);
	virtual pascal long Buf2Hor (TBHandle th, long pos);
	virtual pascal long Buf2Vert (TBHandle th, long pos);
	virtual pascal void BuildReserve (Boolean build);
	virtual pascal void Char2Buf (long charPos, TBHandle *th, long *pos);
	virtual pascal long ChunkSize (TBHandle th, long pos, long epos);
	virtual pascal void ClrLines (TBHandle th);
	virtual pascal void Draw (Rect *area);
	virtual pascal void DropBuf (void);
	virtual pascal Boolean DropSome (void);
	virtual pascal long FindLine (TBHandle th, long pos);
	virtual pascal void FindRun (short clicks, long h, long v, long *cbeg,
		long *cend, long *cloc);
	virtual pascal void Free (void);
	virtual pascal void Highlight (long start, long end);
	virtual pascal void Hor2Buf (long h, TBHandle th, long *pos, Boolean *left);
	virtual pascal void IBigText (Rect *display, TextStyle *theStyle,
		long tabChars, TView *itsView);
	virtual pascal void Recalc (long vfrom, long vto, Boolean adjTop);
	virtual pascal void RecalcBuf (TBHandle th);
	virtual pascal Boolean SetLSVal (LSHandle lh, long line, short val);
	virtual pascal void Vert2Buf (long v, TBHandle *th, long *pos);
};

#endif
