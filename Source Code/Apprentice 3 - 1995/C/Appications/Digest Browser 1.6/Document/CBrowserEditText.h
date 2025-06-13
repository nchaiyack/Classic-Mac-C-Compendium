/******************************************************************************
 CBrowserEditText.h
 ******************************************************************************/
 
#define _H_CBrowserEditText

#include "CEditText.h"				/* Interface for its superclass		*/

class CBrowserEditText : public CEditText {		/* Class Declaration				*/

public:
	void		IBrowserEditText(CView *anEnclosure, CBureaucrat *aSupervisor,
							short aWidth, short aHeight,
							short aHEncl, short aVEncl,
							SizingOption aHSizing, SizingOption aVSizing,
							short aLineWidth);
								/** Instance Variables **/
	virtual void		CalcTERects(void);
};
