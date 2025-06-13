/*************************************************************************************

 CScrollListDragger.h

		Interface for CScrollListDragger
	
	SUPERCLASS = CMouseTask
	
		© 1992 Dave Harkness

*************************************************************************************/


#define _H_CScrollListDragger

#include <CMouseTask.h>
#include <LongCoordinates.h>


class CScrollList;
class CArray;


class CScrollListDragger : public CMouseTask
{
public:

	void			IScrollListDragger( CScrollList *aTable, short theModifiers,
										short aHeight, short fOptions);
	
	virtual void	Dispose( void);
	
	virtual void	BeginTracking( LongPt *startPt);
	virtual void	KeepTracking( LongPt *currPt, LongPt *prevPt, LongPt *startPt);
	virtual void	EndTracking( LongPt *currPt, LongPt *prevPt, LongPt *startPt);

protected:

	CScrollList		*itsScrollList;		// client scroll list
	CArray			*itsArray;			// client array
	short			movingCell;			// cell we're moving
	short			modifierKeys;		// modifier keys from the mousedown event
	short			cellHeight;			// height of each cell
	short			listOptions;		// options selected for the list
	LongRect		dragRect;			// dragging rect
	LongRect		clipRect;			// rectangle to clip to
	Boolean			dragging;			// TRUE if dragging yet

};