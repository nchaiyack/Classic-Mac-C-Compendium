/*
 * CSampleAnimCursor.h
 *
 */



/********************************/

#pragma once

/********************************/

#include "CQixableCursor.h"

/********************************/

enum {
	kQixCursorID = 128,
	kStripedArrowID,
	kBeachballID,
	kSpinningEarthID
} ;

/********************************/



class CSampleAnimCursor : public CQixableCursor {
	
public:
	
	void			ISampleAnimCursor(void);
	
	void			useQixCursor(void);
	void			useStripedArrowCursor(void);
	void			useBeachballCursor(void);
	void			useSpinningEarthCursor(void);
	
protected:
	
	
private:
	
} ;
