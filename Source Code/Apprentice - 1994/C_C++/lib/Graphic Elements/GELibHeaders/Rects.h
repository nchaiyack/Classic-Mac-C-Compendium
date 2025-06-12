/*
	Rects.h
	
	Rectangle handling routines for Graphic Elements release version 1.0b1
	
	Copyright 1994 by Al Evans. All rights reserved.
	
	3/7/94
	
*/

#include "List.h"

//Entry in a list of rectangles
typedef struct rEntry *RListEntryPtr;

typedef struct rEntry {
	RListEntryPtr		nextEntry;
	Rect				thisRect;
} RListEntry;

//Coding masks for overlapping rectangles, see CodeRects below
typedef enum {	noOvlp =		0,
				ovlpTop = 		1,
	  			ovlpLeft =		1 << 1,
				ovlpBottom =	1 << 2,
				ovlpRight =		1 << 3,
				ovlpTR =		1 << 4,
				ovlpTL =		1 << 5,
				ovlpBL =		1 << 6,
				ovlpBR =		1 << 7,
				uncoded =		1 << 8
		     } RectCode;

#ifdef __cplusplus
extern "C" {
#endif


//Returns a code as defined above showing how testRect overlaps baseRect
RectCode CodeRect(const Rect* testRect, const Rect* baseRect);

/*
	The following function is meant to be called iteratively until it returns
	false. If code is non-zero, it returns rectangles in *src2 and *dst2 which
	represent srcRect and dstRect "wrapped around" totalSource. It alters
	the value of code, to reflect the fact that it has handled one case of 
	"wraparound". If code is noOvlp, it clips srcRect against totalSource,
	clips destRect accordingly, and returns the results in src2 and dst2.
*/

Boolean SplitRects(RectCode *code, const Rect* totalSource, const Rect* srcRect,
		const Rect* dstRect, Rect* src2, Rect* dst2);


//Rectangle list functions

//Returns new rectangle list
LHeaderPtr InitRectList(short nRects);

Boolean InsertRectInList(LHeaderPtr listHead, Rect* theRect);

//Removes all rectangles from rectList
void ClearRectList( LHeaderPtr rectList);


//Fast assembly-language rectangle arithmetic

//IFF rects intersect, intersection is returned in rect2
extern Boolean RectsIntersect(const Rect * rect1, Rect * rect2);

// rect2 = rect1 U rect2
extern void RectUnion(Rect * rect1, Rect * rect2 );

// rect1->top = rect1->left = rect1->bottom = rect1->right = 0
extern void RectZero(Rect * rect1);

// true IFF (rect1->top >= rect1->bottom) || (rect1->left >= rect1->right)
extern Boolean RectEmpty(Rect * rect1);

// Offsets rect1 by dx, dy
extern void RectOffset(Rect * rect1, long dx, long dy);

#ifdef __cplusplus
}
#endif
