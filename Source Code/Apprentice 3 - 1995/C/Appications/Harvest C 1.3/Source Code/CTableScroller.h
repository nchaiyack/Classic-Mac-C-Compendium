/******************************************************************************
 CTableScroller.h

		
	SUPERCLASS = CScrollPane
	
	Copyright © 1991 Symantec Corporation. All rights reserved.
	

 ******************************************************************************/

#define _H_CTableScroller

#include "CScrollPane.h"

class CTableLabels;

class CTableScroller : public CScrollPane
{
public:

	CTableLabels	*itsRowLabels;
	CTableLabels	*itsColLabels;
	
	void	ITableScroller(	CView *anEnclosure, CBureaucrat *aSupervisor,
					short aWidth, short	aHeight, short aHEncl, short aVEncl,
					SizingOption aHSizing, SizingOption aVSizing,
					Boolean	hasHoriz, Boolean hasVert, Boolean hasSizeBox);

	virtual void	SetLabels( CTableLabels *rowLabels, CTableLabels *colLabels);
	
	virtual void	ScrollBits( long hDelta, long vDelta);
	virtual void	DoScroll( long hDelta, long vDelta);
	
};