/*
 * CQixableCursor.c
 * A subclass of CAnimCursor;  draws a "Qix" over your B&W cursors.
 * Version 1.0, 13 May 1992
 *
 * by Jamie McCarthy
 * Internet: k044477@kzoo.edu				AppleLink: j.mccarthy
 * Telephone:  800-421-4157 (9:00-5:00 Eastern time)
 * I'm releasing this code with the hope that someone will get something
 * out of it.  Feedback of any sort, even just letting me know that you're
 * using it, is greatly appreciated!
 *
 * CQixableCursor's source code is in the public domain.  If you make changes,
 * please do me the courtesy of letting me know before you redistribute it.
 *
 * This class basically does nothing special except draw lines over the
 * cursors it finds.  If you want it to not do that, call setQixing(FALSE).
 *
 * CQixableCursor requires CRandom, which is available at ftp.brown.edu and
 * other fine archives.
 *
 * Changes from 1.0b3 to 1.0:
 * Changed name from CQixCursor to CQixableCursor.
 * Added and implemented the setQixing() and getQixing() calls.
 * Initialized gRandom if it wasn't already.
 *
 * Changes from 1.0b1 to 1.0b3:
 * Fixed the "if srcXor and an odd number of lines, leave a stray pixel" bug.
 * Fixed the "if max number of lines, hover in the bottom left corner" bug.
 * Improved efficiency somewhat.
 *
 */



/********************************/

#include "CQixableCursor.h"

/********************************/

#include <QuickDraw.h>

#include "CRandom.h"

/********************************/

static void doDrawLine(Bits16 theData,
	short lineLoc[2][2],
	short xferMode);

static void randomPositiveVelocity(short *theVelocity);

/********************************/



void CQixableCursor::IQixableCursor(short rsrcID)
{
	register short i, j, k;
	
	isQixing = TRUE;
	
	inherited::IAnimCursor(rsrcID);
	
	setNQixLines(5);
	setQixXferMode(srcOr);
	
	for (i = 0; i < 2; ++i) {
		for (j = 0; j < 2; ++j) {
			for (k = 0; k < kMaxNQixLines; ++k) {
				lineLoc[k][i][j] = 1;
				lineLoc[k][i][j] = 1;
			}
			randomPositiveVelocity(&velocity[i][j]);
			if (j == 1) {
				while (velocity[i][1] == velocity[i][0]) {
					randomPositiveVelocity(&velocity[i][1]);
				}
			}
		}
	}
	
	setTicksBetweenCursors(kDefaultQixTicksBetweenCursors);
	
	setMode(kCACModeInterrupted);
	
	if (gRandom == NULL) {
		gRandom = new(CRandom);
		gRandom->IRandom();
	}
}



void CQixableCursor::setNQixLines(short nLines)
{
	nQixLines = nLines;
	
	if (nQixLines < 1) nQixLines = 1;
	else if (nQixLines > kMaxNQixLines) nQixLines = kMaxNQixLines;
}



short CQixableCursor::getNQixLines(void)
{
	return nQixLines;
}



void CQixableCursor::setQixXferMode(short newXferMode)
{
	switch (newXferMode) {
		
		case srcOr:
		case srcXor:
			xferMode = newXferMode;
			break;
		
		default:
				/* Invalid mode passed -- do nothing. */
			break;
		
	}
}



short CQixableCursor::getQixXferMode(void)
{
	return xferMode;
}



void CQixableCursor::setQixing(Boolean newQixing)
{
	isQixing = newQixing;
	determineTryToUseColor();
}



Boolean CQixableCursor::getQixing(void)
{
	return isQixing;
}



void CQixableCursor::nextCursor(void)
{
	register short i, j, k;
	register short cQixLine;
	register short temp;
	
	inherited::nextCursor();
	
	if (getQixing() && !usingColorCursors) {
		
			/* Update the now-current cursor to put the lines in it. */
		
		cQixLine = nQixLines-1;
	
		
		undrawLine(0);
		
		for (i = 1; i <= cQixLine; ++i) {
			for (j = 0; j < 2; ++j) {
				for (k = 0; k < 2; ++k) {
					lineLoc[i-1][j][k] = lineLoc[i][j][k];
				}
			}
		}
		
		for (i = 0; i < 2; ++i) {
			for (j = 0; j < 2; ++j) {
				temp = lineLoc[cQixLine][i][j];
				temp += velocity[i][j];
				if (temp < 1 || temp > 14) {
					short newSign;
					newSign = (velocity[i][j] < 0 ? 1 : -1);
					randomPositiveVelocity(&velocity[i][j]);
					velocity[i][j] *= newSign;
					if (temp < 1) temp = 1; else temp = 14;
					temp += velocity[i][j];
				}
				lineLoc[cQixLine][i][j] = temp;
			}
		}
		
		drawLine(cQixLine);
		
	}
}



void CQixableCursor::determineTryToUseColor(void)
{
		/*
		 * If we're presently Qixing, don't try to load color.  If not, then
		 * load color--which may disable Qixing later.  The moral is--if you
		 * want Qixing, be sure to call setQixing(TRUE) before you call
		 * useAnimCursorID().
		 */
	inherited::determineTryToUseColor();
	if (tryToUseColor) {
		tryToUseColor = (!isQixing);
	}
}



void CQixableCursor::undrawLine(short wLine)
{
	register acurPtr theAcurPtr;
	theAcurPtr = *itsAcurHndl;
	
	if (getQixXferMode() == srcXor) {
		
		if (lineLoc[wLine][0][0] == 1
			&& lineLoc[wLine][0][1] == 1
			&& lineLoc[wLine][1][0] == 1
			&& lineLoc[wLine][1][1] == 1) {
			
			return ;
			
		}
		
		doDrawLine( (**theAcurPtr->cursor[theAcurPtr->cCursor]).data,
			lineLoc[wLine],
			srcXor );
		
	} else {
		
		doDrawLine( (**theAcurPtr->cursor[theAcurPtr->cCursor]).data,
			lineLoc[wLine],
			srcBic );
		
	}
}



void CQixableCursor::drawLine(short wLine)
{
	register acurPtr theAcurPtr;
	theAcurPtr = *itsAcurHndl;
	
	if (lineLoc[wLine][0][0] == 1
		&& lineLoc[wLine][0][1] == 1
		&& lineLoc[wLine][1][0] == 1
		&& lineLoc[wLine][1][1] == 1) {
		
		return ;
		
	}
	
	doDrawLine( (**theAcurPtr->cursor[theAcurPtr->cCursor]).data,
		lineLoc[wLine],
		getQixXferMode() );
}



	/* Note that the macros are for column 0 on the right, 15 on the left. */
#define setBit(theBits16,wCol,wRow) theBits16[wRow] |=  (1<<wCol)
#define clrBit(theBits16,wCol,wRow) theBits16[wRow] &= ~(1<<wCol)
#define xorBit(theBits16,wCol,wRow) theBits16[wRow] ^=  (1<<wCol)
#define drawBit(theBits16,wCol,wRow,xferMode) {										\
		if (xferMode==srcOr) setBit(theBits16,wCol,wRow);							\
		else if (xferMode==srcBic) clrBit(theBits16,wCol,wRow);					\
		else xorBit(theBits16,wCol,wRow);												\
	}

static void doDrawLine(register Bits16 theData,
	register short lineLoc[2][2],
	register short xferMode)
{
		/* Bresenham's line-drawing algorithm, in integer arithmetic. */
	
	register short hSign, vSign;
	short deltaH, deltaV;
	register short cOffsetH, cOffsetV;
	register short d, inc1, inc2;
	
	cOffsetH = lineLoc[0][0];
	cOffsetV = lineLoc[0][1];
	
		/*
		 * Set the sign flags, and perform an absolute-value on the deltas
		 * at the same time.  By taking the absolute values, the number of
		 * cases is reduced from twelve to three.
		 */
	if ( (deltaH = lineLoc[1][0]-cOffsetH) >= 0) {
		hSign = 1;
	} else {
		hSign = -1;
		deltaH = -deltaH;
	}
	if ( (deltaV = lineLoc[1][1]-cOffsetV) >= 0) {
		vSign = 1;
	} else {
		vSign = -1;
		deltaV = -deltaV;
	}
	
	if (deltaH > deltaV) {
		
		d = (deltaV << 1) - deltaH;
		inc1 = deltaV << 1;
		inc2 = (deltaV - deltaH) << 1;
		
		drawBit(theData, cOffsetH, cOffsetV, xferMode);
		while (cOffsetH != lineLoc[1][0]) {
			cOffsetH += hSign;
			if (d < 0) {
				d += inc1;
			} else {
				cOffsetV += vSign;
				d += inc2;
			}
			drawBit(theData, cOffsetH, cOffsetV, xferMode);
		}
		
	}
	
	else
	
	if (deltaH < deltaV) {
		
		d = (deltaH << 1) - deltaV;
		inc1 = deltaH << 1;
		inc2 = (deltaH - deltaV) << 1;
		
		drawBit(theData, cOffsetH, cOffsetV, xferMode);
		while (cOffsetV != lineLoc[1][1]) {
			cOffsetV += vSign;
			if (d < 0) {
				d += inc1;
			} else {
				cOffsetH += hSign;
				d += inc2;
			}
			drawBit(theData, cOffsetH, cOffsetV, xferMode);
		}
		
	}
	
	else
	
	/* we know that (deltaH == deltaV) */ {
		
		drawBit(theData, cOffsetH, cOffsetV, xferMode);
		while (cOffsetH != lineLoc[1][0]) {
			cOffsetH += hSign;
			cOffsetV += vSign;
			drawBit(theData, cOffsetH, cOffsetV, xferMode);
		}
		
	}
}



static void randomPositiveVelocity(short *theVelocity)
{
		/* Get a random number from 1 to 3, with 2 most likely. */
	*theVelocity = (gRandom->linearShort(3, 6) >> 1);
}
