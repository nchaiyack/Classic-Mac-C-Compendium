//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CMouseControlsArrayPane.cp
//|
//| This file contains the interface to the CMouseControlsArray class.  The
//| CMouseControlsArrayPane class implements the the scrolling list in the mouseMouse
//| controls window.
//|________________________________________________________________________________


#include <CArrayPane.h>


class CMouseControlsArrayPane : public CArrayPane
	{

  public:

	void IMouseControlsArrayPane( CView *anEnclosure, CBureaucrat *aSupervisor,
							short aWidth, short aHeight,
							short aHEncl, short aVEncl,
							SizingOption aHSizing, SizingOption aVSizing);

	void DrawCell(Cell theCell, Rect *cellRect);
	
	};
