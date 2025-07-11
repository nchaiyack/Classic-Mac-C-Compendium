/****
 * CSelectLine.h
 *
 *	A text edit pane for a tiny editor.
 *
 ****/


#define _H_CSelectLine

// JRB change from CEditText to CBrowserEditText
#include "CStyleText.h"

struct CSelectLine : CStyleText {
protected:
	long		selLine;
public:
								/** Contruction/Destruction **/
	void	ISelectLine(CView *anEnclosure, CBureaucrat *aSupervisor,
				short vLoc, short vHeight);

	void	FindEnds(long line, long *lineStart, long *lineEnd);
	void	DoClick(Point hitPt, short modifierKeys, long when);
	void	DoKeyDown(char theChar, Byte keyCode, EventRecord *macEvent);
	void	SetSelectedLine( long line, Boolean fRedraw);
	long	GetSelectedLine(void);

	void	AdjustCursor(Point where, RgnHandle mouseRgn);
};
