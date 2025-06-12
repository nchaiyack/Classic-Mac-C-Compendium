/*
 * CQixableCursor.h
 * Version 1.0, 13 May 1992
 *
 */



/********************************/

#pragma once

/********************************/

#include "CAnimCursor.h"

/********************************/

	/* The maximum number of lines possible.  Fifteen would be quite crowded. */
#define kMaxNQixLines (15)

	/* The Qix likes to go a little faster than the standard default. */
#define kDefaultQixTicksBetweenCursors (4)

/********************************/



class CQixableCursor : public CAnimCursor {
	
public:
	
	void			IQixableCursor(short rsrcID);
	
	void			setNQixLines(short nLines);
	short			getNQixLines(void);
	
	void			setQixXferMode(short newXferMode);
	short			getQixXferMode(void);
	
	void			setQixing(Boolean newQixing);
	Boolean		getQixing(void);
	
protected:
	
	Boolean		isQixing;
	short			nQixLines;
	short			xferMode;
	short			lineLoc[kMaxNQixLines][2][2];
	short			velocity[2][2];
	
	void			determineTryToUseColor(void); // override
	
	void			nextCursor(void); // override
	
	void			undrawLine(short wLine);
	void			drawLine(short wLine);
	
private:
	
} ;
